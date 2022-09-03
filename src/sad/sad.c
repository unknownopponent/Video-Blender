#include "sad.h"

#include <math.h>

uint64_t sad_u8_square(uint8_t* block1, uint8_t* block2, uint64_t offset, uint64_t width)
{
	uint64_t res = 0;

	for (size_t i = 0; i < width; i++)
		for (size_t j = 0; j < width; j++)
			res += abs(block1[i * offset + j] - block2[i * offset + j]);

	return res;
}

uint64_t sad_u8_rectangle(uint8_t* block1, uint8_t* block2, uint64_t offset, uint64_t width, uint64_t height)
{
	uint64_t res = 0;

	for (size_t i = 0; i < height; i++)
		for (size_t j = 0; j < width; j++)
			res += abs(block1[i * offset + j] - block2[i * offset + j]);

	return res;
}

#if defined(X86_64) || defined(X86_32)

#include <emmintrin.h>
#include <tmmintrin.h>
#include <immintrin.h>

SSE2_TARGET uint64_t sad_u8_2x2_sse2(uint8_t* block1, uint8_t* block2, uint64_t offset)
{
	__m128i xmm0, xmm1;

	// load 2x2 block1
	xmm0 = _mm_cvtsi64_si128(
		((uint64_t)(*(uint16_t*)block1) << 16)
		| (uint64_t)(*(uint16_t*)(block1 + offset))
	);

	// load 2x2 block2
	xmm1 = _mm_cvtsi64_si128(
		((uint64_t)(*(uint16_t*)block2) << 16)
		| (uint64_t)(*(uint16_t*)(block2 + offset))
	);

	xmm0 = _mm_sad_epu8(xmm0, xmm1);

	return _mm_cvtsi128_si32(xmm0);
}

SSE2_TARGET uint64_t sad_u8_6x2_sse2(uint8_t* block1, uint8_t* block2, uint64_t offset)
{
	__m128i xmm0, xmm1, xmm2, xmm3;

	//load 6x2 block1
	xmm0 = _mm_cvtsi64_si128(
		(*(uint64_t*)block1) & 0x0000ffffffffffff
	);
	xmm1 = _mm_cvtsi64_si128(
		(*(uint64_t*)(block1 + offset - 2)) & 0xffffffffffff0000
	);

	//load 6x2 block2
	xmm2 = _mm_cvtsi64_si128(
		(*(uint64_t*)block2) & 0x0000ffffffffffff
	);
	xmm3 = _mm_cvtsi64_si128(
		(*(uint64_t*)(block2 + offset - 2)) & 0xffffffffffff0000
	);

	xmm0 = _mm_sad_epu8(xmm0, xmm2);
	xmm1 = _mm_sad_epu8(xmm1, xmm3);

	xmm0 = _mm_add_epi16(xmm0, xmm1);

	return _mm_cvtsi128_si32(xmm0);
}

SSE2_TARGET uint64_t sad_u8_4x4_sse2(uint8_t* block1, uint8_t* block2, uint64_t offset)
{
	__m128i xmm0, xmm1, xmm2, xmm3;

	//load 4x4 block1 data

	xmm0 = _mm_cvtsi64_si128(
		((uint64_t)(*(uint32_t*)block1) << 32)
		| (uint64_t)(*(uint32_t*)(block1 + offset))
	);
	xmm1 = _mm_cvtsi64_si128(
		((uint64_t)(*(uint32_t*)(block1 + offset * 2)) << 32)
		| (uint64_t)(*(uint32_t*)(block1 + offset * 3))
	);

	//load 4x4 block2 data

	xmm2 = _mm_cvtsi64_si128(
		((uint64_t)(*(uint32_t*)block2) << 32)
		| (uint64_t)(*(uint32_t*)(block2 + offset))
	);
	xmm3 = _mm_cvtsi64_si128(
		((uint64_t)(*(uint32_t*)(block2 + offset * 2)) << 32)
		| (uint64_t)(*(uint32_t*)(block2 + offset * 3))
	);

	//sum absolute differences

	xmm0 = _mm_sad_epu8(xmm0, xmm2);
	xmm1 = _mm_sad_epu8(xmm1, xmm3);

	xmm0 = _mm_add_epi16(xmm0, xmm1);

	return _mm_cvtsi128_si32(xmm0);
}

SSSE3_TARGET uint64_t sad_u8_12x4_ssse3(uint8_t* block1, uint8_t* block2, uint64_t offset)
{
	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;

	static uint64_t mask1[] = { 0xffffffffffffffff, 0x00000000ffffffff };
	static uint64_t mask2[] = { 0xffffffff00000000, 0xffffffffffffffff };

	//load 12x4 block1
	xmm0 = _mm_loadu_si128((__m128i*)block1);
	xmm1 = _mm_loadu_si128((__m128i*)(block1 + offset));
	xmm2 = _mm_loadu_si128((__m128i*)(block1 + 2 * offset));
	xmm3 = _mm_loadu_si128((__m128i*)(block1 + 3 * offset - 4));

	xmm0 = _mm_and_si128(xmm0, *(__m128i*)mask1);
	xmm1 = _mm_and_si128(xmm1, *(__m128i*)mask1);
	xmm2 = _mm_and_si128(xmm2, *(__m128i*)mask1);
	xmm3 = _mm_and_si128(xmm3, *(__m128i*)mask2);

	//load 12x4 block2
	xmm4 = _mm_loadu_si128((__m128i*)block2);
	xmm5 = _mm_loadu_si128((__m128i*)(block2 + offset));
	xmm6 = _mm_loadu_si128((__m128i*)(block2 + 2 * offset));
	xmm7 = _mm_loadu_si128((__m128i*)(block2 + 3 * offset - 4));

	xmm4 = _mm_and_si128(xmm4, *(__m128i*)mask1);
	xmm5 = _mm_and_si128(xmm5, *(__m128i*)mask1);
	xmm6 = _mm_and_si128(xmm6, *(__m128i*)mask1);
	xmm7 = _mm_and_si128(xmm7, *(__m128i*)mask2);

	//sum absolute differences
	xmm0 = _mm_sad_epu8(xmm0, xmm4);
	xmm1 = _mm_sad_epu8(xmm1, xmm5);
	xmm2 = _mm_sad_epu8(xmm2, xmm6);
	xmm3 = _mm_sad_epu8(xmm3, xmm7);

	xmm0 = _mm_add_epi16(xmm0, xmm1);
	xmm2 = _mm_add_epi16(xmm2, xmm3);

	xmm0 = _mm_add_epi16(xmm0, xmm2);

	xmm0 = _mm_shuffle_epi32(xmm0, 8); // 00 00 10 00
	xmm0 = _mm_hadd_epi32(xmm0, xmm0);

	return _mm_cvtsi128_si32(xmm0);
}

SSE2_TARGET uint64_t sad_u8_8x8_sse2(uint8_t* block1, uint8_t* block2, uint64_t offset)
{
	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;

	//load 8x8 block1 data

	xmm0 = _mm_loadl_epi64((__m128i*)block1);
	xmm1 = _mm_loadl_epi64((__m128i*)(block1 + offset));
	xmm2 = _mm_loadl_epi64((__m128i*)(block1 + offset * 2));
	xmm3 = _mm_loadl_epi64((__m128i*)(block1 + offset * 3));
	xmm4 = _mm_loadl_epi64((__m128i*)(block1 + offset * 4));
	xmm5 = _mm_loadl_epi64((__m128i*)(block1 + offset * 5));
	xmm6 = _mm_loadl_epi64((__m128i*)(block1 + offset * 6));
	xmm7 = _mm_loadl_epi64((__m128i*)(block1 + offset * 7));

	//sum absolute differences

	xmm0 = _mm_sad_epu8(xmm0, *(__m128i*)block2);
	xmm1 = _mm_sad_epu8(xmm1, *(__m128i*)(block2 + offset));
	xmm2 = _mm_sad_epu8(xmm2, *(__m128i*)(block2 + offset * 2));
	xmm3 = _mm_sad_epu8(xmm3, *(__m128i*)(block2 + offset * 3));
	xmm4 = _mm_sad_epu8(xmm4, *(__m128i*)(block2 + offset * 4));
	xmm5 = _mm_sad_epu8(xmm5, *(__m128i*)(block2 + offset * 5));
	xmm6 = _mm_sad_epu8(xmm6, *(__m128i*)(block2 + offset * 6));
	xmm7 = _mm_sad_epu8(xmm7, *(__m128i*)(block2 + offset * 7));

	xmm0 = _mm_add_epi16(xmm0, xmm1);
	xmm2 = _mm_add_epi16(xmm2, xmm3);
	xmm4 = _mm_add_epi16(xmm4, xmm5);
	xmm6 = _mm_add_epi16(xmm6, xmm7);

	xmm0 = _mm_add_epi16(xmm0, xmm2);
	xmm4 = _mm_add_epi16(xmm4, xmm6);

	xmm0 = _mm_add_epi16(xmm0, xmm4);

	return _mm_cvtsi128_si32(xmm0);
}

