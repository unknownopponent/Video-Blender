#pragma once

//#define WIN32_LEAN_AND_MEAN
//#include <Windows.h>

#include <threads.h>

typedef struct Thread
{
	void* function;
	void* params;

	//HANDLE h;
	thrd_t id;
} Thread;

//struct must be filled with data
char launch_thread(Thread* thread);

char join_thread(Thread* thread);
