#include "sad.h"

#include <math.h>

uint64_t sad_u8_rectangle(uint8_t* block1, uint8_t* block2, uint64_t offset, uint64_t width, uint64_t height)
{
	uint64_t res = 0;

	for (size_t i = 0; i < height; i++)
	{
		for (size_t j = 0; j < width; j++)
		{
			res += abs((int16_t)block1[i * offset + j] - (int16_t)block2[i * offset + j]);
		}
	}

	return res;
}

SSE2_TARGET uint64_t sad_u8_rectangle_sse2(uint8_t* block1, uint8_t* block2, uint64_t offset, uint64_t width, uint64_t height)
{
	if (width % 16)
	{
		return sad_u8_rectangle(block1, block2, offset, width, height);
	}

	__m128i tmp = _mm_setzero_si128();

	for (size_t i = 0; i < height; i++)
	{
		for (size_t j = 0; j < width / 16; j++)
		{
			__m128i sad = _mm_sad_epu8(
				_mm_loadu_si128((__m128i*)(block1 + i * offset + j * sizeof(__m128i))),
				_mm_loadu_si128((__m128i*)(block2 + i * offset + j * sizeof(__m128i)))
			);

			tmp = _mm_add_epi64(tmp, sad);
		}
	}

	int64_t tmp_array[2];
	_mm_storeu_si128((__m128i*)tmp_array, tmp);

	return tmp_array[0] + tmp_array[1];
}

AVX2_TARGET uint64_t sad_u8_rectangle_avx2(uint8_t* block1, uint8_t* block2, uint64_t offset, uint64_t width, uint64_t height)
{
	if (width % 32)
	{
		return sad_u8_rectangle_sse2(block1, block2, offset, width, height);
	}

	__m256i tmp = _mm256_setzero_si256();

	for (size_t i = 0; i < height; i++)
	{
		for (size_t j = 0; j < width / 32; j++)
		{
			__m256i sad = _mm256_sad_epu8(
				_mm256_loadu_si256((__m256i*)(block1 + i * offset + j * sizeof(__m256i))),
				_mm256_loadu_si256((__m256i*)(block2 + i * offset + j * sizeof(__m256i)))
			);

			tmp = _mm256_add_epi64(tmp, sad);
		}
	}

	int64_t tmp_array[4];
	_mm256_storeu_si256((__m256i*)tmp_array, tmp);

	return tmp_array[0] + tmp_array[1] + tmp_array[2] + tmp_array[3];
}