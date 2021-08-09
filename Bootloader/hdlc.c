#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "shareddefs.h"
#include "sharedinterface.h"
#include "queue.h"
#include "timer.h"
#include "serial.h"
#include "hdlc.h"

#ifdef LINUX
BOOL bDiscardNext = FALSE;
extern BOOL bIsController;
extern BOOL bTossSendPacket;
extern int nNextNS;
extern char* pHandheldOffset;
#else
char* pHandheldOffset = "";
#endif


/////
// hdlc control block
// one for each interface type
// indexed by interface type
/////
static HDLCCONTROL hdlcControlBlock[3];

static HDLC_QUEUE hdlcReceivedQueue;
static HDLC_QUEUE hdlcFreeQueue;
static HDLC_QUEUE_ENTRY hdlcReceiveQueueEntries[HDLC_MAX_RECEIVE_PACKETS];

static unsigned long nOurESN;
static unsigned long nSendersESN;

static BOOL bWeAreSlave;

static eINTERFACE eConnectToInterface;
static int nNewConnectionsCount[INTERFACE_COUNT];
static void printHDLCControl(char* szText,eINTERFACE eInterface, unsigned char nHDLCControl)
{
#ifdef SHOW_HDLC_TRAFFIC
   char *pType = "??";
   int nType = 0;
   int nNR = 0;
   int nNS = 0;
	printf("%s [%d] %s: ", pHandheldOffset, (int)eInterface, szText);
   if(nHDLCControl & SF_BIT)
   {
      if(nHDLCControl & UF_BIT)
      {
        nType = (nHDLCControl&UFLAG_TYPE1_MASK)>>UFLAG_TYPE1_SHIFT;
				nType |= (nHDLCControl&UFLAG_TYPE2_MASK)>>UFLAG_TYPE2_SHIFT;
        switch(nType)
        {
            case eUPACKET_TYPE_SABM:
               pType = "SABM";
               break;
            case eUPACKET_TYPE_DISC:
               pType = "DISC";
               break;
            case eUPACKET_TYPE_DM:
               pType = "DM";
               break;
            case eUPACKET_TYPE_UA:
               pType = "UA";
               break;
            default:
               break;
        }
        printf("U-Frame Type[%d][%s] ", nType, pType);
      }
      else
      {
        nType = (nHDLCControl&SFLAG_TYPE_MASK)>>SFLAG_TYPE_SHIFT;
				nNR =   (nHDLCControl&NR_MASK)>>NR_SHIFT;
        switch(nType)
        {
            case eSPACKET_TYPE_RR:
               pType = "RR";
               break;
            case eSPACKET_TYPE_RNR:
               pType = "RNR";
               break;
            case eSPACKET_TYPE_REJ:
               pType = "REJ";
               break;
            default:
               break;
        }
        printf("S-Frame NR[%d] Type[%d][%s] ", nNR, nType, pType);
      }
   }
   else
   {
				nNR =   (nHDLCControl&NR_MASK)>>NR_SHIFT;
				nNS =   (nHDLCControl&NS_MASK)>>NS_SHIFT;
        printf("I-Frame NR[%d] NS[%d] ", nNR, nNS);
        //printf("nPacketID_Status[%d] nPacketType[%d] ", nNR, nNS);
   }
   printf(" PF[%d]\n", ((nHDLCControl&PF_BIT)?1:0));
#endif
}

///////////////////////////////////////////////////////////
// sliding window receive packet check
/////
// nNR         - received ack number - indicates next expected
//                sequence number (nNR can equal nLHS - window is closed)
// nRHS        - right hand side of window - seq number to the
//                right of the last valid number that can be used
// nWindowSize - size of the window
///////////////////////////////////////////////////////////
static int checkNR(int nNR, int nRHS, int nWindowSize)
{
   int nNumAck = 0;  // number of ack frames
   int nLHS; // left hand side of the window
   /////
   // compute nLHS
   /////
   if( (nRHS-nWindowSize) >= 0)
   {
      nLHS = nRHS-nWindowSize;
   }
   else
   {
      nLHS = nRHS - nWindowSize + HDLC_WINDOW_SIZE;
   }
   /////
   // Is nr in window
   /////
   if( ((nLHS < nRHS) && (nNR >= nLHS && nNR <= nRHS)) || // when window contains numbers in sequence
			((nLHS > nRHS) && (nNR >= nLHS || nNR <= nRHS)) )
   {
         /////
         // Yes in the window
				 // number of frames ack = nNR - nRHS
         /////
         if((nLHS < nRHS))
         {
            nNumAck = nNR-nLHS;
         }
         else
         {
            if(nNR <= nRHS)
            {
               nNumAck = nNR + HDLC_WINDOW_SIZE - nLHS;
            }
            else
            {
               nNumAck = nNR - nLHS;
            }
         }
   }
   return(nNumAck);
}

