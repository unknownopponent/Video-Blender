#pragma once

#ifdef _WIN32
#include <windows.h>
#else
#include <threads.h>
#endif

typedef struct Mutex
{
#ifdef _WIN32
	HANDLE h;
#else
	mtx_t id;
#endif
} Mutex;

char create_mutex(Mutex* mutex);

char lock_mutex(Mutex* mutex);
char unlock_mutex(Mutex* mutex);

char delete_mutex(Mutex* mutex);
