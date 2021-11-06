#include "Thread.h"

#include <stdio.h>

#ifdef _WIN32

char launch_thread(Thread* thread)
{
	thread->h = CreateThread(0, 0, thread->function, thread->params, 0, 0);

	if (thread->h)
		return 0;
	return 1;
}

char join_thread(Thread* thread)
{
	if (!thread->h)
	{
		fprintf(stderr, "trying to join a null thread\n");
		return 1;
	}

	WaitForSingleObject(thread->h, INFINITE);
	CloseHandle(thread->h);
	thread->h = 0;
	return 0;
}

#else

char launch_thread(Thread* thread)
{
	if (thrd_create(&thread->id, thread->function, thread->params) == thrd_success)
		return 0;
	return 1;
}

char join_thread(Thread* thread)
{
	if (!thread->id)
	{
		fprintf(stderr, "trying to join a null thread\n");
		return 1;
	}

	int res;

	if (thrd_join(thread->id, &res) != thrd_success)
		return 1;

	thread->id = 0;
	return 0;
}

#endif