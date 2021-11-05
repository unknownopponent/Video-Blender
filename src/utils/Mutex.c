#include "Mutex.h"

char create_mutex(Mutex* mutex)
{
	mutex->h = CreateMutexA(0, 0, 0);

	if (mutex->h)
		return 0;
	return 1;
}

char lock_mutex(Mutex* mutex)
{
	return WaitForSingleObject(mutex->h, INFINITE);
}

char unlock_mutex(Mutex* mutex)
{
	return ReleaseMutex(mutex->h);
}

char delete_mutex(Mutex* mutex)
{
	if (!mutex->h)
		return 1;
	char tmp = CloseHandle(mutex->h);
	mutex->h = 0;
	return tmp;
}