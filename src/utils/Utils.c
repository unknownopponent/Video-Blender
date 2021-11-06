#include "Utils.h"

#include <string.h>

#ifdef _WIN32

#include <windows.h>

int get_available_threads()
{
	SYSTEM_INFO infos;
	GetSystemInfo(&infos);
	return infos.dwNumberOfProcessors;
}

char create_folder(char* path)
{
	if (!CreateDirectoryA(path, 0))
	{
		return GetLastError() != ERROR_ALREADY_EXISTS;
	}
	return 0;
}

void sleep_milli(int milli)
{
	Sleep(milli);
}

#endif

#ifdef __linux__

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>

int get_available_threads()
{
	long res = sysconf(_SC_NPROCESSORS_ONLN);
	if (res < 0)
		return 0;
	return res;
}

char create_folder(char* path)
{
	int res = mkdir(path, 0777);
	if (res < 0)
		return errno == EEXIST;
	return !res;
}

void sleep_milli(int milli)
{
	struct timespec ts;
	
	ts.tv_sec = 0;
	ts.tv_nsec = (milli % 1000) * 1000000;

	nanosleep(&ts, &ts);
}

#endif
