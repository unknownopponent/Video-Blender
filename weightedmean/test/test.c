
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

#include "../src/weighted_mean.h"

char test()
{
	void volatile (*weightedmean_u8)(void**, void*, uint64_t, uint64_t, void*);

	uint8_t reference[512];

	uint8_t block1[512];
	uint8_t block2[512];
	uint8_t block3[512];

	uint8_t* blocks[3] = { block1, block2, block3 };

	for (size_t i = 0; i < 512; i++)
		blocks[0][i] = i % 256;
	for (size_t i = 0; i < 512; i++)
		blocks[1][i] = (512 - i) % 256;
	for (size_t i = 0; i < 512; i++)
		blocks[2][i] = (i * 23) % 256;

	uint8_t* result = malloc(512 + 33);
	if (!result)
	{
		fprintf(stderr, "out of memory\n");
		return 1;
	}
	size_t offset = 32 - ((size_t)result & 31) + 1;

	int32_t weightsi32[3] = { 1, 2, 5 };
	int64_t weightsi64[3] = { 1, 2, 5 };
	float weightsf[3] = { 1.0f, 2.0f, 5.0f };
	double weightsd[3] = { 1.0, 2.0, 5.0 };

	weightedmean_u8 = (void*)weightedmean_u8_i64;

	weightedmean_u8((void*)blocks, weightsi64, 3, 512, reference);

	size_t count = 18;

	void* functions[] = {
		weightedmean_u8_i64,
		weightedmean_u8_f64,
		weightedmean_u8_i64_x86,
		weightedmean_u8_f64_x86,
		weightedmean_u8_i64_sse2_nt,
		weightedmean_u8_f64_sse2_nt,
		weightedmean_u8_i32_sse41_shift,
		weightedmean_u8_i32_sse41_nt_shift,
		weightedmean_u8_f32_sse41,
		weightedmean_u8_f32_sse41_nt,
		weightedmean_u8_f32_fma128,
		weightedmean_u8_f32_fma128_nt,
		weightedmean_u8_i32_avx2_shift,
		weightedmean_u8_i32_avx2_nt_shift,
		weightedmean_u8_f32_avx2,
		weightedmean_u8_f32_avx2_nt,
		weightedmean_u8_f32_fma_avx2,
		weightedmean_u8_f32_fma_avx2_nt
	};

	char* functions_names[] = {
		"weightedmean_u8_i64",
		"weightedmean_u8_f64",
		"weightedmean_u8_i64_x86",
		"weightedmean_u8_f64_x86",
		"weightedmean_u8_i64_sse2_nt",
		"weightedmean_u8_f64_sse2_nt",
		"weightedmean_u8_i32_sse41_shift",
		"weightedmean_u8_i32_sse41_nt_shift",
		"weightedmean_u8_f32_sse41",
		"weightedmean_u8_f32_sse41_nt",
		"weightedmean_u8_f32_fma128",
		"weightedmean_u8_f32_fma128_nt",
		"weightedmean_u8_i32_avx2_shift",
		"weightedmean_u8_i32_avx2_nt_shift",
		"weightedmean_u8_f32_avx2",
		"weightedmean_u8_f32_avx2_nt",
		"weightedmean_u8_f32_fma_avx2",
		"weightedmean_u8_f32_fma_avx2_nt"
	};

	void* weights[] = {
		weightsi64,
		weightsd,
		weightsi64,
		weightsd,
		weightsi64,
		weightsd,
		weightsi32,
		weightsi32,
		weightsf,
		weightsf,
		weightsf,
		weightsf,
		weightsi32,
		weightsi32,
		weightsf,
		weightsf,
		weightsf,
		weightsf
	};

	puts("tests");

	for (size_t i = 0; i < count; i++)
	{
		memset(result, 0, 512 + 32);

		weightedmean_u8 = functions[i];

		weightedmean_u8((void*)blocks, weights[i], 3, 512, (void*)(result + offset));

		if (memcmp(reference, result + offset, 512))
		{
			printf("%s different results\n", functions_names[i]);
			free(result);
			return 1;
		}

		printf("%s passed\n", functions_names[i]);
	}

	puts("tests passed\n");

	return 0;
}

#define ITERATIONS 100
#define FRAME_SIZE 1920*1080*3
#define FRAME_COUNT 8

