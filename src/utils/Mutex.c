#include "Mutex.h"

#ifdef _WIN32

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

#else

char create_mutex(Mutex* mutex)
{
	if (mtx_init(&mutex->id, mtx_plain) == thrd_success)
		return 0;
	return 1;
}

char lock_mutex(Mutex* mutex)
{
	return mtx_lock(&mutex->id) != thrd_success;
}

char unlock_mutex(Mutex* mutex)
{
	return mtx_unlock(&mutex->id) != thrd_success;
}

char delete_mutex(Mutex* mutex)
{
	mtx_destroy(&mutex->id);
	return 0;
}

#endif