SSSE3_TARGET uint64_t sad_u8_24x8_ssse3(uint8_t* block1, uint8_t* block2, uint64_t offset)
{
	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15, xmm16;

	// load 16x16 block1

	xmm0 = _mm_loadu_si128((__m128i*)block1);
	xmm1 = _mm_loadu_si128((__m128i*)(block1 + offset));
	xmm2 = _mm_loadu_si128((__m128i*)(block1 + offset * 2));
	xmm3 = _mm_loadu_si128((__m128i*)(block1 + offset * 3));
	xmm4 = _mm_loadu_si128((__m128i*)(block1 + offset * 4));
	xmm5 = _mm_loadu_si128((__m128i*)(block1 + offset * 5));
	xmm6 = _mm_loadu_si128((__m128i*)(block1 + offset * 6));
	xmm7 = _mm_loadu_si128((__m128i*)(block1 + offset * 7));

	//sum absolute differences

	xmm0 = _mm_sad_epu8(xmm0, *(__m128i*)block2);
	xmm1 = _mm_sad_epu8(xmm1, *(__m128i*)(block2 + offset));
	xmm2 = _mm_sad_epu8(xmm2, *(__m128i*)(block2 + offset * 2));
	xmm3 = _mm_sad_epu8(xmm3, *(__m128i*)(block2 + offset * 3));
	xmm4 = _mm_sad_epu8(xmm4, *(__m128i*)(block2 + offset * 4));
	xmm5 = _mm_sad_epu8(xmm5, *(__m128i*)(block2 + offset * 5));
	xmm6 = _mm_sad_epu8(xmm6, *(__m128i*)(block2 + offset * 6));
	xmm7 = _mm_sad_epu8(xmm7, *(__m128i*)(block2 + offset * 7));

	xmm0 = _mm_add_epi16(xmm0, xmm1);
	xmm2 = _mm_add_epi16(xmm2, xmm3);
	xmm4 = _mm_add_epi16(xmm4, xmm5);
	xmm6 = _mm_add_epi16(xmm6, xmm7);

	xmm0 = _mm_add_epi16(xmm0, xmm2);
	xmm4 = _mm_add_epi16(xmm4, xmm6);

	xmm0 = _mm_add_epi16(xmm0, xmm4);

	// load 8x8 block1 data in 16x4

	xmm1 = _mm_loadl_epi64((__m128i*)(block1 + 16));
	xmm2 = _mm_loadl_epi64((__m128i*)(block1 + 16 + offset));
	xmm3 = _mm_loadl_epi64((__m128i*)(block1 + 16 + offset * 2));
	xmm4 = _mm_loadl_epi64((__m128i*)(block1 + 16 + offset * 3));
	xmm5 = _mm_loadl_epi64((__m128i*)(block1 + 16 + offset * 4));
	xmm6 = _mm_loadl_epi64((__m128i*)(block1 + 16 + offset * 5));
	xmm7 = _mm_loadl_epi64((__m128i*)(block1 + 16 + offset * 6));
	xmm8 = _mm_loadl_epi64((__m128i*)(block1 + 16 + offset * 7));

	xmm1 = _mm_slli_si128(xmm1, 8);
	xmm2 = _mm_slli_si128(xmm2, 8);
	xmm3 = _mm_slli_si128(xmm3, 8);
	xmm4 = _mm_slli_si128(xmm4, 8);

	xmm1 = _mm_or_si128(xmm1, xmm5);
	xmm2 = _mm_or_si128(xmm2, xmm6);
	xmm3 = _mm_or_si128(xmm3, xmm7);
	xmm4 = _mm_or_si128(xmm4, xmm8);

	// load 8x8 block2 data in 16x4

	xmm9 = _mm_loadl_epi64((__m128i*)(block2 + 16));
	xmm10 = _mm_loadl_epi64((__m128i*)(block2 + 16 + offset));
	xmm11 = _mm_loadl_epi64((__m128i*)(block2 + 16 + offset * 2));
	xmm12 = _mm_loadl_epi64((__m128i*)(block2 + 16 + offset * 3));
	xmm13 = _mm_loadl_epi64((__m128i*)(block2 + 16 + offset * 4));
	xmm14 = _mm_loadl_epi64((__m128i*)(block2 + 16 + offset * 5));
	xmm15 = _mm_loadl_epi64((__m128i*)(block2 + 16 + offset * 6));
	xmm16 = _mm_loadl_epi64((__m128i*)(block2 + 16 + offset * 7));

	xmm9 = _mm_slli_si128(xmm9, 8);
	xmm10 = _mm_slli_si128(xmm10, 8);
	xmm11 = _mm_slli_si128(xmm11, 8);
	xmm12 = _mm_slli_si128(xmm12, 8);

	xmm9 = _mm_or_si128(xmm9, xmm13);
	xmm10 = _mm_or_si128(xmm10, xmm14);
	xmm11 = _mm_or_si128(xmm11, xmm15);
	xmm12 = _mm_or_si128(xmm12, xmm16);

	//sum absolute differences

	xmm1 = _mm_sad_epu8(xmm1, xmm9);
	xmm2 = _mm_sad_epu8(xmm2, xmm10);
	xmm3 = _mm_sad_epu8(xmm3, xmm11);
	xmm4 = _mm_sad_epu8(xmm4, xmm12);

	xmm1 = _mm_add_epi16(xmm1, xmm2);
	xmm3 = _mm_add_epi16(xmm3, xmm4);

	xmm1 = _mm_add_epi16(xmm1, xmm3);

	// add xmm0 xmm1

	xmm0 = _mm_shuffle_epi32(xmm0, 8); // 00 00 10 00
	xmm1 = _mm_shuffle_epi32(xmm1, 8); // 00 00 10 00
	xmm0 = _mm_add_epi32(xmm0, xmm1);
	xmm0 = _mm_hadd_epi32(xmm0, xmm0);

	return _mm_cvtsi128_si32(xmm0);
}

AVX2_TARGET uint64_t sad_u8_24x8_avx2(uint8_t* block1, uint8_t* block2, uint64_t offset)
{
	__m256i ymm0, ymm1, ymm2, ymm3, ymm4, ymm5, ymm6, ymm7, ymm8, ymm9, ymm10, ymm11, ymm12, ymm13, ymm14, ymm15;

	static uint64_t mask1[] = { 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0x0 };
	static uint64_t mask2[] = { 0x0, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff };

	//load 24x8 block1
	ymm0 = _mm256_loadu_si256((__m256i*)block1);
	ymm1 = _mm256_loadu_si256((__m256i*)(block1 + offset));
	ymm2 = _mm256_loadu_si256((__m256i*)(block1 + 2 * offset));
	ymm3 = _mm256_loadu_si256((__m256i*)(block1 + 3 * offset));
	ymm4 = _mm256_loadu_si256((__m256i*)(block1 + 4 * offset));
	ymm5 = _mm256_loadu_si256((__m256i*)(block1 + 5 * offset));
	ymm6 = _mm256_loadu_si256((__m256i*)(block1 + 6 * offset));
	ymm7 = _mm256_loadu_si256((__m256i*)(block1 + 7 * offset - 8));

	ymm0 = _mm256_and_si256(ymm0, *(__m256i*)mask1);
	ymm1 = _mm256_and_si256(ymm1, *(__m256i*)mask1);
	ymm2 = _mm256_and_si256(ymm2, *(__m256i*)mask1);
	ymm3 = _mm256_and_si256(ymm3, *(__m256i*)mask1);
	ymm4 = _mm256_and_si256(ymm4, *(__m256i*)mask1);
	ymm5 = _mm256_and_si256(ymm5, *(__m256i*)mask1);
	ymm6 = _mm256_and_si256(ymm6, *(__m256i*)mask1);
	ymm7 = _mm256_and_si256(ymm7, *(__m256i*)mask2);

	//load 24x8 block2
	ymm8 = _mm256_loadu_si256((__m256i*)block2);
	ymm9 = _mm256_loadu_si256((__m256i*)(block2 + offset));
	ymm10 = _mm256_loadu_si256((__m256i*)(block2 + 2 * offset));
	ymm11 = _mm256_loadu_si256((__m256i*)(block2 + 3 * offset));
	ymm12 = _mm256_loadu_si256((__m256i*)(block2 + 4 * offset));
	ymm13 = _mm256_loadu_si256((__m256i*)(block2 + 5 * offset));
	ymm14 = _mm256_loadu_si256((__m256i*)(block2 + 6 * offset));
	ymm15 = _mm256_loadu_si256((__m256i*)(block2 + 7 * offset - 8));

	ymm8 = _mm256_and_si256(ymm8, *(__m256i*)mask1);
	ymm9 = _mm256_and_si256(ymm9, *(__m256i*)mask1);
	ymm10 = _mm256_and_si256(ymm10, *(__m256i*)mask1);
	ymm11 = _mm256_and_si256(ymm11, *(__m256i*)mask1);
	ymm12 = _mm256_and_si256(ymm12, *(__m256i*)mask1);
	ymm13 = _mm256_and_si256(ymm13, *(__m256i*)mask1);
	ymm14 = _mm256_and_si256(ymm14, *(__m256i*)mask1);
	ymm15 = _mm256_and_si256(ymm15, *(__m256i*)mask2);

	//sum absolute differences

	ymm0 = _mm256_sad_epu8(ymm0, ymm8);
	ymm1 = _mm256_sad_epu8(ymm1, ymm9);
	ymm2 = _mm256_sad_epu8(ymm2, ymm10);
	ymm3 = _mm256_sad_epu8(ymm3, ymm11);
	ymm4 = _mm256_sad_epu8(ymm4, ymm12);
	ymm5 = _mm256_sad_epu8(ymm5, ymm13);
	ymm6 = _mm256_sad_epu8(ymm6, ymm14);
	ymm7 = _mm256_sad_epu8(ymm7, ymm15);

	ymm0 = _mm256_add_epi32(ymm0, ymm1);
	ymm2 = _mm256_add_epi32(ymm2, ymm3);
	ymm4 = _mm256_add_epi32(ymm4, ymm5);
	ymm6 = _mm256_add_epi32(ymm6, ymm7);

	ymm0 = _mm256_add_epi32(ymm0, ymm2);
	ymm4 = _mm256_add_epi32(ymm4, ymm6);

	ymm0 = _mm256_add_epi32(ymm0, ymm4);

	ymm1 = ymm0;

	static int32_t perm1[] = { 0, 2, -1, -1, -1, -1, -1, -1 };
	static int32_t perm2[] = { 4, 6, -1, -1, -1, -1, -1, -1 };

	ymm0 = _mm256_permutevar8x32_epi32(ymm0, *(__m256i*)perm1);
	ymm1 = _mm256_permutevar8x32_epi32(ymm1, *(__m256i*)perm2);

	ymm0 = _mm256_hadd_epi32(ymm0, ymm0);
	ymm1 = _mm256_hadd_epi32(ymm1, ymm1);

	ymm0 = _mm256_add_epi32(ymm0, ymm1);

	return _mm_cvtsi128_si32(_mm256_castsi256_si128(ymm0));
}

