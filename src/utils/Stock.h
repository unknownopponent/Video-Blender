#pragma once

#include "Queue.h"

typedef struct Stock
{
	Queue elements;
	Queue availble;
} Stock;

void sinit(Stock* stock, unsigned int elementSize);

int sadd(Stock* stock, void* element);
int sget_available(Stock* stock, void** element, int* index);
int sget(Stock* stock, int index, void** element);

int sset(Stock* stock, int index, char boolean);

int sremove(Stock* stock);