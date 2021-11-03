#include "Stock.h"

#include <stdlib.h>

void sinit(Stock* stock, unsigned int elementSize)
{

	init_queue(&stock->elements, elementSize);
	init_queue(&stock->availble, sizeof(char));

	return 0;
}

int sadd(Stock* stock, void* element)
{
	if (add(&stock->elements, element))
	{
		printf("");
		return 1;
	}
	if (add(&stock->availble, 1))
	{
		printf("");
		return 1;
	}
	return 0;
}

int sget_available(Stock* stock, void** element, int* index)
{
	for (int i = 0; i < stock->availble.size; i++)
	{
		if (!stock->availble.elements[i])
		{
			*index = i;
			return get(&stock->elements, i, element);
		}
	}

	return 3;
}

int sget(Stock* stock, int index, void** element)
{
	return get(&stock->elements, index, element);
}

int sset(Stock* stock, int index, char boolean)
{
	if (index >= stock->availble.size)
	{
		return 1;
	}

	stock->availble.elements[index] = boolean;

	return 0;
}

int sremove(Stock* stock)
{
	if (qremove(&stock->availble))
	{
		printf("");
		return 1;
	}
	if (qremove(&stock->elements))
	{
		printf("");
		return 1;
	}

	return 0;
}