SSSE3_TARGET uint64_t sad_u8_16x16_ssse3(uint8_t* block1, uint8_t* block2, uint64_t offset)
{
	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;

	// load 16x16 block1 data

	xmm0 = _mm_loadu_si128((__m128i*)block1);
	xmm1 = _mm_loadu_si128((__m128i*)(block1 + offset));
	xmm2 = _mm_loadu_si128((__m128i*)(block1 + offset * 2));
	xmm3 = _mm_loadu_si128((__m128i*)(block1 + offset * 3));
	xmm4 = _mm_loadu_si128((__m128i*)(block1 + offset * 4));
	xmm5 = _mm_loadu_si128((__m128i*)(block1 + offset * 5));
	xmm6 = _mm_loadu_si128((__m128i*)(block1 + offset * 6));
	xmm7 = _mm_loadu_si128((__m128i*)(block1 + offset * 7));
	xmm8 = _mm_loadu_si128((__m128i*)(block1 + offset * 8));
	xmm9 = _mm_loadu_si128((__m128i*)(block1 + offset * 9));
	xmm10 = _mm_loadu_si128((__m128i*)(block1 + offset * 10));
	xmm11 = _mm_loadu_si128((__m128i*)(block1 + offset * 11));
	xmm12 = _mm_loadu_si128((__m128i*)(block1 + offset * 12));
	xmm13 = _mm_loadu_si128((__m128i*)(block1 + offset * 13));
	xmm14 = _mm_loadu_si128((__m128i*)(block1 + offset * 14));
	xmm15 = _mm_loadu_si128((__m128i*)(block1 + offset * 15));

	//sum absolute values with 16x16 block2 data

	xmm0 = _mm_sad_epu8(xmm0, *(__m128i*)block2);
	xmm1 = _mm_sad_epu8(xmm1, *(__m128i*)(block2 + offset));
	xmm2 = _mm_sad_epu8(xmm2, *(__m128i*)(block2 + offset * 2));
	xmm3 = _mm_sad_epu8(xmm3, *(__m128i*)(block2 + offset * 3));
	xmm4 = _mm_sad_epu8(xmm4, *(__m128i*)(block2 + offset * 4));
	xmm5 = _mm_sad_epu8(xmm5, *(__m128i*)(block2 + offset * 5));
	xmm6 = _mm_sad_epu8(xmm6, *(__m128i*)(block2 + offset * 6));
	xmm7 = _mm_sad_epu8(xmm7, *(__m128i*)(block2 + offset * 7));
	xmm8 = _mm_sad_epu8(xmm8, *(__m128i*)(block2 + offset * 8));
	xmm9 = _mm_sad_epu8(xmm9, *(__m128i*)(block2 + offset * 9));
	xmm10 = _mm_sad_epu8(xmm10, *(__m128i*)(block2 + offset * 10));
	xmm11 = _mm_sad_epu8(xmm11, *(__m128i*)(block2 + offset * 11));
	xmm12 = _mm_sad_epu8(xmm12, *(__m128i*)(block2 + offset * 12));
	xmm13 = _mm_sad_epu8(xmm13, *(__m128i*)(block2 + offset * 13));
	xmm14 = _mm_sad_epu8(xmm14, *(__m128i*)(block2 + offset * 14));
	xmm15 = _mm_sad_epu8(xmm15, *(__m128i*)(block2 + offset * 15));

	xmm0 = _mm_add_epi32(xmm0, xmm1);
	xmm2 = _mm_add_epi32(xmm2, xmm3);
	xmm4 = _mm_add_epi32(xmm4, xmm5);
	xmm6 = _mm_add_epi32(xmm6, xmm7);
	xmm8 = _mm_add_epi32(xmm8, xmm9);
	xmm10 = _mm_add_epi32(xmm10, xmm11);
	xmm12 = _mm_add_epi32(xmm12, xmm13);
	xmm14 = _mm_add_epi32(xmm14, xmm15);

	xmm0 = _mm_add_epi32(xmm0, xmm2);
	xmm4 = _mm_add_epi32(xmm4, xmm6);
	xmm8 = _mm_add_epi32(xmm8, xmm10);
	xmm12 = _mm_add_epi32(xmm12, xmm14);

	xmm0 = _mm_add_epi32(xmm0, xmm4);
	xmm8 = _mm_add_epi32(xmm8, xmm12);

	xmm0 = _mm_add_epi32(xmm0, xmm8);

	xmm0 = _mm_shuffle_epi32(xmm0, 8);
	xmm0 = _mm_hadd_epi32(xmm0, xmm0);

	return _mm_cvtsi128_si32(xmm0);
}

