#pragma once

//#define WIN32_LEAN_AND_MEAN
//#include <windows.h>

#include <threads.h>

typedef struct Mutex
{
	mtx_t id;
	//HANDLE h;
} Mutex;

char create_mutex(Mutex* mutex);

char lock_mutex(Mutex* mutex);
char unlock_mutex(Mutex* mutex);

char delete_mutex(Mutex* mutex);
