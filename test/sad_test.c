
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../src/sad/sad.h"

char test()
{
	uint8_t block1[4096];
	uint8_t block2[4096];

	for (size_t i = 0; i < 4096; i++)
	{
		block1[i] = rand();
		block2[i] = rand();
		//block1[i] = 1;
		//block2[i] = i%32;
	}

	volatile uint64_t(*sad)(void*, void*, uint64_t);
	volatile uint64_t(*sad_square)(void*, void*, uint64_t, uint64_t) = (void*)sad_u8_square;
	volatile uint64_t(*sad_rectangle)(void*, void*, uint64_t, uint64_t, uint64_t) = (void*)sad_u8_rectangle;

	puts("tests");
	
	void* squares[] = {
		sad_u8_2x2_sse2,
		sad_u8_4x4_sse2,
		sad_u8_8x8_sse2,
		sad_u8_16x16_ssse3,
		sad_u8_32x32_ssse3,
		sad_u8_32x32_avx2
	};

	char* squares_names[] = {
		"sad_u8_2x2_sse2",
		"sad_u8_4x4_sse2",
		"sad_u8_8x8_sse2",
		"sad_u8_16x16_ssse3",
		"sad_u8_32x32_ssse3",
		"sad_u8_32x32_avx2"
	};

	uint64_t square_widths[] = {
		2,
		4,
		8,
		16,
		32,
		32
	};

	for (size_t i = 0; i < 6; i++)
	{
		uint64_t ref = sad_square(block1, block2, 32, square_widths[i]);

		sad = squares[i];

		uint64_t res = sad(block1, block2, 32);

		if (res != ref)
		{
			fprintf(stderr, "test failled, %s returned %llu, reference %llu\n", squares_names[i], res, ref);
			return 1;
		}

		printf("%s passed\n", squares_names[i]);
	}

	void* rectangles[] = {
		sad_u8_6x2_sse2,
		sad_u8_12x4_ssse3,
		sad_u8_24x8_ssse3,
		sad_u8_24x8_avx2,
		sad_u8_48x16_ssse3,
		sad_u8_48x16_avx2,
		sad_u8_96x32_ssse3,
		sad_u8_96x32_avx2
	};

	char* rectangles_names[] = {
		"sad_u8_6x2_sse2",
		"sad_u8_12x4_ssse3",
		"sad_u8_24x8_ssse3",
		"sad_u8_24x8_avx2",
		"sad_u8_48x16_ssse3",
		"sad_u8_48x16_avx2",
		"sad_u8_96x32_ssse3",
		"sad_u8_96x32_avx2"
	};

	uint64_t rectangle_widths[] = {
		6,
		12,
		24,
		24,
		48,
		48,
		96,
		96
	};

	uint64_t rectangle_heights[] = {
		2,
		4,
		8,
		8,
		16,
		16,
		32,
		32,
	};

	for (size_t i = 0; i < 8; i++)
	{
		uint64_t ref = sad_rectangle(block1, block2, 96, rectangle_widths[i], rectangle_heights[i]);

		sad = rectangles[i];

		uint64_t res = sad(block1, block2, 96);

		if (res != ref)
		{
			fprintf(stderr, "test failled, %s returned %llu, reference %llu\n", rectangles_names[i], res, ref);
			return 1;
		}

		printf("%s passed\n", rectangles_names[i]);
	}

	puts("tests passed\n");

	return 0;
}

#define ITERATIONS 100000000