SSSE3_TARGET uint64_t sad_u8_48x16_ssse3(uint8_t* block1, uint8_t* block2, uint64_t offset)
{
	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;

	uint64_t res;

	// load 16x16 block1

	xmm0 = _mm_loadu_si128((__m128i*)block1);
	xmm1 = _mm_loadu_si128((__m128i*)(block1 + offset));
	xmm2 = _mm_loadu_si128((__m128i*)(block1 + offset * 2));
	xmm3 = _mm_loadu_si128((__m128i*)(block1 + offset * 3));
	xmm4 = _mm_loadu_si128((__m128i*)(block1 + offset * 4));
	xmm5 = _mm_loadu_si128((__m128i*)(block1 + offset * 5));
	xmm6 = _mm_loadu_si128((__m128i*)(block1 + offset * 6));
	xmm7 = _mm_loadu_si128((__m128i*)(block1 + offset * 7));
	xmm8 = _mm_loadu_si128((__m128i*)(block1 + offset * 8));
	xmm9 = _mm_loadu_si128((__m128i*)(block1 + offset * 9));
	xmm10 = _mm_loadu_si128((__m128i*)(block1 + offset * 10));
	xmm11 = _mm_loadu_si128((__m128i*)(block1 + offset * 11));
	xmm12 = _mm_loadu_si128((__m128i*)(block1 + offset * 12));
	xmm13 = _mm_loadu_si128((__m128i*)(block1 + offset * 13));
	xmm14 = _mm_loadu_si128((__m128i*)(block1 + offset * 14));
	xmm15 = _mm_loadu_si128((__m128i*)(block1 + offset * 15));

	//sum absolute values with 16x16 block2

	xmm0 = _mm_sad_epu8(xmm0, *(__m128i*)block2);
	xmm1 = _mm_sad_epu8(xmm1, *(__m128i*)(block2 + offset));
	xmm2 = _mm_sad_epu8(xmm2, *(__m128i*)(block2 + offset * 2));
	xmm3 = _mm_sad_epu8(xmm3, *(__m128i*)(block2 + offset * 3));
	xmm4 = _mm_sad_epu8(xmm4, *(__m128i*)(block2 + offset * 4));
	xmm5 = _mm_sad_epu8(xmm5, *(__m128i*)(block2 + offset * 5));
	xmm6 = _mm_sad_epu8(xmm6, *(__m128i*)(block2 + offset * 6));
	xmm7 = _mm_sad_epu8(xmm7, *(__m128i*)(block2 + offset * 7));
	xmm8 = _mm_sad_epu8(xmm8, *(__m128i*)(block2 + offset * 8));
	xmm9 = _mm_sad_epu8(xmm9, *(__m128i*)(block2 + offset * 9));
	xmm10 = _mm_sad_epu8(xmm10, *(__m128i*)(block2 + offset * 10));
	xmm11 = _mm_sad_epu8(xmm11, *(__m128i*)(block2 + offset * 11));
	xmm12 = _mm_sad_epu8(xmm12, *(__m128i*)(block2 + offset * 12));
	xmm13 = _mm_sad_epu8(xmm13, *(__m128i*)(block2 + offset * 13));
	xmm14 = _mm_sad_epu8(xmm14, *(__m128i*)(block2 + offset * 14));
	xmm15 = _mm_sad_epu8(xmm15, *(__m128i*)(block2 + offset * 15));

	xmm0 = _mm_add_epi32(xmm0, xmm1);
	xmm2 = _mm_add_epi32(xmm2, xmm3);
	xmm4 = _mm_add_epi32(xmm4, xmm5);
	xmm6 = _mm_add_epi32(xmm6, xmm7);
	xmm8 = _mm_add_epi32(xmm8, xmm9);
	xmm10 = _mm_add_epi32(xmm10, xmm11);
	xmm12 = _mm_add_epi32(xmm12, xmm13);
	xmm14 = _mm_add_epi32(xmm14, xmm15);

	xmm0 = _mm_add_epi32(xmm0, xmm2);
	xmm4 = _mm_add_epi32(xmm4, xmm6);
	xmm8 = _mm_add_epi32(xmm8, xmm10);
	xmm12 = _mm_add_epi32(xmm12, xmm14);

	xmm0 = _mm_add_epi32(xmm0, xmm4);
	xmm8 = _mm_add_epi32(xmm8, xmm12);

	xmm0 = _mm_add_epi32(xmm0, xmm8);

	xmm0 = _mm_shuffle_epi32(xmm0, 8);
	xmm0 = _mm_hadd_epi32(xmm0, xmm0);

	res = _mm_cvtsi128_si32(xmm0);

	// load 16x16 frame1 data

	xmm0 = _mm_loadu_si128((__m128i*)(block1 + 16));
	xmm1 = _mm_loadu_si128((__m128i*)(block1 + offset + 16));
	xmm2 = _mm_loadu_si128((__m128i*)(block1 + offset * 2 + 16));
	xmm3 = _mm_loadu_si128((__m128i*)(block1 + offset * 3 + 16));
	xmm4 = _mm_loadu_si128((__m128i*)(block1 + offset * 4 + 16));
	xmm5 = _mm_loadu_si128((__m128i*)(block1 + offset * 5 + 16));
	xmm6 = _mm_loadu_si128((__m128i*)(block1 + offset * 6 + 16));
	xmm7 = _mm_loadu_si128((__m128i*)(block1 + offset * 7 + 16));
	xmm8 = _mm_loadu_si128((__m128i*)(block1 + offset * 8 + 16));
	xmm9 = _mm_loadu_si128((__m128i*)(block1 + offset * 9 + 16));
	xmm10 = _mm_loadu_si128((__m128i*)(block1 + offset * 10 + 16));
	xmm11 = _mm_loadu_si128((__m128i*)(block1 + offset * 11 + 16));
	xmm12 = _mm_loadu_si128((__m128i*)(block1 + offset * 12 + 16));
	xmm13 = _mm_loadu_si128((__m128i*)(block1 + offset * 13 + 16));
	xmm14 = _mm_loadu_si128((__m128i*)(block1 + offset * 14 + 16));
	xmm15 = _mm_loadu_si128((__m128i*)(block1 + offset * 15 + 16));

	//sum absolute values with 16x16 frame2 data

	xmm0 = _mm_sad_epu8(xmm0, *(__m128i*)(block2 + 16));
	xmm1 = _mm_sad_epu8(xmm1, *(__m128i*)(block2 + offset + 16));
	xmm2 = _mm_sad_epu8(xmm2, *(__m128i*)(block2 + offset * 2 + 16));
	xmm3 = _mm_sad_epu8(xmm3, *(__m128i*)(block2 + offset * 3 + 16));
	xmm4 = _mm_sad_epu8(xmm4, *(__m128i*)(block2 + offset * 4 + 16));
	xmm5 = _mm_sad_epu8(xmm5, *(__m128i*)(block2 + offset * 5 + 16));
	xmm6 = _mm_sad_epu8(xmm6, *(__m128i*)(block2 + offset * 6 + 16));
	xmm7 = _mm_sad_epu8(xmm7, *(__m128i*)(block2 + offset * 7 + 16));
	xmm8 = _mm_sad_epu8(xmm8, *(__m128i*)(block2 + offset * 8 + 16));
	xmm9 = _mm_sad_epu8(xmm9, *(__m128i*)(block2 + offset * 9 + 16));
	xmm10 = _mm_sad_epu8(xmm10, *(__m128i*)(block2 + offset * 10 + 16));
	xmm11 = _mm_sad_epu8(xmm11, *(__m128i*)(block2 + offset * 11 + 16));
	xmm12 = _mm_sad_epu8(xmm12, *(__m128i*)(block2 + offset * 12 + 16));
	xmm13 = _mm_sad_epu8(xmm13, *(__m128i*)(block2 + offset * 13 + 16));
	xmm14 = _mm_sad_epu8(xmm14, *(__m128i*)(block2 + offset * 14 + 16));
	xmm15 = _mm_sad_epu8(xmm15, *(__m128i*)(block2 + offset * 15 + 16));

	xmm0 = _mm_add_epi32(xmm0, xmm1);
	xmm2 = _mm_add_epi32(xmm2, xmm3);
	xmm4 = _mm_add_epi32(xmm4, xmm5);
	xmm6 = _mm_add_epi32(xmm6, xmm7);
	xmm8 = _mm_add_epi32(xmm8, xmm9);
	xmm10 = _mm_add_epi32(xmm10, xmm11);
	xmm12 = _mm_add_epi32(xmm12, xmm13);
	xmm14 = _mm_add_epi32(xmm14, xmm15);

	xmm0 = _mm_add_epi32(xmm0, xmm2);
	xmm4 = _mm_add_epi32(xmm4, xmm6);
	xmm8 = _mm_add_epi32(xmm8, xmm10);
	xmm12 = _mm_add_epi32(xmm12, xmm14);

	xmm0 = _mm_add_epi32(xmm0, xmm4);
	xmm8 = _mm_add_epi32(xmm8, xmm12);

	xmm0 = _mm_add_epi32(xmm0, xmm8);

	xmm0 = _mm_shuffle_epi32(xmm0, 8);
	xmm0 = _mm_hadd_epi32(xmm0, xmm0);

	res += _mm_cvtsi128_si32(xmm0);

	// load 16x16 frame1 data

	xmm0 = _mm_loadu_si128((__m128i*)(block1 + 32));
	xmm1 = _mm_loadu_si128((__m128i*)(block1 + offset + 32));
	xmm2 = _mm_loadu_si128((__m128i*)(block1 + offset * 2 + 32));
	xmm3 = _mm_loadu_si128((__m128i*)(block1 + offset * 3 + 32));
	xmm4 = _mm_loadu_si128((__m128i*)(block1 + offset * 4 + 32));
	xmm5 = _mm_loadu_si128((__m128i*)(block1 + offset * 5 + 32));
	xmm6 = _mm_loadu_si128((__m128i*)(block1 + offset * 6 + 32));
	xmm7 = _mm_loadu_si128((__m128i*)(block1 + offset * 7 + 32));
	xmm8 = _mm_loadu_si128((__m128i*)(block1 + offset * 8 + 32));
	xmm9 = _mm_loadu_si128((__m128i*)(block1 + offset * 9 + 32));
	xmm10 = _mm_loadu_si128((__m128i*)(block1 + offset * 10 + 32));
	xmm11 = _mm_loadu_si128((__m128i*)(block1 + offset * 11 + 32));
	xmm12 = _mm_loadu_si128((__m128i*)(block1 + offset * 12 + 32));
	xmm13 = _mm_loadu_si128((__m128i*)(block1 + offset * 13 + 32));
	xmm14 = _mm_loadu_si128((__m128i*)(block1 + offset * 14 + 32));
	xmm15 = _mm_loadu_si128((__m128i*)(block1 + offset * 15 + 32));

	//sum absolute values with 16x16 frame2 data

	xmm0 = _mm_sad_epu8(xmm0, *(__m128i*)(block2 + 32));
	xmm1 = _mm_sad_epu8(xmm1, *(__m128i*)(block2 + offset + 32));
	xmm2 = _mm_sad_epu8(xmm2, *(__m128i*)(block2 + offset * 2 + 32));
	xmm3 = _mm_sad_epu8(xmm3, *(__m128i*)(block2 + offset * 3 + 32));
	xmm4 = _mm_sad_epu8(xmm4, *(__m128i*)(block2 + offset * 4 + 32));
	xmm5 = _mm_sad_epu8(xmm5, *(__m128i*)(block2 + offset * 5 + 32));
	xmm6 = _mm_sad_epu8(xmm6, *(__m128i*)(block2 + offset * 6 + 32));
	xmm7 = _mm_sad_epu8(xmm7, *(__m128i*)(block2 + offset * 7 + 32));
	xmm8 = _mm_sad_epu8(xmm8, *(__m128i*)(block2 + offset * 8 + 32));
	xmm9 = _mm_sad_epu8(xmm9, *(__m128i*)(block2 + offset * 9 + 32));
	xmm10 = _mm_sad_epu8(xmm10, *(__m128i*)(block2 + offset * 10 + 32));
	xmm11 = _mm_sad_epu8(xmm11, *(__m128i*)(block2 + offset * 11 + 32));
	xmm12 = _mm_sad_epu8(xmm12, *(__m128i*)(block2 + offset * 12 + 32));
	xmm13 = _mm_sad_epu8(xmm13, *(__m128i*)(block2 + offset * 13 + 32));
	xmm14 = _mm_sad_epu8(xmm14, *(__m128i*)(block2 + offset * 14 + 32));
	xmm15 = _mm_sad_epu8(xmm15, *(__m128i*)(block2 + offset * 15 + 32));

	xmm0 = _mm_add_epi32(xmm0, xmm1);
	xmm2 = _mm_add_epi32(xmm2, xmm3);
	xmm4 = _mm_add_epi32(xmm4, xmm5);
	xmm6 = _mm_add_epi32(xmm6, xmm7);
	xmm8 = _mm_add_epi32(xmm8, xmm9);
	xmm10 = _mm_add_epi32(xmm10, xmm11);
	xmm12 = _mm_add_epi32(xmm12, xmm13);
	xmm14 = _mm_add_epi32(xmm14, xmm15);

	xmm0 = _mm_add_epi32(xmm0, xmm2);
	xmm4 = _mm_add_epi32(xmm4, xmm6);
	xmm8 = _mm_add_epi32(xmm8, xmm10);
	xmm12 = _mm_add_epi32(xmm12, xmm14);

	xmm0 = _mm_add_epi32(xmm0, xmm4);
	xmm8 = _mm_add_epi32(xmm8, xmm12);

	xmm0 = _mm_add_epi32(xmm0, xmm8);

	xmm0 = _mm_shuffle_epi32(xmm0, 8);
	xmm0 = _mm_hadd_epi32(xmm0, xmm0);

	return res + _mm_cvtsi128_si32(xmm0);
}

