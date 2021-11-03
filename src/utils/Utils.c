#include "Utils.h"

#include <string.h>

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