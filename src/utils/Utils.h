#pragma once

int file_exists(char* path);

int str_equals_case_insensitive(char* str1, char* str2);

int cstr_to_int(char* str, int* result);

int char_index_in_str(char* str, char c, int* index);