#!/bin/sh

gcc \
	-I../src/weighted_mean \
	../src/weighted_mean/*.c \
	../test/weighted_mean_test.c \
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
