#pragma once

#include <stdlib.h>
#include <stdio.h>

inline void oom(uint64_t size)
{
	fprintf(stderr, "out of memory %ulld\n", size);
	exit(1);
}