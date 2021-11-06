#pragma once

#ifdef _WIN32 
#include <Windows.h>
#else
#include <threads.h>
#endif

typedef struct Thread
{
	void* function;
	void* params;

#ifdef _WIN32
	HANDLE h;
#else
	thrd_t id;
#endif
} Thread;

//struct must be filled with data(function and params)
char launch_thread(Thread* thread);

char join_thread(Thread* thread);
