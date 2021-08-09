#ifndef HDLC_H
#define HDLC_H
#include "queue.h"
//#include "serial.h"

#define SHOW_HDLC_TRAFFIC

#define NO_TRAFFIC_TIMEOUT 1000
#define NO_TRAFFIC_RETRIES 5
typedef struct hdlcSendQueueEntry
{
	QUEUE_ENTRY* pNext;
	eINTERFACE eInterface;
	HDLCPACKET hdlcPacket;
}HDLC_QUEUE_ENTRY;

typedef struct hdlc_queue
{
	QUEUE head;
}HDLC_QUEUE;

#define HDLC_MAX_RECEIVE_PACKETS 20

typedef struct hdlcControl
{
	BOOL bConnected;
	/////
	// the next packet we expect to receive
	/////
	unsigned char nVR;
	
	/////
	// the last packet we sent
	/////
	unsigned char nVS;
	
	/////
	// number to the right of the last acknowledged by the peer
	/////
	unsigned char nRHS;
	
	unsigned char nNoTrafficRetries;
	QUEUE sentQueue;
	QUEUE freeQueue;
	HDLC_QUEUE_ENTRY hdlcSentPacketHistory[SEND_WINDOW_SIZE];
	TIMERCTL packetTimer;
}HDLCCONTROL;

int hdlcGetNewConnectionsCount(eINTERFACE eInterface);
void hdlcProcessor(eINTERFACE eInterface, unsigned char* pPacket, int nPacketLength);
BOOL hdlcIsConnectedTo(eINTERFACE eInterface);
BOOL hdlcIsConnected(void);
void hdlcDoWork(void);
void hdlcInit(unsigned int nOurESN, unsigned int nSendersESN, BOOL bSlave, eINTERFACE eInterfaceToConnect);
void hdlcSetSendersESN(unsigned int nRemoteESN);
void hdlcSendResponse(eINTERFACE eInterface, HDLCPACKET* pHDLCPacket);
void hdlcSendCommand(eINTERFACE eInterface, HDLCPACKET* pHDLCPacket);
BOOL hdlcReceivePacket(HDLCPACKET* pReceivedPacket, eINTERFACE* peInterface);
void hdlcSendPacket(eINTERFACE eInterface, HDLCPACKET* pCommand);

#endif		// HDLC_H
