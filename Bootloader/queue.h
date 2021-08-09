#ifndef QUEUE_H
#define QUEUE_H
typedef struct queue_entry
{
	struct queue_entry* next;
	//unsigned char data[1];
}QUEUE_ENTRY;
typedef struct queue
{
	QUEUE_ENTRY *head;
}QUEUE;
void queueInitialize(QUEUE* pQueue);
void queueAdd(QUEUE* pQueue, QUEUE_ENTRY* pEntry);
QUEUE_ENTRY* queuePop(QUEUE* pQueue);
QUEUE_ENTRY* queuePeek(QUEUE* pQueue, QUEUE_ENTRY* pPrevEntry);
#endif		// QUEUE_H