///////////////////////////////////////////////////////////
// sliding window sent packet check
// is this the packet we are expecting
// or could it be a resend?
//
// nNS         - the packet sequence number they sent
// nVR         - the next packet we expect to receive
//
// return:
//           -1, lower than what we expect
//            0, what we expect
//            1, higher than we expect
///////////////////////////////////////////////////////////
static int checkNS(int nNS, int nVR)
{
   int nRetVal = 0;  // assume ok
   if(nVR != nNS)
   {
		  /////
      // not the one we expected
      // could it be a resend?
      // if so, it should be lower
      ///////
		int nS1 = nNS;
		nRetVal = 1;
		if(nVR < nNS)
		{
			/////
			// wrap N(s) back around 
			/////
			nS1 = nNS-HDLC_WINDOW_SIZE;
		}
		if((nVR-SEND_WINDOW_SIZE) <= nS1)
		{
			/////
			// yep, nNS is lower than nVR
			// and still in the window
			/////
			nRetVal = -1;
		}
   }
   return(nRetVal);
}
static void xmitPacket(eINTERFACE eInterface, HDLCPACKET* pHDLCPacket)
{
	int nInIndex;
	int nOutIndex;
	unsigned char nChecksum = 0;
	unsigned char* pDataIn = (unsigned char*)pHDLCPacket;
	unsigned char outgoingPacket[(sizeof(HDLCPACKET)*2)];
	HDLCCONTROL *pHDLCControl = &hdlcControlBlock[(int)eInterface];
	int nNR;
	
	if(0 ==(pHDLCPacket->nHDLCControl&SF_BIT))
	{	
		/////
		// add N(r)
		/////
		nNR = pHDLCControl->nVR;

		pHDLCPacket->nHDLCControl &= ~(NR_MASK);
		pHDLCPacket->nHDLCControl |= ((nNR&NR_MASK2)<<NR_SHIFT);
	}
	else if(0 == (pHDLCPacket->nHDLCControl&UF_BIT))
	{
		/////
		// add N(r)
		/////
		nNR = pHDLCControl->nVR;
		pHDLCPacket->nHDLCControl &= ~(NR_MASK);
		pHDLCPacket->nHDLCControl |= ((nNR&NR_MASK2)<<NR_SHIFT);
	}
	
	/////
	// add our ESN
	/////
	pHDLCPacket->command.nSendersESN = nOurESN;

	/////
	// calculate checksum
	/////
	for(nInIndex=0; nInIndex< sizeof(HDLCPACKET)-1; nInIndex++)
	{
		nChecksum ^= pDataIn[nInIndex];
	}
	pHDLCPacket->nChecksum = nChecksum;

	 //printf("eInterface[%d]\n", eInterface);
   printHDLCControl("XM", eInterface, pHDLCPacket->nHDLCControl);
	/////
	// escape all packet delimeters
	/////
	nOutIndex = 0;
	for(nInIndex=0; nInIndex<sizeof(HDLCPACKET); nInIndex++)
	{
		if(PACKET_DELIM == pDataIn[nInIndex])
		{
			outgoingPacket[nOutIndex++] = PACKET_ESCAPE;
			outgoingPacket[nOutIndex++] = PACKET_ESCAPE_DELIM;
		}
		else if(PACKET_ESCAPE == pDataIn[nInIndex])
		{
			outgoingPacket[nOutIndex++] = PACKET_ESCAPE;
			outgoingPacket[nOutIndex++] = PACKET_ESCAPE_ESCAPE;		
		}
		else
		{
			outgoingPacket[nOutIndex++] = pDataIn[nInIndex];
		}
	}

	serialSendPacket(eInterface, outgoingPacket, nOutIndex);
}

