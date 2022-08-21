@echo off

gcc ^
	-I ../src/*.h ^
	../src/*.c ^
	../test/test.c ^
	-o test.exe ^
	-O3


if %errorlevel% neq 0 (
	echo build failled
	exit /b %errorlevel%
)

test.exe

if %errorlevel% neq 0 (
	echo test failled
	exit /b %errorlevel%
)
	
exit /b 0