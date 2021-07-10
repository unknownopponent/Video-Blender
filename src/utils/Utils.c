#include "Utils.h"

#include <string.h>

#include <windows.h>

int file_exists(char* path)
{
	WIN32_FIND_DATA data;
	if (FindFirstFileA(path, &data) == INVALID_HANDLE_VALUE)
		return 0;
	return 1;
}

int str_equals_case_insensitive(char* str1, char* str2)
{
	int len = strlen(str1);
	for (int i = 0; i < len; i++)
	{
		if (str1[i] >= 'A' && str1[i] <= 'Z')
		{
			if ((str1[i] + 'a' - 'A') == str2[i])
			{
				continue;
			}
		}
		if (str1[i] >= 'a' && str1[i] <= 'z')
		{
			if ((str1[i] - 'a' + 'A') == str2[i])
			{
				continue;
			}
		}
		if (str1[i] == str2[i])
		{
			continue;
		}
		return 0;
	}

	return 1;
}

int cstr_to_int(char* str, int* result)
{
	int len = strlen(str);
	int tmp = 0;
	char negative = str[0] == '-';
	*result = 0;

	for (int i = negative; i < len; i++)
	{
		if (str[i] < '0' || str[i] > '9')
			return 1;
		tmp = str[i] - '0';
		for (int j = i+1; j < len; j++)
		{
			tmp *= 10;
		}
		*result += tmp;
	}
	if ( negative )
		*result = -*result;

	return 0;
}

int char_index_in_str(char* str, char c, int* index)
{
	int len = strlen(str);

	for (int i = 0; i < len; i++)
	{
		if (str[i] == c)
		{
			*index = i;
			return 0;
		}
	}

	return 1;
}