static void resendAllPackets(eINTERFACE eInterface, HDLCCONTROL *pHDLCControl)
{

	/////
	// resend all packets on the sent queue
	/////
	HDLC_QUEUE_ENTRY* pEntry = (HDLC_QUEUE_ENTRY*)queuePeek(&pHDLCControl->sentQueue, (QUEUE_ENTRY*)NULL);
	while(NULL != pEntry)
	{
         printHDLCControl("Resend Packet", eInterface, pEntry->hdlcPacket.nHDLCControl);
		/////
		// resend this command
		/////
		xmitPacket(eInterface, &pEntry->hdlcPacket);
		pEntry = (HDLC_QUEUE_ENTRY*)queuePeek(&pHDLCControl->sentQueue, (QUEUE_ENTRY*)pEntry);
	}
}
static void discardSentPackets(eINTERFACE eInterface, HDLCCONTROL *pHDLCControl, int nNumAck)
{

   int i;
   for(i=0; i<nNumAck; i++)
   {
      HDLC_QUEUE_ENTRY* pEntry = (HDLC_QUEUE_ENTRY*)queuePop(&pHDLCControl->sentQueue);
      if(NULL != pEntry)
      {
         printHDLCControl("Discard Packet", eInterface, pEntry->hdlcPacket.nHDLCControl);
         queueAdd(&pHDLCControl->freeQueue, (QUEUE_ENTRY*)pEntry);
      }
   }
}
static BOOL addSendPacketToQueue(HDLCPACKET *hdlcPacket, HDLCCONTROL *pHDLCControl)
{
	BOOL bRetVal = FALSE;
	HDLC_QUEUE_ENTRY *pEntry = (HDLC_QUEUE_ENTRY*)queuePop(&pHDLCControl->freeQueue);
	if(NULL != pEntry)
	{
		/////
		// got a free entry
		// copy this command to it
		// and put it onto the sent queue
		/////
		memcpy((unsigned char*)&pEntry->hdlcPacket, hdlcPacket, sizeof(HDLCPACKET));
		queueAdd(&pHDLCControl->sentQueue, (QUEUE_ENTRY*)pEntry);
		bRetVal = TRUE;
	}
	return bRetVal;
}
void hdlcSendPacket(eINTERFACE eInterface, HDLCPACKET* pHDLCPacket)
{
	HDLCCONTROL *pHDLCControl = &hdlcControlBlock[(int)eInterface];
	int nNS;
	
	if(0 == (pHDLCPacket->nHDLCControl&SF_BIT))
	{	
		//printf("1-Send I-Frame\n");
		/////
		// I frame
		// only send it if we are connected
		/////
		if(pHDLCControl->bConnected)
		{
			/////
			// N(s)
			/////
			nNS = pHDLCControl->nVS;
#ifdef LINUX
                if(0 <= nNextNS)
                {
                  /////
                  // replace NS with this
                  // to test packet reject
                  /////
                  nNS = nNextNS;
                  nNextNS = -1;
                }
#endif
			pHDLCPacket->nHDLCControl &= ~(NS_MASK);
			pHDLCPacket->nHDLCControl |= ((nNS&NS_MASK2)<<NS_SHIFT);
		
			/////
			// bump the packet sequence
			// for next time
			/////
			pHDLCControl->nVS++;
		
			/////
			// getting ready to send a packet
			// add it to the sent packet queue for this interface
			// if that queue is full, then discard this packet
			/////
			//printf("2-Send I-Frame\n");
			if(addSendPacketToQueue(pHDLCPacket, pHDLCControl))
			{
				//printf("3-Send I-Frame\n");
				xmitPacket(eInterface, pHDLCPacket);
			}
		}
	}
	else
	{
		/////
		// S or U frame
		// just send these
		/////
		xmitPacket(eInterface, pHDLCPacket);
	}
}