char benchmark()
{
	int32_t weightsi32[13] = { 1, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048 };
	int64_t weightsi64[13] = { 1, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048 };
	float weightsf[13] = { 1.0f, 1.0f, 2.0f, 4.0f, 8.0f, 16.0f, 32.0f, 64.0f, 128.0f, 256.0f, 512.0f, 1024.0f, 2048.0f };
	double weightsd[13] = { 1.0, 1.0, 2.0, 4.0, 8.0, 16.0, 32.0, 64.0, 128.0, 256.0, 512.0, 1024.0, 2048.0 };

	uint8_t* frames[FRAME_COUNT];
	for (size_t i = 0; i < FRAME_COUNT; i++)
	{
		frames[i] = malloc(FRAME_SIZE);
		if (!frames[i])
		{
			printf("out of memory\n");
			return 1;
		}
	}

	uint8_t* result = malloc(FRAME_SIZE);
	if (!result)
	{
		fprintf(stderr, "out of memory\n");
		return 1;
	}

	printf("benchmark %llu bytes %d=>1 %d iterations\n", FRAME_SIZE, FRAME_COUNT, ITERATIONS);

	clock_t ref;

	uint64_t diff;

	size_t count = 18;

	void* functions[] = {
		weightedmean_u8_i64,
		weightedmean_u8_f64,
		weightedmean_u8_i64_x86,
		weightedmean_u8_f64_x86,
		weightedmean_u8_i64_sse2_nt,
		weightedmean_u8_f64_sse2_nt,
		weightedmean_u8_i32_sse41_shift,
		weightedmean_u8_i32_sse41_nt_shift,
		weightedmean_u8_f32_sse41,
		weightedmean_u8_f32_sse41_nt,
		weightedmean_u8_f32_fma128,
		weightedmean_u8_f32_fma128_nt,
		weightedmean_u8_i32_avx2_shift,
		weightedmean_u8_i32_avx2_nt_shift,
		weightedmean_u8_f32_avx2,
		weightedmean_u8_f32_avx2_nt,
		weightedmean_u8_f32_fma_avx2,
		weightedmean_u8_f32_fma_avx2_nt
	};

	char* functions_names[] = {
		"weightedmean_u8_i64",
		"weightedmean_u8_f64",
		"weightedmean_u8_i64_x86",
		"weightedmean_u8_f64_x86",
		"weightedmean_u8_i64_sse2_nt",
		"weightedmean_u8_f64_sse2_nt",
		"weightedmean_u8_i32_sse41_shift",
		"weightedmean_u8_i32_sse41_nt_shift",
		"weightedmean_u8_f32_sse41",
		"weightedmean_u8_f32_sse41_nt",
		"weightedmean_u8_f32_fma128",
		"weightedmean_u8_f32_fma128_nt",
		"weightedmean_u8_i32_avx2_shift",
		"weightedmean_u8_i32_avx2_nt_shift",
		"weightedmean_u8_f32_avx2",
		"weightedmean_u8_f32_avx2_nt",
		"weightedmean_u8_f32_fma_avx2",
		"weightedmean_u8_f32_fma_avx2_nt"
	};

	void* weights[] = {
		weightsi64,
		weightsd,
		weightsi64,
		weightsd,
		weightsi64,
		weightsd,
		weightsi32,
		weightsi32,
		weightsf,
		weightsf,
		weightsf,
		weightsf,
		weightsi32,
		weightsi32,
		weightsf,
		weightsf,
		weightsf,
		weightsf
	};

	void volatile (*weightedmean_u8)(void**, void*, uint64_t, uint64_t, void*);

	for (size_t i = 0; i < count; i++)
	{
		weightedmean_u8 = functions[i];

		ref = clock();

		for (size_t j = 0; j < ITERATIONS; j++)
			weightedmean_u8((void*)frames, weights[i], FRAME_COUNT, FRAME_SIZE, result);

		diff = clock() - ref;

		printf("%s %f s => %f fps\n", functions_names[i], (double)diff / (double)CLOCKS_PER_SEC, (double)ITERATIONS / ((double)diff / (double)CLOCKS_PER_SEC));
	}

	return 0;
}

int main(void)
{
	if (test())
	{
		fprintf(stderr, "tests failled\n");
		return 1;
	}
	if (benchmark())
	{
		fprintf(stderr, "benchmark failled\n");
		return 1;
	}

	return 0;
}