AVX2_TARGET uint64_t sad_u8_48x16_avx2(uint8_t* block1, uint8_t* block2, uint64_t offset)
{
	__m256i ymm0, ymm1, ymm2, ymm3, ymm4, ymm5, ymm6, ymm7, ymm8, ymm9, ymm10, ymm11, ymm12, ymm13, ymm14, ymm15;

	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;

	uint64_t res;

	//load 32x16 block1 data

	ymm0 = _mm256_loadu_si256((__m256i*)block1);
	ymm1 = _mm256_loadu_si256((__m256i*)(block1 + offset));
	ymm2 = _mm256_loadu_si256((__m256i*)(block1 + offset * 2));
	ymm3 = _mm256_loadu_si256((__m256i*)(block1 + offset * 3));
	ymm4 = _mm256_loadu_si256((__m256i*)(block1 + offset * 4));
	ymm5 = _mm256_loadu_si256((__m256i*)(block1 + offset * 5));
	ymm6 = _mm256_loadu_si256((__m256i*)(block1 + offset * 6));
	ymm7 = _mm256_loadu_si256((__m256i*)(block1 + offset * 7));
	ymm8 = _mm256_loadu_si256((__m256i*)(block1 + offset * 8));
	ymm9 = _mm256_loadu_si256((__m256i*)(block1 + offset * 9));
	ymm10 = _mm256_loadu_si256((__m256i*)(block1 + offset * 10));
	ymm11 = _mm256_loadu_si256((__m256i*)(block1 + offset * 11));
	ymm12 = _mm256_loadu_si256((__m256i*)(block1 + offset * 12));
	ymm13 = _mm256_loadu_si256((__m256i*)(block1 + offset * 13));
	ymm14 = _mm256_loadu_si256((__m256i*)(block1 + offset * 14));
	ymm15 = _mm256_loadu_si256((__m256i*)(block1 + offset * 15));

	//sum absolute differences

	ymm0 = _mm256_sad_epu8(ymm0, *(__m256i*)(block2));
	ymm1 = _mm256_sad_epu8(ymm1, *(__m256i*)(block2 + offset));
	ymm2 = _mm256_sad_epu8(ymm2, *(__m256i*)(block2 + offset * 2));
	ymm3 = _mm256_sad_epu8(ymm3, *(__m256i*)(block2 + offset * 3));
	ymm4 = _mm256_sad_epu8(ymm4, *(__m256i*)(block2 + offset * 4));
	ymm5 = _mm256_sad_epu8(ymm5, *(__m256i*)(block2 + offset * 5));
	ymm6 = _mm256_sad_epu8(ymm6, *(__m256i*)(block2 + offset * 6));
	ymm7 = _mm256_sad_epu8(ymm7, *(__m256i*)(block2 + offset * 7));
	ymm8 = _mm256_sad_epu8(ymm8, *(__m256i*)(block2 + offset * 8));
	ymm9 = _mm256_sad_epu8(ymm9, *(__m256i*)(block2 + offset * 9));
	ymm10 = _mm256_sad_epu8(ymm10, *(__m256i*)(block2 + offset * 10));
	ymm11 = _mm256_sad_epu8(ymm11, *(__m256i*)(block2 + offset * 11));
	ymm12 = _mm256_sad_epu8(ymm12, *(__m256i*)(block2 + offset * 12));
	ymm13 = _mm256_sad_epu8(ymm13, *(__m256i*)(block2 + offset * 13));
	ymm14 = _mm256_sad_epu8(ymm14, *(__m256i*)(block2 + offset * 14));
	ymm15 = _mm256_sad_epu8(ymm15, *(__m256i*)(block2 + offset * 15));

	ymm0 = _mm256_add_epi32(ymm0, ymm1);
	ymm2 = _mm256_add_epi32(ymm2, ymm3);
	ymm4 = _mm256_add_epi32(ymm4, ymm5);
	ymm6 = _mm256_add_epi32(ymm6, ymm7);
	ymm8 = _mm256_add_epi32(ymm8, ymm9);
	ymm10 = _mm256_add_epi32(ymm10, ymm11);
	ymm12 = _mm256_add_epi32(ymm12, ymm13);
	ymm14 = _mm256_add_epi32(ymm14, ymm15);

	ymm0 = _mm256_add_epi32(ymm0, ymm2);
	ymm4 = _mm256_add_epi32(ymm4, ymm6);
	ymm8 = _mm256_add_epi32(ymm8, ymm10);
	ymm12 = _mm256_add_epi32(ymm12, ymm14);

	ymm0 = _mm256_add_epi32(ymm0, ymm4);
	ymm8 = _mm256_add_epi32(ymm8, ymm12);

	ymm0 = _mm256_add_epi32(ymm0, ymm8);

	xmm1 = _mm256_extracti128_si256(ymm0, 1);

	xmm0 = _mm_slli_si128(_mm256_extracti128_si256(ymm0, 0), 4);
	xmm0 = _mm_or_si128(xmm0, xmm1);

	xmm0 = _mm_hadd_epi32(xmm0, xmm0);
	xmm0 = _mm_hadd_epi32(xmm0, xmm0);

	res = _mm_cvtsi128_si32(xmm0);

	//load 16x16 block1 data

	xmm0 = _mm_loadu_si128((__m128i*)(block1 + 32));
	xmm1 = _mm_loadu_si128((__m128i*)(block1 + offset + 32));
	xmm2 = _mm_loadu_si128((__m128i*)(block1 + offset * 2 + 32));
	xmm3 = _mm_loadu_si128((__m128i*)(block1 + offset * 3 + 32));
	xmm4 = _mm_loadu_si128((__m128i*)(block1 + offset * 4 + 32));
	xmm5 = _mm_loadu_si128((__m128i*)(block1 + offset * 5 + 32));
	xmm6 = _mm_loadu_si128((__m128i*)(block1 + offset * 6 + 32));
	xmm7 = _mm_loadu_si128((__m128i*)(block1 + offset * 7 + 32));
	xmm8 = _mm_loadu_si128((__m128i*)(block1 + offset * 8 + 32));
	xmm9 = _mm_loadu_si128((__m128i*)(block1 + offset * 9 + 32));
	xmm10 = _mm_loadu_si128((__m128i*)(block1 + offset * 10 + 32));
	xmm11 = _mm_loadu_si128((__m128i*)(block1 + offset * 11 + 32));
	xmm12 = _mm_loadu_si128((__m128i*)(block1 + offset * 12 + 32));
	xmm13 = _mm_loadu_si128((__m128i*)(block1 + offset * 13 + 32));
	xmm14 = _mm_loadu_si128((__m128i*)(block1 + offset * 14 + 32));
	xmm15 = _mm_loadu_si128((__m128i*)(block1 + offset * 15 + 32));

	//sum absolute values with 16x16 block2 data

	xmm0 = _mm_sad_epu8(xmm0, *(__m128i*)(block2 + 32));
	xmm1 = _mm_sad_epu8(xmm1, *(__m128i*)(block2 + offset + 32));
	xmm2 = _mm_sad_epu8(xmm2, *(__m128i*)(block2 + offset * 2 + 32));
	xmm3 = _mm_sad_epu8(xmm3, *(__m128i*)(block2 + offset * 3 + 32));
	xmm4 = _mm_sad_epu8(xmm4, *(__m128i*)(block2 + offset * 4 + 32));
	xmm5 = _mm_sad_epu8(xmm5, *(__m128i*)(block2 + offset * 5 + 32));
	xmm6 = _mm_sad_epu8(xmm6, *(__m128i*)(block2 + offset * 6 + 32));
	xmm7 = _mm_sad_epu8(xmm7, *(__m128i*)(block2 + offset * 7 + 32));
	xmm8 = _mm_sad_epu8(xmm8, *(__m128i*)(block2 + offset * 8 + 32));
	xmm9 = _mm_sad_epu8(xmm9, *(__m128i*)(block2 + offset * 9 + 32));
	xmm10 = _mm_sad_epu8(xmm10, *(__m128i*)(block2 + offset * 10 + 32));
	xmm11 = _mm_sad_epu8(xmm11, *(__m128i*)(block2 + offset * 11 + 32));
	xmm12 = _mm_sad_epu8(xmm12, *(__m128i*)(block2 + offset * 12 + 32));
	xmm13 = _mm_sad_epu8(xmm13, *(__m128i*)(block2 + offset * 13 + 32));
	xmm14 = _mm_sad_epu8(xmm14, *(__m128i*)(block2 + offset * 14 + 32));
	xmm15 = _mm_sad_epu8(xmm15, *(__m128i*)(block2 + offset * 15 + 32));

	xmm0 = _mm_add_epi32(xmm0, xmm1);
	xmm2 = _mm_add_epi32(xmm2, xmm3);
	xmm4 = _mm_add_epi32(xmm4, xmm5);
	xmm6 = _mm_add_epi32(xmm6, xmm7);
	xmm8 = _mm_add_epi32(xmm8, xmm9);
	xmm10 = _mm_add_epi32(xmm10, xmm11);
	xmm12 = _mm_add_epi32(xmm12, xmm13);
	xmm14 = _mm_add_epi32(xmm14, xmm15);

	xmm0 = _mm_add_epi32(xmm0, xmm2);
	xmm4 = _mm_add_epi32(xmm4, xmm6);
	xmm8 = _mm_add_epi32(xmm8, xmm10);
	xmm12 = _mm_add_epi32(xmm12, xmm14);

	xmm0 = _mm_add_epi32(xmm0, xmm4);
	xmm8 = _mm_add_epi32(xmm8, xmm12);

	xmm0 = _mm_add_epi32(xmm0, xmm8);

	xmm0 = _mm_shuffle_epi32(xmm0, 8);
	xmm0 = _mm_hadd_epi32(xmm0, xmm0);

	return res + _mm_cvtsi128_si32(xmm0);
}