char benchmark()
{
	uint8_t block1[4096];
	uint8_t block2[4096];

	for (size_t i = 0; i < 4096; i++)
	{
		block1[i] = rand();
		block2[i] = rand();
	}

	volatile uint64_t(*sad)(void*, void*, uint64_t);
	volatile uint64_t(*sad_square)(void*, void*, uint64_t, uint64_t);
	volatile uint64_t(*sad_rectangle)(void*, void*, uint64_t, uint64_t, uint64_t);

	printf("benchmark %d iterations\n", ITERATIONS);

	clock_t ref;
	uint64_t diff;

	#define SQUARE_COUNT 5
	#define SQUARE_VERSION_COUNT 2

	void* squares[SQUARE_COUNT][SQUARE_VERSION_COUNT] = {
		{sad_u8_2x2_sse2, 0},
		{sad_u8_4x4_sse2, 0},
		{sad_u8_8x8_sse2, 0},
		{sad_u8_16x16_ssse3, 0},
		{sad_u8_32x32_ssse3, sad_u8_32x32_avx2}
	};

	char* squares_names[SQUARE_COUNT][SQUARE_VERSION_COUNT] = {
		{"sad_u8_2x2_sse2", 0},
		{"sad_u8_4x4_sse2", 0},
		{"sad_u8_8x8_sse2", 0},
		{"sad_u8_16x16_ssse3", 0},
		{"sad_u8_32x32_ssse3", "sad_u8_32x32_avx2"}
	};

	uint64_t square_widths[] = {
		2,
		4,
		8,
		16,
		32,
	};

	for (size_t i = 0; i < SQUARE_COUNT; i++)
	{
		sad_square = (void*)sad_u8_square;

		ref = clock();

		for (size_t j = 0; j < ITERATIONS; j++)
			sad_square(block1, block2, 32, square_widths[i]);

		diff = clock() - ref;

		printf("sad_u8_square %llu, %f s => %llu sad/s\n",
			square_widths[i],
			(double)diff / (double)CLOCKS_PER_SEC,
			(uint64_t)((double)ITERATIONS * (double)CLOCKS_PER_SEC / (double)diff)
		);

		for (size_t j = 0; j < SQUARE_VERSION_COUNT; j++)
		{
			sad = squares[i][j];
			if (sad == 0)
				continue;

			ref = clock();

			for (size_t j = 0; j < ITERATIONS; j++)
				sad(block1, block2, 32);

			diff = clock() - ref;

			printf("%s %llu, %f s => %llu sad/s\n",
				squares_names[i][j],
				square_widths[i],
				(double)diff / (double)CLOCKS_PER_SEC,
				(uint64_t)((double)ITERATIONS * (double)CLOCKS_PER_SEC / (double)diff)
			);
		}

		puts("");
	}

	#define RECTANGLE_COUNT 5
	#define RECTANGLE_VERSION_COUNT 2

	void* rectangles[RECTANGLE_COUNT][RECTANGLE_VERSION_COUNT] = {
		{sad_u8_6x2_sse2, 0},
		{sad_u8_12x4_ssse3, 0},
		{sad_u8_24x8_ssse3, sad_u8_24x8_avx2},
		{sad_u8_48x16_ssse3, sad_u8_48x16_avx2},
		{sad_u8_96x32_ssse3, sad_u8_96x32_avx2}
	};

	char* rectangles_names[RECTANGLE_COUNT][RECTANGLE_VERSION_COUNT] = {
		{"sad_u8_6x2_sse2", 0},
		{"sad_u8_12x4_ssse3", 0},
		{"sad_u8_24x8_ssse3", "sad_u8_24x8_avx2"},
		{"sad_u8_48x16_ssse3", "sad_u8_48x16_avx2"},
		{"sad_u8_96x32_ssse3", "sad_u8_96x32_avx2"}
	};

	uint64_t rectangle_widths[] = {
		6,
		12,
		24,
		48,
		96
	};

	uint64_t rectangle_heights[] = {
		2,
		4,
		8,
		16,
		32
	};

	for (size_t i = 0; i < RECTANGLE_COUNT; i++)
	{
		sad_rectangle = (void*)sad_u8_rectangle;

		ref = clock();

		for (size_t j = 0; j < ITERATIONS; j++)
			sad_rectangle(block1, block2, 96, rectangle_widths[i], rectangle_heights[i]);

		diff = clock() - ref;

		printf("sad_u8_rectangle %llux%llu, %f s => %llu sad/s\n",
			rectangle_widths[i],
			rectangle_heights[i],
			(double)diff / (double)CLOCKS_PER_SEC,
			(uint64_t)((double)ITERATIONS * (double)CLOCKS_PER_SEC / (double)diff)
		);

		for (size_t j = 0; j < RECTANGLE_VERSION_COUNT; j++)
		{
			sad = rectangles[i][j];
			if (sad == 0)
				continue;

			ref = clock();

			for (size_t j = 0; j < ITERATIONS; j++)
				sad(block1, block2, 96);

			diff = clock() - ref;

			printf("%s %llux%llu, %f s => %llu sad/s\n",
				rectangles_names[i][j],
				rectangle_widths[i],
				rectangle_heights[i],
				(double)diff / (double)CLOCKS_PER_SEC,
				(uint64_t)((double)ITERATIONS * (double)CLOCKS_PER_SEC / (double)diff)
			);
		}

		puts("");
	}

	return 0;
}

int main()
{
	if (test())
	{
		fprintf(stderr, "tests failled\n");
		return 1;
	}
	if (benchmark())
	{
		fprintf(stderr, "benchmarks failled\n");
		return 1;
	}
	return 0;
}