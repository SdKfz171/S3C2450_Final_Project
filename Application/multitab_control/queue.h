#ifndef __LB_QUEUE_H__
#define __LB_QUEUE_H__

typedef char * Data;

typedef struct _node
{
	Data data;
	struct _node * next;
} Node;

typedef struct 
{
	Node * front;
	Node * rear;

	int size;
} Queue;


void QueueInit(Queue * q);
int IsEmpty(Queue * q);

void Enqueue(Queue * q, Data data);
Data Dequeue(Queue * q);
Data Peek(Queue * q);

#endif