SSSE3_TARGET uint64_t sad_u8_32x32_ssse3(uint8_t* block1, uint8_t* block2, uint64_t offset)
{
	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;

	uint64_t res;

	// load 16x16 block1

	xmm0 = _mm_loadu_si128((__m128i*)block1);
	xmm1 = _mm_loadu_si128((__m128i*)(block1 + offset));
	xmm2 = _mm_loadu_si128((__m128i*)(block1 + offset * 2));
	xmm3 = _mm_loadu_si128((__m128i*)(block1 + offset * 3));
	xmm4 = _mm_loadu_si128((__m128i*)(block1 + offset * 4));
	xmm5 = _mm_loadu_si128((__m128i*)(block1 + offset * 5));
	xmm6 = _mm_loadu_si128((__m128i*)(block1 + offset * 6));
	xmm7 = _mm_loadu_si128((__m128i*)(block1 + offset * 7));
	xmm8 = _mm_loadu_si128((__m128i*)(block1 + offset * 8));
	xmm9 = _mm_loadu_si128((__m128i*)(block1 + offset * 9));
	xmm10 = _mm_loadu_si128((__m128i*)(block1 + offset * 10));
	xmm11 = _mm_loadu_si128((__m128i*)(block1 + offset * 11));
	xmm12 = _mm_loadu_si128((__m128i*)(block1 + offset * 12));
	xmm13 = _mm_loadu_si128((__m128i*)(block1 + offset * 13));
	xmm14 = _mm_loadu_si128((__m128i*)(block1 + offset * 14));
	xmm15 = _mm_loadu_si128((__m128i*)(block1 + offset * 15));

	//sum absolute values with 16x16 block2

	xmm0 = _mm_sad_epu8(xmm0, *(__m128i*)block2);
	xmm1 = _mm_sad_epu8(xmm1, *(__m128i*)(block2 + offset));
	xmm2 = _mm_sad_epu8(xmm2, *(__m128i*)(block2 + offset * 2));
	xmm3 = _mm_sad_epu8(xmm3, *(__m128i*)(block2 + offset * 3));
	xmm4 = _mm_sad_epu8(xmm4, *(__m128i*)(block2 + offset * 4));
	xmm5 = _mm_sad_epu8(xmm5, *(__m128i*)(block2 + offset * 5));
	xmm6 = _mm_sad_epu8(xmm6, *(__m128i*)(block2 + offset * 6));
	xmm7 = _mm_sad_epu8(xmm7, *(__m128i*)(block2 + offset * 7));
	xmm8 = _mm_sad_epu8(xmm8, *(__m128i*)(block2 + offset * 8));
	xmm9 = _mm_sad_epu8(xmm9, *(__m128i*)(block2 + offset * 9));
	xmm10 = _mm_sad_epu8(xmm10, *(__m128i*)(block2 + offset * 10));
	xmm11 = _mm_sad_epu8(xmm11, *(__m128i*)(block2 + offset * 11));
	xmm12 = _mm_sad_epu8(xmm12, *(__m128i*)(block2 + offset * 12));
	xmm13 = _mm_sad_epu8(xmm13, *(__m128i*)(block2 + offset * 13));
	xmm14 = _mm_sad_epu8(xmm14, *(__m128i*)(block2 + offset * 14));
	xmm15 = _mm_sad_epu8(xmm15, *(__m128i*)(block2 + offset * 15));

	xmm0 = _mm_add_epi32(xmm0, xmm1);
	xmm2 = _mm_add_epi32(xmm2, xmm3);
	xmm4 = _mm_add_epi32(xmm4, xmm5);
	xmm6 = _mm_add_epi32(xmm6, xmm7);
	xmm8 = _mm_add_epi32(xmm8, xmm9);
	xmm10 = _mm_add_epi32(xmm10, xmm11);
	xmm12 = _mm_add_epi32(xmm12, xmm13);
	xmm14 = _mm_add_epi32(xmm14, xmm15);

	xmm0 = _mm_add_epi32(xmm0, xmm2);
	xmm4 = _mm_add_epi32(xmm4, xmm6);
	xmm8 = _mm_add_epi32(xmm8, xmm10);
	xmm12 = _mm_add_epi32(xmm12, xmm14);

	xmm0 = _mm_add_epi32(xmm0, xmm4);
	xmm8 = _mm_add_epi32(xmm8, xmm12);

	xmm0 = _mm_add_epi32(xmm0, xmm8);

	xmm0 = _mm_shuffle_epi32(xmm0, 8);
	xmm0 = _mm_hadd_epi32(xmm0, xmm0);

	res = _mm_cvtsi128_si32(xmm0);

	// load 16x16 block1

	xmm0 = _mm_loadu_si128((__m128i*)(block1 + 16));
	xmm1 = _mm_loadu_si128((__m128i*)(block1 + offset + 16));
	xmm2 = _mm_loadu_si128((__m128i*)(block1 + offset * 2 + 16));
	xmm3 = _mm_loadu_si128((__m128i*)(block1 + offset * 3 + 16));
	xmm4 = _mm_loadu_si128((__m128i*)(block1 + offset * 4 + 16));
	xmm5 = _mm_loadu_si128((__m128i*)(block1 + offset * 5 + 16));
	xmm6 = _mm_loadu_si128((__m128i*)(block1 + offset * 6 + 16));
	xmm7 = _mm_loadu_si128((__m128i*)(block1 + offset * 7 + 16));
	xmm8 = _mm_loadu_si128((__m128i*)(block1 + offset * 8 + 16));
	xmm9 = _mm_loadu_si128((__m128i*)(block1 + offset * 9 + 16));
	xmm10 = _mm_loadu_si128((__m128i*)(block1 + offset * 10 + 16));
	xmm11 = _mm_loadu_si128((__m128i*)(block1 + offset * 11 + 16));
	xmm12 = _mm_loadu_si128((__m128i*)(block1 + offset * 12 + 16));
	xmm13 = _mm_loadu_si128((__m128i*)(block1 + offset * 13 + 16));
	xmm14 = _mm_loadu_si128((__m128i*)(block1 + offset * 14 + 16));
	xmm15 = _mm_loadu_si128((__m128i*)(block1 + offset * 15 + 16));

	//sum absolute values with 16x16 block2

	xmm0 = _mm_sad_epu8(xmm0, *(__m128i*)(block2 + 16));
	xmm1 = _mm_sad_epu8(xmm1, *(__m128i*)(block2 + offset + 16));
	xmm2 = _mm_sad_epu8(xmm2, *(__m128i*)(block2 + offset * 2 + 16));
	xmm3 = _mm_sad_epu8(xmm3, *(__m128i*)(block2 + offset * 3 + 16));
	xmm4 = _mm_sad_epu8(xmm4, *(__m128i*)(block2 + offset * 4 + 16));
	xmm5 = _mm_sad_epu8(xmm5, *(__m128i*)(block2 + offset * 5 + 16));
	xmm6 = _mm_sad_epu8(xmm6, *(__m128i*)(block2 + offset * 6 + 16));
	xmm7 = _mm_sad_epu8(xmm7, *(__m128i*)(block2 + offset * 7 + 16));
	xmm8 = _mm_sad_epu8(xmm8, *(__m128i*)(block2 + offset * 8 + 16));
	xmm9 = _mm_sad_epu8(xmm9, *(__m128i*)(block2 + offset * 9 + 16));
	xmm10 = _mm_sad_epu8(xmm10, *(__m128i*)(block2 + offset * 10 + 16));
	xmm11 = _mm_sad_epu8(xmm11, *(__m128i*)(block2 + offset * 11 + 16));
	xmm12 = _mm_sad_epu8(xmm12, *(__m128i*)(block2 + offset * 12 + 16));
	xmm13 = _mm_sad_epu8(xmm13, *(__m128i*)(block2 + offset * 13 + 16));
	xmm14 = _mm_sad_epu8(xmm14, *(__m128i*)(block2 + offset * 14 + 16));
	xmm15 = _mm_sad_epu8(xmm15, *(__m128i*)(block2 + offset * 15 + 16));

	xmm0 = _mm_add_epi32(xmm0, xmm1);
	xmm2 = _mm_add_epi32(xmm2, xmm3);
	xmm4 = _mm_add_epi32(xmm4, xmm5);
	xmm6 = _mm_add_epi32(xmm6, xmm7);
	xmm8 = _mm_add_epi32(xmm8, xmm9);
	xmm10 = _mm_add_epi32(xmm10, xmm11);
	xmm12 = _mm_add_epi32(xmm12, xmm13);
	xmm14 = _mm_add_epi32(xmm14, xmm15);

	xmm0 = _mm_add_epi32(xmm0, xmm2);
	xmm4 = _mm_add_epi32(xmm4, xmm6);
	xmm8 = _mm_add_epi32(xmm8, xmm10);
	xmm12 = _mm_add_epi32(xmm12, xmm14);

	xmm0 = _mm_add_epi32(xmm0, xmm4);
	xmm8 = _mm_add_epi32(xmm8, xmm12);

	xmm0 = _mm_add_epi32(xmm0, xmm8);

	xmm0 = _mm_shuffle_epi32(xmm0, 8);
	xmm0 = _mm_hadd_epi32(xmm0, xmm0);

	res += _mm_cvtsi128_si32(xmm0);

	// load 16x16 block1

	xmm0 = _mm_loadu_si128((__m128i*)(block1 + offset * 16));
	xmm1 = _mm_loadu_si128((__m128i*)(block1 + offset * 17));
	xmm2 = _mm_loadu_si128((__m128i*)(block1 + offset * 18));
	xmm3 = _mm_loadu_si128((__m128i*)(block1 + offset * 19));
	xmm4 = _mm_loadu_si128((__m128i*)(block1 + offset * 20));
	xmm5 = _mm_loadu_si128((__m128i*)(block1 + offset * 21));
	xmm6 = _mm_loadu_si128((__m128i*)(block1 + offset * 22));
	xmm7 = _mm_loadu_si128((__m128i*)(block1 + offset * 23));
	xmm8 = _mm_loadu_si128((__m128i*)(block1 + offset * 24));
	xmm9 = _mm_loadu_si128((__m128i*)(block1 + offset * 25));
	xmm10 = _mm_loadu_si128((__m128i*)(block1 + offset * 26));
	xmm11 = _mm_loadu_si128((__m128i*)(block1 + offset * 27));
	xmm12 = _mm_loadu_si128((__m128i*)(block1 + offset * 28));
	xmm13 = _mm_loadu_si128((__m128i*)(block1 + offset * 29));
	xmm14 = _mm_loadu_si128((__m128i*)(block1 + offset * 30));
	xmm15 = _mm_loadu_si128((__m128i*)(block1 + offset * 31));

	//sum absolute values with 16x16 block2

	xmm0 = _mm_sad_epu8(xmm0, *(__m128i*)(block2 + offset * 16));
	xmm1 = _mm_sad_epu8(xmm1, *(__m128i*)(block2 + offset * 17));
	xmm2 = _mm_sad_epu8(xmm2, *(__m128i*)(block2 + offset * 18));
	xmm3 = _mm_sad_epu8(xmm3, *(__m128i*)(block2 + offset * 19));
	xmm4 = _mm_sad_epu8(xmm4, *(__m128i*)(block2 + offset * 20));
	xmm5 = _mm_sad_epu8(xmm5, *(__m128i*)(block2 + offset * 21));
	xmm6 = _mm_sad_epu8(xmm6, *(__m128i*)(block2 + offset * 22));
	xmm7 = _mm_sad_epu8(xmm7, *(__m128i*)(block2 + offset * 23));
	xmm8 = _mm_sad_epu8(xmm8, *(__m128i*)(block2 + offset * 24));
	xmm9 = _mm_sad_epu8(xmm9, *(__m128i*)(block2 + offset * 25));
	xmm10 = _mm_sad_epu8(xmm10, *(__m128i*)(block2 + offset * 26));
	xmm11 = _mm_sad_epu8(xmm11, *(__m128i*)(block2 + offset * 27));
	xmm12 = _mm_sad_epu8(xmm12, *(__m128i*)(block2 + offset * 28));
	xmm13 = _mm_sad_epu8(xmm13, *(__m128i*)(block2 + offset * 29));
	xmm14 = _mm_sad_epu8(xmm14, *(__m128i*)(block2 + offset * 30));
	xmm15 = _mm_sad_epu8(xmm15, *(__m128i*)(block2 + offset * 31));

	xmm0 = _mm_add_epi32(xmm0, xmm1);
	xmm2 = _mm_add_epi32(xmm2, xmm3);
	xmm4 = _mm_add_epi32(xmm4, xmm5);
	xmm6 = _mm_add_epi32(xmm6, xmm7);
	xmm8 = _mm_add_epi32(xmm8, xmm9);
	xmm10 = _mm_add_epi32(xmm10, xmm11);
	xmm12 = _mm_add_epi32(xmm12, xmm13);
	xmm14 = _mm_add_epi32(xmm14, xmm15);

	xmm0 = _mm_add_epi32(xmm0, xmm2);
	xmm4 = _mm_add_epi32(xmm4, xmm6);
	xmm8 = _mm_add_epi32(xmm8, xmm10);
	xmm12 = _mm_add_epi32(xmm12, xmm14);

	xmm0 = _mm_add_epi32(xmm0, xmm4);
	xmm8 = _mm_add_epi32(xmm8, xmm12);

	xmm0 = _mm_add_epi32(xmm0, xmm8);

	xmm0 = _mm_shuffle_epi32(xmm0, 8);
	xmm0 = _mm_hadd_epi32(xmm0, xmm0);

	res += _mm_cvtsi128_si32(xmm0);

	// load 16x16 block1

	xmm0 = _mm_loadu_si128((__m128i*)(block1 + offset * 16 + 16));
	xmm1 = _mm_loadu_si128((__m128i*)(block1 + offset * 17 + 16));
	xmm2 = _mm_loadu_si128((__m128i*)(block1 + offset * 18 + 16));
	xmm3 = _mm_loadu_si128((__m128i*)(block1 + offset * 19 + 16));
	xmm4 = _mm_loadu_si128((__m128i*)(block1 + offset * 20 + 16));
	xmm5 = _mm_loadu_si128((__m128i*)(block1 + offset * 21 + 16));
	xmm6 = _mm_loadu_si128((__m128i*)(block1 + offset * 22 + 16));
	xmm7 = _mm_loadu_si128((__m128i*)(block1 + offset * 23 + 16));
	xmm8 = _mm_loadu_si128((__m128i*)(block1 + offset * 24 + 16));
	xmm9 = _mm_loadu_si128((__m128i*)(block1 + offset * 25 + 16));
	xmm10 = _mm_loadu_si128((__m128i*)(block1 + offset * 26 + 16));
	xmm11 = _mm_loadu_si128((__m128i*)(block1 + offset * 27 + 16));
	xmm12 = _mm_loadu_si128((__m128i*)(block1 + offset * 28 + 16));
	xmm13 = _mm_loadu_si128((__m128i*)(block1 + offset * 29 + 16));
	xmm14 = _mm_loadu_si128((__m128i*)(block1 + offset * 30 + 16));
	xmm15 = _mm_loadu_si128((__m128i*)(block1 + offset * 31 + 16));

	//sum absolute values with 16x16 block2

	xmm0 = _mm_sad_epu8(xmm0, *(__m128i*)(block2 + offset * 16 + 16));
	xmm1 = _mm_sad_epu8(xmm1, *(__m128i*)(block2 + offset * 17 + 16));
	xmm2 = _mm_sad_epu8(xmm2, *(__m128i*)(block2 + offset * 18 + 16));
	xmm3 = _mm_sad_epu8(xmm3, *(__m128i*)(block2 + offset * 19 + 16));
	xmm4 = _mm_sad_epu8(xmm4, *(__m128i*)(block2 + offset * 20 + 16));
	xmm5 = _mm_sad_epu8(xmm5, *(__m128i*)(block2 + offset * 21 + 16));
	xmm6 = _mm_sad_epu8(xmm6, *(__m128i*)(block2 + offset * 22 + 16));
	xmm7 = _mm_sad_epu8(xmm7, *(__m128i*)(block2 + offset * 23 + 16));
	xmm8 = _mm_sad_epu8(xmm8, *(__m128i*)(block2 + offset * 24 + 16));
	xmm9 = _mm_sad_epu8(xmm9, *(__m128i*)(block2 + offset * 25 + 16));
	xmm10 = _mm_sad_epu8(xmm10, *(__m128i*)(block2 + offset * 26 + 16));
	xmm11 = _mm_sad_epu8(xmm11, *(__m128i*)(block2 + offset * 27 + 16));
	xmm12 = _mm_sad_epu8(xmm12, *(__m128i*)(block2 + offset * 28 + 16));
	xmm13 = _mm_sad_epu8(xmm13, *(__m128i*)(block2 + offset * 29 + 16));
	xmm14 = _mm_sad_epu8(xmm14, *(__m128i*)(block2 + offset * 30 + 16));
	xmm15 = _mm_sad_epu8(xmm15, *(__m128i*)(block2 + offset * 31 + 16));

	xmm0 = _mm_add_epi32(xmm0, xmm1);
	xmm2 = _mm_add_epi32(xmm2, xmm3);
	xmm4 = _mm_add_epi32(xmm4, xmm5);
	xmm6 = _mm_add_epi32(xmm6, xmm7);
	xmm8 = _mm_add_epi32(xmm8, xmm9);
	xmm10 = _mm_add_epi32(xmm10, xmm11);
	xmm12 = _mm_add_epi32(xmm12, xmm13);
	xmm14 = _mm_add_epi32(xmm14, xmm15);

	xmm0 = _mm_add_epi32(xmm0, xmm2);
	xmm4 = _mm_add_epi32(xmm4, xmm6);
	xmm8 = _mm_add_epi32(xmm8, xmm10);
	xmm12 = _mm_add_epi32(xmm12, xmm14);

	xmm0 = _mm_add_epi32(xmm0, xmm4);
	xmm8 = _mm_add_epi32(xmm8, xmm12);

	xmm0 = _mm_add_epi32(xmm0, xmm8);

	xmm0 = _mm_shuffle_epi32(xmm0, 8);
	xmm0 = _mm_hadd_epi32(xmm0, xmm0);

	return res + _mm_cvtsi128_si32(xmm0);
}

