#include <stdlib.h>
#include <stdio.h>
#include "queue.h"


void queueInitialize(QUEUE* pQueue)
{
	pQueue->head = NULL;
}

void queueAdd(QUEUE* pQueue, QUEUE_ENTRY* pEntry)
{
	int nLimitCount = 10000;
	QUEUE_ENTRY* pNode = pQueue->head;
	pEntry->next = NULL;
	if(NULL == pNode)
	{
		pQueue->head = pEntry;
		//printf("queueAdd\n");
	}
	else
	{
		if(pNode != pEntry)
		{
			while(NULL != pNode->next)
			{
				if(0 >= nLimitCount--)
				{
					pNode = NULL;
					break;
				}
				if(pNode->next == pEntry)
				{
					pNode = NULL;
					break;
				}
				pNode = pNode->next;
			}
			if(NULL != pNode)
			{
				pNode->next = pEntry;
			}
		}
		else
		{
			printf("queueAdd-2 Loop Detected\r\n");
		}
	}
}

QUEUE_ENTRY* queuePop(QUEUE* pQueue)
{
	QUEUE_ENTRY* pEntry = pQueue->head;
	
	if(NULL != pEntry)
	{
		pQueue->head = pEntry->next;
		pEntry->next = NULL;
	}

	return pEntry;
}

QUEUE_ENTRY* queuePeek(QUEUE* pQueue, QUEUE_ENTRY* pPrevEntry)
{
   QUEUE_ENTRY* pEntry = NULL;
   if(NULL == pPrevEntry)
   {
      pEntry = pQueue->head;
   }
   else
   {
      pEntry = pPrevEntry->next;
   }
   return pEntry;
}