static void hdlcSendSFrame(eINTERFACE eInterface, eSPACKET_TYPE eSPacketType, BOOL bPoll)
{
	HDLCPACKET hdlcPacket;
	int nPacketType = (int)eSPacketType;
	
	hdlcPacket.nHDLCControl = (nPacketType<<SFLAG_TYPE_SHIFT)&SFLAG_TYPE_MASK;;
	hdlcPacket.nHDLCControl |= SF_BIT;
	if(bPoll)
	{
		hdlcPacket.nHDLCControl |= PF_BIT;
	}
	hdlcSendPacket(eInterface, &hdlcPacket);
}
static void hdlcSendUFrame(eINTERFACE eInterface, eUPACKET_TYPE eUPacketType, BOOL bPoll)
{
	HDLCPACKET hdlcPacket;
	int nPacketType = (int)eUPacketType;
	
	hdlcPacket.nHDLCControl = ((nPacketType&0x1C)<<UFLAG_TYPE2_SHIFT);
	hdlcPacket.nHDLCControl |=((nPacketType&0x03)<<UFLAG_TYPE1_SHIFT);
	hdlcPacket.nHDLCControl |= SF_BIT|UF_BIT;
	if(bPoll)
	{
		hdlcPacket.nHDLCControl |= PF_BIT;
	}
	hdlcSendPacket(eInterface, &hdlcPacket);
}
static BOOL rawPacketToHDLC(unsigned char* pPacket, int nPacketLength, HDLCPACKET* pHDLCPacket)
{
	BOOL bRetVal = FALSE;
	int nInIndex = 0;
	int nOutIndex = 0;
	BOOL bBadPacket = FALSE;
	unsigned char nChecksum = 0;
	unsigned char *pC = (unsigned char*)pHDLCPacket;
		/////
		// unescape 0x7E and 0x7D
		/////
	do
	{
		while(1)
		{
			unsigned char nData = pPacket[nInIndex++];
			if(PACKET_ESCAPE == nData)
			{
				/////
				// found the start of an escape sequence
				/////
				//if(nPacketLength <= ++nInIndex)
				if(nPacketLength <= nInIndex)
				{
					bBadPacket = TRUE;
					printf("rawPacketToHDLC: nPacketLength[%d] <= nInIndex[%d] bBadPacket = TRUE\n",nPacketLength,nInIndex);
					break;
				}
				nData = pPacket[nInIndex++];
				if(PACKET_ESCAPE_DELIM == nData)
				{
					nData = PACKET_DELIM;
				}
				else if(PACKET_ESCAPE_ESCAPE == nData)
				{
					nData = PACKET_ESCAPE;
				}
				else
				{
					bBadPacket = TRUE;
					printf("rawPacketToHDLC: nData[%d] No Escape badPacket = TRUE\n",nData);
					break;
				}
			}
			pC[nOutIndex++] = nData;
			if(nInIndex >= nPacketLength)
			{
				//printf("rawPacketToHDLC: nInIndex[%d] >= nPacketLength[%d] badPacket = TRUE\n",nPacketLength,nInIndex);
				break;
			}
			if(nOutIndex >= sizeof(HDLCPACKET))
			{
				printf("rawPacketToHDLC: nOutIndex[%d] >= sizeof(HDLCPACKET)[%d] badPacket = TRUE\n",nOutIndex,sizeof(HDLCPACKET));
				break;
			}
		}
		if(bBadPacket)
		{
			printf("rawPacketToHDLC: Bad Packet\n");
			break;
		}
		if(sizeof(HDLCPACKET) != nOutIndex)
		{
			//printf("rawPacketToHDLC: sizeof(HDLCPACKET)[%d] != nOutIndex[%d] Bad Packet\n",sizeof(HDLCPACKET),nOutIndex);
			break;
		}
		
		/////
		// we have a packet
		// so compute the checksum
		/////

		for(nOutIndex=0; nOutIndex < sizeof(HDLCPACKET); nOutIndex++)
		{
			nChecksum ^= pC[nOutIndex];
		}
		if(0 == nChecksum)
		{
			bRetVal = TRUE;
		}
	}	while(0);
	
	return bRetVal;
}
static void hdlcInitHDLCControlBlock(int nInterface)
{
	int i;

	memset((unsigned char*)&hdlcControlBlock[nInterface], 0, sizeof(HDLCCONTROL));
	hdlcControlBlock[nInterface].bConnected = FALSE;
	queueInitialize(&hdlcControlBlock[nInterface].freeQueue);
	queueInitialize(&hdlcControlBlock[nInterface].sentQueue);
	for(i=0;i<SEND_WINDOW_SIZE;i++)
	{
		queueAdd(&hdlcControlBlock[nInterface].freeQueue, (QUEUE_ENTRY*)&hdlcControlBlock[nInterface].hdlcSentPacketHistory[i]);
	}
	initTimer(&hdlcControlBlock[nInterface].packetTimer);
	
	startTimer(&hdlcControlBlock[nInterface].packetTimer, NO_TRAFFIC_TIMEOUT);
	hdlcControlBlock[nInterface].nNoTrafficRetries = NO_TRAFFIC_RETRIES;
	
	hdlcControlBlock[nInterface].nRHS = SEND_WINDOW_SIZE;
}