AVX2_TARGET uint64_t sad_u8_32x32_avx2(uint8_t* block1, uint8_t* block2, uint64_t offset)
{
	__m256i ymm0, ymm1, ymm2, ymm3, ymm4, ymm5, ymm6, ymm7, ymm8, ymm9, ymm10, ymm11, ymm12, ymm13, ymm14, ymm15;

	__m128i xmm0, xmm1;

	uint64_t res;

	//load 32x16 block1

	ymm0 = _mm256_loadu_si256((__m256i*)block1);
	ymm1 = _mm256_loadu_si256((__m256i*)(block1 + offset));
	ymm2 = _mm256_loadu_si256((__m256i*)(block1 + offset * 2));
	ymm3 = _mm256_loadu_si256((__m256i*)(block1 + offset * 3));
	ymm4 = _mm256_loadu_si256((__m256i*)(block1 + offset * 4));
	ymm5 = _mm256_loadu_si256((__m256i*)(block1 + offset * 5));
	ymm6 = _mm256_loadu_si256((__m256i*)(block1 + offset * 6));
	ymm7 = _mm256_loadu_si256((__m256i*)(block1 + offset * 7));
	ymm8 = _mm256_loadu_si256((__m256i*)(block1 + offset * 8));
	ymm9 = _mm256_loadu_si256((__m256i*)(block1 + offset * 9));
	ymm10 = _mm256_loadu_si256((__m256i*)(block1 + offset * 10));
	ymm11 = _mm256_loadu_si256((__m256i*)(block1 + offset * 11));
	ymm12 = _mm256_loadu_si256((__m256i*)(block1 + offset * 12));
	ymm13 = _mm256_loadu_si256((__m256i*)(block1 + offset * 13));
	ymm14 = _mm256_loadu_si256((__m256i*)(block1 + offset * 14));
	ymm15 = _mm256_loadu_si256((__m256i*)(block1 + offset * 15));

	//sum absolute differences

	ymm0 = _mm256_sad_epu8(ymm0, *(__m256i*)(block2));
	ymm1 = _mm256_sad_epu8(ymm1, *(__m256i*)(block2 + offset));
	ymm2 = _mm256_sad_epu8(ymm2, *(__m256i*)(block2 + offset * 2));
	ymm3 = _mm256_sad_epu8(ymm3, *(__m256i*)(block2 + offset * 3));
	ymm4 = _mm256_sad_epu8(ymm4, *(__m256i*)(block2 + offset * 4));
	ymm5 = _mm256_sad_epu8(ymm5, *(__m256i*)(block2 + offset * 5));
	ymm6 = _mm256_sad_epu8(ymm6, *(__m256i*)(block2 + offset * 6));
	ymm7 = _mm256_sad_epu8(ymm7, *(__m256i*)(block2 + offset * 7));
	ymm8 = _mm256_sad_epu8(ymm8, *(__m256i*)(block2 + offset * 8));
	ymm9 = _mm256_sad_epu8(ymm9, *(__m256i*)(block2 + offset * 9));
	ymm10 = _mm256_sad_epu8(ymm10, *(__m256i*)(block2 + offset * 10));
	ymm11 = _mm256_sad_epu8(ymm11, *(__m256i*)(block2 + offset * 11));
	ymm12 = _mm256_sad_epu8(ymm12, *(__m256i*)(block2 + offset * 12));
	ymm13 = _mm256_sad_epu8(ymm13, *(__m256i*)(block2 + offset * 13));
	ymm14 = _mm256_sad_epu8(ymm14, *(__m256i*)(block2 + offset * 14));
	ymm15 = _mm256_sad_epu8(ymm15, *(__m256i*)(block2 + offset * 15));

	ymm0 = _mm256_add_epi32(ymm0, ymm1);
	ymm2 = _mm256_add_epi32(ymm2, ymm3);
	ymm4 = _mm256_add_epi32(ymm4, ymm5);
	ymm6 = _mm256_add_epi32(ymm6, ymm7);
	ymm8 = _mm256_add_epi32(ymm8, ymm9);
	ymm10 = _mm256_add_epi32(ymm10, ymm11);
	ymm12 = _mm256_add_epi32(ymm12, ymm13);
	ymm14 = _mm256_add_epi32(ymm14, ymm15);

	ymm0 = _mm256_add_epi32(ymm0, ymm2);
	ymm4 = _mm256_add_epi32(ymm4, ymm6);
	ymm8 = _mm256_add_epi32(ymm8, ymm10);
	ymm12 = _mm256_add_epi32(ymm12, ymm14);

	ymm0 = _mm256_add_epi32(ymm0, ymm4);
	ymm8 = _mm256_add_epi32(ymm8, ymm12);

	ymm0 = _mm256_add_epi32(ymm0, ymm8);

	xmm1 = _mm256_extracti128_si256(ymm0, 1);

	xmm0 = _mm_slli_si128(_mm256_extracti128_si256(ymm0, 0), 4);
	xmm0 = _mm_or_si128(xmm0, xmm1);

	xmm0 = _mm_hadd_epi32(xmm0, xmm0);
	xmm0 = _mm_hadd_epi32(xmm0, xmm0);

	res = _mm_cvtsi128_si32(xmm0);

	//load 32x16 block1

	ymm0 = _mm256_loadu_si256((__m256i*)(block1 + offset * 16));
	ymm1 = _mm256_loadu_si256((__m256i*)(block1 + offset * 17));
	ymm2 = _mm256_loadu_si256((__m256i*)(block1 + offset * 18));
	ymm3 = _mm256_loadu_si256((__m256i*)(block1 + offset * 19));
	ymm4 = _mm256_loadu_si256((__m256i*)(block1 + offset * 20));
	ymm5 = _mm256_loadu_si256((__m256i*)(block1 + offset * 21));
	ymm6 = _mm256_loadu_si256((__m256i*)(block1 + offset * 22));
	ymm7 = _mm256_loadu_si256((__m256i*)(block1 + offset * 23));
	ymm8 = _mm256_loadu_si256((__m256i*)(block1 + offset * 24));
	ymm9 = _mm256_loadu_si256((__m256i*)(block1 + offset * 25));
	ymm10 = _mm256_loadu_si256((__m256i*)(block1 + offset * 26));
	ymm11 = _mm256_loadu_si256((__m256i*)(block1 + offset * 27));
	ymm12 = _mm256_loadu_si256((__m256i*)(block1 + offset * 28));
	ymm13 = _mm256_loadu_si256((__m256i*)(block1 + offset * 29));
	ymm14 = _mm256_loadu_si256((__m256i*)(block1 + offset * 30));
	ymm15 = _mm256_loadu_si256((__m256i*)(block1 + offset * 31));

	//sum absolute differences

	ymm0 = _mm256_sad_epu8(ymm0, *(__m256i*)(block2 + offset * 16));
	ymm1 = _mm256_sad_epu8(ymm1, *(__m256i*)(block2 + offset * 17));
	ymm2 = _mm256_sad_epu8(ymm2, *(__m256i*)(block2 + offset * 18));
	ymm3 = _mm256_sad_epu8(ymm3, *(__m256i*)(block2 + offset * 19));
	ymm4 = _mm256_sad_epu8(ymm4, *(__m256i*)(block2 + offset * 20));
	ymm5 = _mm256_sad_epu8(ymm5, *(__m256i*)(block2 + offset * 21));
	ymm6 = _mm256_sad_epu8(ymm6, *(__m256i*)(block2 + offset * 22));
	ymm7 = _mm256_sad_epu8(ymm7, *(__m256i*)(block2 + offset * 23));
	ymm8 = _mm256_sad_epu8(ymm8, *(__m256i*)(block2 + offset * 24));
	ymm9 = _mm256_sad_epu8(ymm9, *(__m256i*)(block2 + offset * 25));
	ymm10 = _mm256_sad_epu8(ymm10, *(__m256i*)(block2 + offset * 26));
	ymm11 = _mm256_sad_epu8(ymm11, *(__m256i*)(block2 + offset * 27));
	ymm12 = _mm256_sad_epu8(ymm12, *(__m256i*)(block2 + offset * 28));
	ymm13 = _mm256_sad_epu8(ymm13, *(__m256i*)(block2 + offset * 29));
	ymm14 = _mm256_sad_epu8(ymm14, *(__m256i*)(block2 + offset * 30));
	ymm15 = _mm256_sad_epu8(ymm15, *(__m256i*)(block2 + offset * 31));

	ymm0 = _mm256_add_epi32(ymm0, ymm1);
	ymm2 = _mm256_add_epi32(ymm2, ymm3);
	ymm4 = _mm256_add_epi32(ymm4, ymm5);
	ymm6 = _mm256_add_epi32(ymm6, ymm7);
	ymm8 = _mm256_add_epi32(ymm8, ymm9);
	ymm10 = _mm256_add_epi32(ymm10, ymm11);
	ymm12 = _mm256_add_epi32(ymm12, ymm13);
	ymm14 = _mm256_add_epi32(ymm14, ymm15);

	ymm0 = _mm256_add_epi32(ymm0, ymm2);
	ymm4 = _mm256_add_epi32(ymm4, ymm6);
	ymm8 = _mm256_add_epi32(ymm8, ymm10);
	ymm12 = _mm256_add_epi32(ymm12, ymm14);

	ymm0 = _mm256_add_epi32(ymm0, ymm4);
	ymm8 = _mm256_add_epi32(ymm8, ymm12);

	ymm0 = _mm256_add_epi32(ymm0, ymm8);

	xmm1 = _mm256_extracti128_si256(ymm0, 1);

	xmm0 = _mm_slli_si128(_mm256_extracti128_si256(ymm0, 0), 4);
	xmm0 = _mm_or_si128(xmm0, xmm1);

	xmm0 = _mm_hadd_epi32(xmm0, xmm0);
	xmm0 = _mm_hadd_epi32(xmm0, xmm0);

	return res + _mm_cvtsi128_si32(xmm0);
}

SSSE3_TARGET uint64_t sad_u8_96x32_ssse3(uint8_t* block1, uint8_t* block2, uint64_t offset)
{
	//todo
	return sad_u8_rectangle(block1, block2, offset, 96, 32);
}

AVX2_TARGET uint64_t sad_u8_96x32_avx2(uint8_t* block1, uint8_t* block2, uint64_t offset)
{
	//todo
	return sad_u8_rectangle(block1, block2, offset, 96, 32);
}

#endif // x86