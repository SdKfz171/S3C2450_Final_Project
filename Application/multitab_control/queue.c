#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

void QueueInit(Queue * q)
{
	q->front = NULL;
	q->rear = NULL;
	q->size = 0;
}

int IsEmpty(Queue * q)
{
	if (q->size == 0)
		return 1;
	else
		return 0;
}

void Enqueue(Queue * q, Data data)
{
	Node * newNode = (Node*)malloc(sizeof(Node));
	newNode->next = NULL;
	newNode->data = data;

	if (IsEmpty(q))
	{
		q->front = newNode;
		q->rear = newNode;
	}
	else
	{
		q->rear->next = newNode;
		q->rear = newNode;
	}

	(q->size)++;
}

Data Dequeue(Queue * q)
{
	Node * delNode;
	Data retData;

	if (IsEmpty(q))
	{
		printf("Queue Memory Error!");
		exit(-1);
	}

	delNode = q->front;
	retData = delNode->data;
	q->front = q->front->next;

	free(delNode);
	
	(q->size)--;

	return retData;
}

Data Peek(Queue * q)
{
	if (IsEmpty(q))
	{
		printf("Queue Memory Error!");
		exit(-1);
	}

	return q->front->data;
}