int hdlcGetNewConnectionsCount(eINTERFACE eInterface)
{
	return nNewConnectionsCount[(int)eInterface];
}
void hdlcProcessor(eINTERFACE eInterface, unsigned char* pPacket, int nPacketLength)
{

	HDLCPACKET hdlcPacket;
	HDLC_QUEUE_ENTRY* pReceivedQueueEntry;
	BOOL bPF;
	BOOL bSframe;
	BOOL bUframe;
	HDLCCONTROL *pHDLCControl = &hdlcControlBlock[(int)eInterface];
	do
	{
		//printf("1-hdlcProcessor\n");
		if(!rawPacketToHDLC(pPacket, nPacketLength, &hdlcPacket))
		{
			/////
			// packet not valid
			// so skip out
			/////
			//printf("hdlcProcessor: Received Invalid Packet\n");
			break;
		}


		// packet is valid
		/////
		bPF     = (hdlcPacket.nHDLCControl & PF_BIT)?TRUE:FALSE;
		bSframe = (hdlcPacket.nHDLCControl & SF_BIT)?TRUE:FALSE;

		startTimer(&pHDLCControl->packetTimer, NO_TRAFFIC_TIMEOUT);
		pHDLCControl->nNoTrafficRetries = NO_TRAFFIC_RETRIES;
    printHDLCControl("Got Packet", eInterface, hdlcPacket.nHDLCControl);
		
		/////
		// if from wireless interface
		// is it from our peer
		/////
		if(eINTERFACE_WIRELESS == eInterface)
		{
			//printf("pkt[%X] nSendersESN[%lX]\n", hdlcPacket.command.nSendersESN ,nSendersESN);
			if(hdlcPacket.command.nSendersESN != nSendersESN)
			{
				/////
				// not from our peer
				// so toss the packet
				/////
				//printf("Not from peer [%X] [%lX]\n", hdlcPacket.command.nSendersESN, nSendersESN);
				break;
			}
		}
		
		/////
		// is it an I frame?
		/////
		if(!bSframe)
		{
			/////
			// this is an I frame
			/////
			if(pHDLCControl->bConnected)
			{
				/////
				// we are connected
				/////
				/////
				// valid packet, so grab N(s) and N(r)
				/////
				int nNS = ((hdlcPacket.nHDLCControl&NS_MASK)>>NS_SHIFT);
				int nNR = ((hdlcPacket.nHDLCControl&NR_MASK)>>NR_SHIFT);
				int nNumAck = checkNR(nNR, pHDLCControl->nRHS, SEND_WINDOW_SIZE);
        int nNSTest = checkNS(nNS, pHDLCControl->nVR);
			
#ifdef LINUX
				if(bDiscardNext)
				{
					/////
					// discard this packet
					// to test missing packet recovery
					/////
					bDiscardNext = FALSE;
					break;
				}
#endif
        if(0 != nNSTest)
				{
						/////
						// received N(s) does not match the one we are expecting
						/////
					
						/////
						// if received N(s) is higher than the one we were expecting
						// so send a rej and discard the incoming packet
						// 
						// otherwise, just discard it
						/////
					if(0 < nNSTest)
					{
						hdlcSendSFrame(eInterface, eSPACKET_TYPE_REJ, FALSE);
					}
          break;
				}

				/////
				// now handle all of our sent packets
				// which are acknowledged by n(r) in this packet
				/////
				pHDLCControl->nRHS = (pHDLCControl->nRHS + nNumAck)%HDLC_WINDOW_SIZE;
				discardSentPackets(eInterface, pHDLCControl, nNumAck);
				if(pHDLCControl->nRHS != pHDLCControl->nVS)
				{
					resendAllPackets(eInterface, pHDLCControl);
				}

				
				pHDLCControl->nVR  =  (pHDLCControl->nVR+1)%HDLC_WINDOW_SIZE;	
				
				/////
				// so process the command
				// put it onto the received queue
				// so the command processor can access it
				/////
				pReceivedQueueEntry = (HDLC_QUEUE_ENTRY*)queuePop((QUEUE*)&hdlcFreeQueue);
				if(NULL == pReceivedQueueEntry)
				{
					/////
					// no more entries on the free queue
					// so take the oldest entry from the receive queue
					// this may happen during wireless bonding process
					// we expect to get a flood of packets
					/////
					pReceivedQueueEntry = (HDLC_QUEUE_ENTRY*)queuePop((QUEUE*)&hdlcReceivedQueue);			
				}
				
				if(NULL != pReceivedQueueEntry)
				{
					memcpy((unsigned char*)&pReceivedQueueEntry->hdlcPacket, (unsigned char*)&hdlcPacket, sizeof(hdlcPacket));
					pReceivedQueueEntry->eInterface = eInterface;
					queueAdd((QUEUE*)&hdlcReceivedQueue,(QUEUE_ENTRY*)pReceivedQueueEntry);
				}
				//commandProcessor(eInterface, &hdlcPacket);
			}
			else
			{
				/////
				// not connected, so respond with a U-Frame DM
				/////
				hdlcSendUFrame(eInterface, eUPACKET_TYPE_DM, FALSE);
			}
		}
		else
		{
			int nType;
			/////
			// nope, it is an S frame
			/////
			bUframe = (hdlcPacket.nHDLCControl & UF_BIT)?TRUE:FALSE;
			if(bUframe)
			{
				/////
				// process U frame
				/////
				nType = (hdlcPacket.nHDLCControl&UFLAG_TYPE1_MASK)>>UFLAG_TYPE1_SHIFT;
				nType |= (hdlcPacket.nHDLCControl&UFLAG_TYPE2_MASK)>>UFLAG_TYPE2_SHIFT;
				switch(nType)
				{
					case eUPACKET_TYPE_SABM:
						//printf("%s sabm\n", pHandheldOffset);
						/////
						// got a SABM
						// re-connect whether were were connected or not
						/////
						hdlcInitHDLCControlBlock((int)eInterface);
						pHDLCControl->bConnected = TRUE;
						hdlcSendUFrame(eInterface, eUPACKET_TYPE_UA, FALSE);
						nNewConnectionsCount[(int)eInterface]++;
						break;
					case eUPACKET_TYPE_DISC:
						//printf("%s disc\n", pHandheldOffset);
						/////
						// got a DM
						// remember disconnect
						// and begin to try to re-connect
						/////
						pHDLCControl->bConnected = FALSE;
						hdlcSendUFrame(eInterface, eUPACKET_TYPE_DM, FALSE);
						break;
					case eUPACKET_TYPE_DM:
						//printf("%s dm\n", pHandheldOffset);
						/////
						// got a DM
						// remember disconnect
						// and begin to try to re-connect
						/////
						pHDLCControl->bConnected = FALSE;
						hdlcSendUFrame(eInterface, eUPACKET_TYPE_SABM, FALSE);
						break;
					case eUPACKET_TYPE_UA:
						//printf("%s ua\n", pHandheldOffset);
						/////
						// got a UA
						// if we were disconnected before
						// then we are connected now
						/////
						if(!pHDLCControl->bConnected)
						{
							hdlcInitHDLCControlBlock((int)eInterface);
							pHDLCControl->bConnected = TRUE;
							nNewConnectionsCount[(int)eInterface]++;
						}
						break;
				}
			}
			else
			{
				if(pHDLCControl->bConnected)
				{
					/////
					// process S Frame
					/////
					int nNR = ((hdlcPacket.nHDLCControl&NR_MASK)>>NR_SHIFT);
					int nNumAck = checkNR(nNR, pHDLCControl->nRHS, SEND_WINDOW_SIZE);

					pHDLCControl->nRHS = (pHDLCControl->nRHS + nNumAck)%HDLC_WINDOW_SIZE;
					discardSentPackets(eInterface, pHDLCControl, nNumAck);
					//printf("nRHS[%d] nVS[%d]\n",pHDLCControl->nRHS,pHDLCControl->nVS);  
					if(pHDLCControl->nRHS != pHDLCControl->nVS)
					{
						resendAllPackets(eInterface, pHDLCControl);
					}				
					nType = (hdlcPacket.nHDLCControl&SFLAG_TYPE_MASK)>>SFLAG_TYPE_SHIFT;
					switch(nType)
					{
						case eSPACKET_TYPE_RR:
							/////
							// if poll flag is set
							// then send RR response
							// remember that the far end is ready
							/////
							if(bPF)
							{
								hdlcSendSFrame(eInterface, eSPACKET_TYPE_RR, FALSE);
							}
							break;
						case eSPACKET_TYPE_RNR:
							/////
							// if poll flag is set
							// then send RR response
							// remember that the farend is not ready (How?)
							/////
							if(bPF)
							{
								hdlcSendSFrame(eInterface, eSPACKET_TYPE_RR, FALSE);
							}
							break;
						case eSPACKET_TYPE_REJ:	
							/////
							// if we have the packet they are looking for
							// then resend all of our stored packets
							//
							// otherwise, disconnect so we can get back in sync again
							/////
							{

								BOOL bFound = FALSE;
								HDLC_QUEUE_ENTRY* pEntry = (HDLC_QUEUE_ENTRY*)queuePeek(&pHDLCControl->sentQueue, (QUEUE_ENTRY*)NULL);
								printf("REJ nNR[%d]\n", nNR);
								while(NULL != pEntry)
								{
									int nNS = ((pEntry->hdlcPacket.nHDLCControl&NS_MASK)>>NS_SHIFT);
									printf("REJ nNR[%d] nNS[%d]\n", nNR, nNS);
									if(nNR == nNS)
									{
										bFound = TRUE;
										break;
									}
									pEntry = (HDLC_QUEUE_ENTRY*)queuePeek(&pHDLCControl->sentQueue, (QUEUE_ENTRY*)pEntry);
								}
								
								if(FALSE == bFound)
								{
									/////
									// we don't have the one they are looking for
									// so disconnect and allow reconnection
									/////
									pHDLCControl->bConnected = FALSE;
									hdlcSendUFrame(eInterface, eUPACKET_TYPE_DISC, FALSE);
								}
							}
							break;
					}
				}
			}
		}
	}while(0);
}


