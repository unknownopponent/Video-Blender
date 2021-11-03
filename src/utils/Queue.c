#include "Queue.h"

#include <stdlib.h>

void init_queue(Queue* queue, int elementSize)
{
	queue->elements = 0;
	queue->size = 0;
	queue->elementSize = elementSize;
}

int add(Queue* queue, void* element)
{
	if (queue->size == 0)
	{
		queue->elements = malloc(queue->elementSize);
		if (!queue->elements)
			return 1;
		memcpy(queue->elements, &element, queue->elementSize);
		queue->size = 1;
	}
	else
	{
		char* tmp = malloc(queue->elementSize * (queue->size + 1));
		if (!tmp)
			return 1;
		int size = queue->elementSize * queue->size;
		memcpy(tmp, queue->elements, size);
		memcpy(&tmp[size], &element, queue->elementSize);

		free(queue->elements);
		queue->elements = tmp;
		queue->size += 1;
	}
	return 0;
}

int get(Queue* queue, int index, void** element)
{
	if (queue->size == 0)
		return 1;
	if (queue->size <= index)
		return 2;

	memcpy(element, &queue->elements[index * queue->elementSize], queue->elementSize);

	return 0;
}

int front(Queue* queue, void** element)
{
	return get(queue, 0, element);
}

int qremove(Queue* queue)
{
	if (queue->size == 0)
		return 1;

	if (queue->size == 1)
	{
		free(queue->elements);
		queue->size = 0;
	}
	else
	{
		char* tmp = malloc(queue->elementSize * (queue->size - 1));
		if (!tmp)
			return 2;
		memcpy(tmp, &queue->elements[queue->elementSize], queue->elementSize * (queue->size - 1));
		free(queue->elements);
		queue->elements = tmp;
		queue->size -= 1;
	}
	
	return 0;
}

void empty(Queue* queue)
{
	if (queue->size <= 0)
		return;

	free(queue->elements);
	queue->size = 0;
}