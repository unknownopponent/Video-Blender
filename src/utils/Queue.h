#pragma once

typedef struct Queue
{
	char* elements;
	unsigned int size;
	unsigned int elementSize;
} Queue;

void init(Queue* queue, int elementSize);

int add(Queue* queue, void* element);
int get(Queue* queue, int index, void** element);
int front(Queue* queue, void** element);
int qremove(Queue* queue);

void empty(Queue* queue);