//=======================================================================================
// Public interface
//=======================================================================================
BOOL hdlcIsConnectedTo(eINTERFACE eInterface)
{
	BOOL bConnected = FALSE;
	HDLCCONTROL *pHDLCControl = &hdlcControlBlock[eInterface];
	if(pHDLCControl->bConnected)
	{
		bConnected = TRUE;
	}
	return bConnected;
}
BOOL hdlcIsConnected()
{
	return hdlcIsConnectedTo(eConnectToInterface);
}
void hdlcDoWork()
{
	/////
	// we won't initiate anything
	// if we are a slave
	// so we can comply with half duplex communication
	/////
	if(!bWeAreSlave)
	{
		/////
		// we are master
		// look at only the interface we were told to connect to
		/////
		int nInterface = (int)eConnectToInterface;

		/////
		//for(nInterface=1; nInterface<3; nInterface++)
		{
			HDLCCONTROL *pHDLCControl = &hdlcControlBlock[nInterface];
			if(pHDLCControl->bConnected)
			{
				if(isTimerExpired(&pHDLCControl->packetTimer))
				{
					if(0 < --pHDLCControl->nNoTrafficRetries)
					{
						/////
						// nothing received in time
						// so send an RR
						// ask for a response
						/////
						hdlcSendSFrame((eINTERFACE)nInterface, eSPACKET_TYPE_RR, TRUE);
					}
					else
					{
						/////
						// no traffic received in awhile
						// go to disconnect mode
						/////
						pHDLCControl->bConnected = FALSE;
						hdlcSendUFrame((eINTERFACE)nInterface, eUPACKET_TYPE_DISC, FALSE);
					}
					startTimer(&pHDLCControl->packetTimer, NO_TRAFFIC_TIMEOUT);
				}
			}
			else
			{
				/////
				// not connected, so periodically send SABM
				/////
				if(isTimerExpired(&pHDLCControl->packetTimer))
				{
					hdlcSendUFrame((eINTERFACE)nInterface, eUPACKET_TYPE_SABM, TRUE);
					startTimer(&pHDLCControl->packetTimer, NO_TRAFFIC_TIMEOUT);
				}
			}
		}
	}
}

