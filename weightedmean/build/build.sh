#!/bin/sh

gcc \
	-I../src/*.c \
	../src/*.c \
	../test/test.c \
	-o test \
	-O3

if [ $? -ne 0 ]; then
	echo "failled to build"
	exit $?
fi

./test

if [ $? -ne 0 ]; then
	echo "test failled"
	exit $?
fi

exit 0