void hdlcInit(unsigned int nLocalESN, unsigned int nRemoteESN, BOOL bSlave, eINTERFACE eInterfaceToConnect)
{
	int i;

	bWeAreSlave = bSlave;
	eConnectToInterface = eInterfaceToConnect;
	nOurESN = nLocalESN;
	nSendersESN = nRemoteESN;
	memset((unsigned char*)nNewConnectionsCount, 0, sizeof(nNewConnectionsCount));
	printf("nOurESN[%08X] nSendersESN[%08X] bSlave[%d]\n", nOurESN, nSendersESN, bSlave);
	
	queueInitialize((QUEUE*)&hdlcReceivedQueue);
	queueInitialize((QUEUE*)&hdlcFreeQueue);
	memset((unsigned char*)&hdlcReceiveQueueEntries, 0, sizeof(hdlcReceiveQueueEntries));
	for(i=0;i<HDLC_MAX_RECEIVE_PACKETS; i++)
	{
		queueAdd((QUEUE*)&hdlcFreeQueue, (QUEUE_ENTRY*)&hdlcReceiveQueueEntries[i]);
	}
	if(!bWeAreSlave)
	{
		/////
		// send sabm on the specified interface
		/////
		hdlcInitHDLCControlBlock((int)eInterfaceToConnect);
		hdlcSendUFrame(eInterfaceToConnect, eUPACKET_TYPE_SABM, TRUE);
		hdlcControlBlock[(int)eInterfaceToConnect].bConnected = FALSE;
		startTimer(&hdlcControlBlock[(int)eInterfaceToConnect].packetTimer, NO_TRAFFIC_TIMEOUT);

	}
}
void hdlcSetSendersESN(unsigned int nRemoteESN)
{
	nSendersESN = nRemoteESN;
}
void hdlcSendResponse(eINTERFACE eInterface, HDLCPACKET* pHDLCPacket)
{
	pHDLCPacket->nHDLCControl &= ~PF_BIT;
	hdlcSendPacket(eInterface, pHDLCPacket);
}

void hdlcSendCommand(eINTERFACE eInterface, HDLCPACKET* pHDLCPacket)
{
	pHDLCPacket->nHDLCControl |= PF_BIT;
	hdlcSendPacket(eInterface, pHDLCPacket);
}

BOOL hdlcReceivePacket(HDLCPACKET* pReceivedPacket, eINTERFACE* peInterface)
{
		BOOL bRetVal = FALSE;
		HDLC_QUEUE_ENTRY *pReceivedQueueEntry = (HDLC_QUEUE_ENTRY*)queuePop((QUEUE*)&hdlcReceivedQueue);
		if(NULL != pReceivedQueueEntry)
		{
			memcpy((unsigned char*)pReceivedPacket, (unsigned char*)&pReceivedQueueEntry->hdlcPacket, sizeof(HDLCPACKET));
			*peInterface = pReceivedQueueEntry->eInterface;
			memset((unsigned char*)pReceivedQueueEntry, 0, sizeof(HDLC_QUEUE_ENTRY));
			queueAdd((QUEUE*)&hdlcFreeQueue,(QUEUE_ENTRY*)pReceivedQueueEntry);
			bRetVal = TRUE;
		}
		return bRetVal;
}

