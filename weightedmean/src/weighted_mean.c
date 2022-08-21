
#include "weighted_mean.h"

uint64_t shift_division(uint64_t integer)
{
	for (size_t i = 1; i < 63; i++)
		if ((1 << i) == integer)
			return i;

	return 0;
}

void weightedmean_u8_i64(uint8_t** blocks, int64_t* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result)
{
	int64_t total_weights = 0;
	for (size_t i = 0; i < block_count; i++)
		total_weights += weights[i];

	int64_t tmp;

	uint64_t shift = shift_division(total_weights);
	if (shift)
	{
		//use shift to divide

		for (size_t i = 0; i < block_element_count; i++)
		{
			tmp = 0;

			for (size_t j = 0; j < block_count; j++)
			{
				tmp += blocks[j][i] * weights[j];
			}

			result[i] = (uint8_t)(tmp >> shift);
		}
	}
	else
	{
		//use classic division

		for (size_t i = 0; i < block_element_count; i++)
		{
			tmp = 0;

			for (size_t j = 0; j < block_count; j++)
			{
				tmp += blocks[j][i] * weights[j];
			}

			result[i] = (uint8_t)(tmp / total_weights);
		}
	}
}

void weightedmean_u8_f64(uint8_t** blocks, double* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result)
{
	double total_weights = 0.0;
	for (size_t i = 0; i < block_count; i++)
		total_weights += (double)weights[i];

	double tmp;

	for (size_t i = 0; i < block_element_count; i++)
	{
		tmp = 0.0;

		for (size_t j = 0; j < block_count; j++)
		{
			tmp += (double)blocks[j][i] * weights[j];
		}

		result[i] = (uint8_t)(tmp / total_weights);
	}
}

#if defined(X86_32) || defined(X86_64)

#include <emmintrin.h>
#include <immintrin.h>


void weightedmean_u8_i64_x86(uint8_t** blocks, int64_t* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result)
{
	int64_t total_weights = 0;
	for (size_t i = 0; i < block_count; i++)
		total_weights += weights[i];

	int64_t tmp, tmp2, tmp3, tmp4;

	uint64_t shift = shift_division(total_weights);
	if (shift)
	{
		//use shift to divide

		for (size_t i = 0; i < (block_element_count & 0xfffffffffffffffc); i += 4)
		{
			tmp = 0;
			tmp2 = 0;
			tmp3 = 0;
			tmp4 = 0;

			for (size_t j = 0; j < block_count; j++)
			{
				tmp += blocks[j][i] * weights[j];
				tmp2 += blocks[j][i + 1] * weights[j];
				tmp3 += blocks[j][i + 2] * weights[j];
				tmp4 += blocks[j][i + 3] * weights[j];
			}

			result[i] = (uint8_t)(tmp >> shift);
			result[i + 1] = (uint8_t)(tmp2 >> shift);
			result[i + 2] = (uint8_t)(tmp3 >> shift);
			result[i + 3] = (uint8_t)(tmp4 >> shift);
		}

		//last elements
		
		for (size_t i = block_element_count & 0xfffffffffffffffc; i < block_element_count; i++)
		{
			tmp = 0;

			for (size_t j = 0; j < block_count; j++)
			{
				tmp += blocks[j][i] * weights[j];
			}

			result[i] = (uint8_t)(tmp >> shift);
		}
	}
	else
	{
		//use classic division

		for (size_t i = 0; i < (block_element_count & 0xfffffffffffffffc); i += 4)
		{
			tmp = 0;
			tmp2 = 0;
			tmp3 = 0;
			tmp4 = 0;

			for (size_t j = 0; j < block_count; j++)
			{
				tmp += blocks[j][i] * weights[j];
				tmp2 += blocks[j][i + 1] * weights[j];
				tmp3 += blocks[j][i + 2] * weights[j];
				tmp4 += blocks[j][i + 3] * weights[j];
			}

			result[i] = (uint8_t)(tmp / total_weights);
			result[i + 1] = (uint8_t)(tmp2 / total_weights);
			result[i + 2] = (uint8_t)(tmp3 / total_weights);
			result[i + 3] = (uint8_t)(tmp4 / total_weights);
		}

		//last elements
		
		for (size_t i = block_element_count & 0xfffffffffffffffc; i < block_element_count; i++)
		{
			tmp = 0;

			for (size_t j = 0; j < block_count; j++)
			{
				tmp += blocks[j][i] * weights[j];
			}

			result[i] = (uint8_t)(tmp / total_weights);
		}
	}
}

void weightedmean_u8_f64_x86(uint8_t** blocks, double* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result)
{
	double total_weights = 0.0;
	for (size_t i = 0; i < block_count; i++)
		total_weights += weights[i];

	total_weights = 1.0 / total_weights;

	double tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;

	for (size_t i = 0; i < (block_element_count & 0xfffffffffffffff8); i += 8)
	{
		tmp1 = 0.0;
		tmp2 = 0.0;
		tmp3 = 0.0;
		tmp4 = 0.0;
		tmp5 = 0.0;
		tmp6 = 0.0;
		tmp7 = 0.0;
		tmp8 = 0.0;

		for (size_t j = 0; j < block_count; j++)
		{
			tmp1 += (double)blocks[j][i] * weights[j];
			tmp2 += (double)blocks[j][i + 1] * weights[j];
			tmp3 += (double)blocks[j][i + 2] * weights[j];
			tmp4 += (double)blocks[j][i + 3] * weights[j];
			tmp5 += (double)blocks[j][i + 4] * weights[j];
			tmp6 += (double)blocks[j][i + 5] * weights[j];
			tmp7 += (double)blocks[j][i + 6] * weights[j];
			tmp8 += (double)blocks[j][i + 7] * weights[j];
		}

		result[i] = (uint8_t)(tmp1 * total_weights);
		result[i + 1] = (uint8_t)(tmp2 * total_weights);
		result[i + 2] = (uint8_t)(tmp3 * total_weights);
		result[i + 3] = (uint8_t)(tmp4 * total_weights);
		result[i + 4] = (uint8_t)(tmp5 * total_weights);
		result[i + 5] = (uint8_t)(tmp6 * total_weights);
		result[i + 6] = (uint8_t)(tmp7 * total_weights);
		result[i + 7] = (uint8_t)(tmp8 * total_weights);
	}

	for (size_t i = block_element_count & 0xfffffffffffffff8; i < block_element_count; i++)
	{
		tmp1 = 0.0;

		for (size_t j = 0; j < block_count; j++)
		{
			tmp1 += (double)blocks[j][i] * weights[j];
		}

		result[i] = (uint8_t)(tmp1 * total_weights);
	}
}

SSE2_TARGET void weightedmean_u8_i64_sse2_nt(uint8_t** blocks, int64_t* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result)
{
	int64_t total_weights = 0;
	for (size_t i = 0; i < block_count; i++)
		total_weights += weights[i];

	int64_t tmp, tmp2, tmp3, tmp4;

	uint64_t shift = shift_division(total_weights);
	if (shift)
	{
		//use shift to divide

		for (size_t i = 0; i < (block_element_count & 0xfffffffffffffffc); i += 4)
		{
			tmp = 0;
			tmp2 = 0;
			tmp3 = 0;
			tmp4 = 0;

			for (size_t j = 0; j < block_count; j++)
			{
				tmp += blocks[j][i] * weights[j];
				tmp2 += blocks[j][i + 1] * weights[j];
				tmp3 += blocks[j][i + 2] * weights[j];
				tmp4 += blocks[j][i + 3] * weights[j];
			}

			_mm_stream_si32((int*)(result + i), ((tmp4 >> shift) << 24) | ((tmp3 >> shift) << 16) | ((tmp2 >> shift) << 8) | (tmp >> shift));
		}

		//last elements

		for (size_t i = block_element_count & 0xfffffffffffffffc; i < block_element_count; i++)
		{
			tmp = 0;

			for (size_t j = 0; j < block_count; j++)
			{
				tmp += blocks[j][i] * weights[j];
			}

			result[i] = (uint8_t)(tmp >> shift);
		}
	}
	else
	{
		//use classic division

		for (size_t i = 0; i < (block_element_count & 0xfffffffffffffffc); i += 4)
		{
			tmp = 0;
			tmp2 = 0;
			tmp3 = 0;
			tmp4 = 0;

			for (size_t j = 0; j < block_count; j++)
			{
				tmp += blocks[j][i] * weights[j];
				tmp2 += blocks[j][i + 1] * weights[j];
				tmp3 += blocks[j][i + 2] * weights[j];
				tmp4 += blocks[j][i + 3] * weights[j];
			}

			_mm_stream_si32((int*)(result + i), ((tmp4 / total_weights) << 24) | ((tmp3 / total_weights) << 16) | ((tmp2 / total_weights) << 8) | (tmp / total_weights));
		}

		//last elements

		for (size_t i = block_element_count & 0xfffffffffffffffc; i < block_element_count; i++)
		{
			tmp = 0;

			for (size_t j = 0; j < block_count; j++)
			{
				tmp += blocks[j][i] * weights[j];
			}

			result[i] = (uint8_t)(tmp / total_weights);
		}
	}
}


SSE2_TARGET void weightedmean_u8_f64_sse2_nt(uint8_t** blocks, double* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result)
{
	double total_weights = 0.0;
	for (size_t i = 0; i < block_count; i++)
		total_weights += weights[i];

	total_weights = 1.0 / total_weights;

	double tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;

	for (size_t i = 0; i < (block_element_count & 0xfffffffffffffff8); i += 8)
	{
		tmp1 = 0.0;
		tmp2 = 0.0;
		tmp3 = 0.0;
		tmp4 = 0.0;
		tmp5 = 0.0;
		tmp6 = 0.0;
		tmp7 = 0.0;
		tmp8 = 0.0;

		for (size_t j = 0; j < block_count; j++)
		{
			tmp1 += (double)blocks[j][i] * weights[j];
			tmp2 += (double)blocks[j][i + 1] * weights[j];
			tmp3 += (double)blocks[j][i + 2] * weights[j];
			tmp4 += (double)blocks[j][i + 3] * weights[j];
			tmp5 += (double)blocks[j][i + 4] * weights[j];
			tmp6 += (double)blocks[j][i + 5] * weights[j];
			tmp7 += (double)blocks[j][i + 6] * weights[j];
			tmp8 += (double)blocks[j][i + 7] * weights[j];
		}

		_mm_stream_si32((int*)(result + i), ((uint32_t)(tmp4 * total_weights) << 24) | ((uint32_t)(tmp3 * total_weights) << 16) | ((uint32_t)(tmp2 * total_weights) << 8) | (uint32_t)(tmp1 * total_weights));
		_mm_stream_si32((int*)(result + i + 4), ((uint32_t)(tmp8 * total_weights) << 24) | ((uint32_t)(tmp7 * total_weights) << 16) | ((uint32_t)(tmp6 * total_weights) << 8) | (uint32_t)(tmp5 * total_weights));
	}

	for (size_t i = block_element_count & 0xfffffffffffffff8; i < block_element_count; i++)
	{
		tmp1 = 0.0;

		for (size_t j = 0; j < block_count; j++)
		{
			tmp1 += (double)blocks[j][i] * weights[j];
		}

		result[i] = (uint8_t)(tmp1 * total_weights);
	}
}

SSE41_TARGET void weightedmean_u8_i32_sse41_shift(uint8_t** blocks, int32_t* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result)
{
	int64_t total_weights = 0;
	for (size_t i = 0; i < block_count; i++)
		total_weights += weights[i];

	uint64_t shift = shift_division(total_weights);

	__m128i weight;
	__m128i tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
	__m128i shuf1, shuf2, shuf3, shuf4, shuf5;
	__m128i div;

	static int8_t unpack[] = { 0, -1, -1, -1, 1, -1, -1, -1, 2, -1, -1, -1, 3, -1, -1, -1 };
	shuf1 = _mm_lddqu_si128((__m128i*)unpack);
	static int8_t pack1[] = { 0, 4, 8, 12, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
	shuf2 = _mm_lddqu_si128((__m128i*)pack1);
	static int8_t pack2[] = { -1, -1, -1, -1, 0, 4, 8, 12, -1, -1, -1, -1, -1, -1, -1, -1 };
	shuf3 = _mm_lddqu_si128((__m128i*)pack2);
	static int8_t pack3[] = { -1, -1, -1, -1, -1, -1, -1, -1, 0, 4, 8, 12, -1, -1, -1, -1 };
	shuf4 = _mm_lddqu_si128((__m128i*)pack3);
	static int8_t pack4[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 4, 8, 12 };
	shuf5 = _mm_lddqu_si128((__m128i*)pack4);

	for (size_t i = 0; i < (block_element_count & 0xffffffffffffffe0); i += 32)
	{
		// nul
		tmp1 = _mm_xor_si128(tmp1, tmp1);
		tmp2 = _mm_xor_si128(tmp2, tmp2);
		tmp3 = _mm_xor_si128(tmp3, tmp3);
		tmp4 = _mm_xor_si128(tmp4, tmp4);
		tmp5 = _mm_xor_si128(tmp5, tmp5);
		tmp6 = _mm_xor_si128(tmp6, tmp6);
		tmp7 = _mm_xor_si128(tmp7, tmp7);
		tmp8 = _mm_xor_si128(tmp8, tmp8);

		for (size_t j = 0; j < block_count; j++)
		{
			weight = _mm_set1_epi32(weights[j]);

			xmm0 = _mm_cvtsi32_si128(*(int*)(blocks[j] + i));
			xmm1 = _mm_cvtsi32_si128(*(int*)(blocks[j] + i + 4));
			xmm2 = _mm_cvtsi32_si128(*(int*)(blocks[j] + i + 8));
			xmm3 = _mm_cvtsi32_si128(*(int*)(blocks[j] + i + 12));
			xmm4 = _mm_cvtsi32_si128(*(int*)(blocks[j] + i + 16));
			xmm5 = _mm_cvtsi32_si128(*(int*)(blocks[j] + i + 20));
			xmm6 = _mm_cvtsi32_si128(*(int*)(blocks[j] + i + 24));
			xmm7 = _mm_cvtsi32_si128(*(int*)(blocks[j] + i + 28));

			xmm0 = _mm_shuffle_epi8(xmm0, shuf1);
			xmm1 = _mm_shuffle_epi8(xmm1, shuf1);
			xmm2 = _mm_shuffle_epi8(xmm2, shuf1);
			xmm3 = _mm_shuffle_epi8(xmm3, shuf1);
			xmm4 = _mm_shuffle_epi8(xmm4, shuf1);
			xmm5 = _mm_shuffle_epi8(xmm5, shuf1);
			xmm6 = _mm_shuffle_epi8(xmm6, shuf1);
			xmm7 = _mm_shuffle_epi8(xmm7, shuf1);

			xmm0 = _mm_mullo_epi32(xmm0, weight);
			xmm1 = _mm_mullo_epi32(xmm1, weight);
			xmm2 = _mm_mullo_epi32(xmm2, weight);
			xmm3 = _mm_mullo_epi32(xmm3, weight);
			xmm4 = _mm_mullo_epi32(xmm4, weight);
			xmm5 = _mm_mullo_epi32(xmm5, weight);
			xmm6 = _mm_mullo_epi32(xmm6, weight);
			xmm7 = _mm_mullo_epi32(xmm7, weight);

			tmp1 = _mm_add_epi32(tmp1, xmm0);
			tmp2 = _mm_add_epi32(tmp2, xmm1);
			tmp3 = _mm_add_epi32(tmp3, xmm2);
			tmp4 = _mm_add_epi32(tmp4, xmm3);
			tmp5 = _mm_add_epi32(tmp5, xmm4);
			tmp6 = _mm_add_epi32(tmp6, xmm5);
			tmp7 = _mm_add_epi32(tmp7, xmm6);
			tmp8 = _mm_add_epi32(tmp8, xmm7);
		}

		//division
		tmp1 = _mm_srli_epi32(tmp1, shift);
		tmp2 = _mm_srli_epi32(tmp2, shift);
		tmp3 = _mm_srli_epi32(tmp3, shift);
		tmp4 = _mm_srli_epi32(tmp4, shift);
		tmp5 = _mm_srli_epi32(tmp5, shift);
		tmp6 = _mm_srli_epi32(tmp6, shift);
		tmp7 = _mm_srli_epi32(tmp7, shift);
		tmp8 = _mm_srli_epi32(tmp8, shift);

		tmp1 = _mm_shuffle_epi8(tmp1, shuf2);
		tmp2 = _mm_shuffle_epi8(tmp2, shuf3);
		tmp3 = _mm_shuffle_epi8(tmp3, shuf4);
		tmp4 = _mm_shuffle_epi8(tmp4, shuf5);
		tmp5 = _mm_shuffle_epi8(tmp5, shuf2);
		tmp6 = _mm_shuffle_epi8(tmp6, shuf3);
		tmp7 = _mm_shuffle_epi8(tmp7, shuf4);
		tmp8 = _mm_shuffle_epi8(tmp8, shuf5);

		tmp1 = _mm_or_si128(tmp1, tmp2);
		tmp3 = _mm_or_si128(tmp3, tmp4);
		tmp5 = _mm_or_si128(tmp5, tmp6);
		tmp7 = _mm_or_si128(tmp7, tmp8);

		tmp1 = _mm_or_si128(tmp1, tmp3);
		tmp5 = _mm_or_si128(tmp5, tmp7);

		_mm_storeu_si128((__m128i*)(result + i), tmp1);
		_mm_storeu_si128((__m128i*)(result + i + 16), tmp5);
	}

	uint64_t tmp;

	//last elements

	for (size_t i = block_element_count & 0xffffffffffffffe0; i < block_element_count; i++)
	{
		tmp = 0;

		for (size_t j = 0; j < block_count; j++)
		{
			tmp += blocks[j][i] * weights[j];
		}

		result[i] = (uint8_t)(tmp >> shift);
	}
}

SSE41_TARGET void weightedmean_u8_i32_sse41_nt_shift(uint8_t** blocks, int32_t* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result)
{
	int64_t total_weights = 0;
	for (size_t i = 0; i < block_count; i++)
		total_weights += weights[i];

	uint64_t shift = shift_division(total_weights);

	__m128i weight;
	__m128i tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
	__m128i shuf1, shuf2, shuf3, shuf4, shuf5;
	__m128i div;

	static int8_t unpack[] = { 0, -1, -1, -1, 1, -1, -1, -1, 2, -1, -1, -1, 3, -1, -1, -1 };
	shuf1 = _mm_lddqu_si128((__m128i*)unpack);
	static int8_t pack1[] = { 0, 4, 8, 12, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
	shuf2 = _mm_lddqu_si128((__m128i*)pack1);
	static int8_t pack2[] = { -1, -1, -1, -1, 0, 4, 8, 12, -1, -1, -1, -1, -1, -1, -1, -1 };
	shuf3 = _mm_lddqu_si128((__m128i*)pack2);
	static int8_t pack3[] = { -1, -1, -1, -1, -1, -1, -1, -1, 0, 4, 8, 12, -1, -1, -1, -1 };
	shuf4 = _mm_lddqu_si128((__m128i*)pack3);
	static int8_t pack4[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 4, 8, 12 };
	shuf5 = _mm_lddqu_si128((__m128i*)pack4);

	uint64_t tmp;

	// align 16

	size_t i;

	for (i = 0; i < (16 - ((size_t)result & 15)); i++)
	{
		tmp = 0;

		for (size_t j = 0; j < block_count; j++)
		{
			tmp += blocks[j][i] * weights[j];
		}

		result[i] = (uint8_t)(tmp >> shift);
	}

	for (; i < ((block_element_count - 16 + ((size_t)result & 15)) & 0xffffffffffffffe0); i += 32)
	{
		// nul
		tmp1 = _mm_xor_si128(tmp1, tmp1);
		tmp2 = _mm_xor_si128(tmp2, tmp2);
		tmp3 = _mm_xor_si128(tmp3, tmp3);
		tmp4 = _mm_xor_si128(tmp4, tmp4);
		tmp5 = _mm_xor_si128(tmp5, tmp5);
		tmp6 = _mm_xor_si128(tmp6, tmp6);
		tmp7 = _mm_xor_si128(tmp7, tmp7);
		tmp8 = _mm_xor_si128(tmp8, tmp8);

		for (size_t j = 0; j < block_count; j++)
		{
			weight = _mm_set1_epi32(weights[j]);

			xmm0 = _mm_cvtsi32_si128(*(int*)(blocks[j] + i));
			xmm1 = _mm_cvtsi32_si128(*(int*)(blocks[j] + i + 4));
			xmm2 = _mm_cvtsi32_si128(*(int*)(blocks[j] + i + 8));
			xmm3 = _mm_cvtsi32_si128(*(int*)(blocks[j] + i + 12));
			xmm4 = _mm_cvtsi32_si128(*(int*)(blocks[j] + i + 16));
			xmm5 = _mm_cvtsi32_si128(*(int*)(blocks[j] + i + 20));
			xmm6 = _mm_cvtsi32_si128(*(int*)(blocks[j] + i + 24));
			xmm7 = _mm_cvtsi32_si128(*(int*)(blocks[j] + i + 28));

			xmm0 = _mm_shuffle_epi8(xmm0, shuf1);
			xmm1 = _mm_shuffle_epi8(xmm1, shuf1);
			xmm2 = _mm_shuffle_epi8(xmm2, shuf1);
			xmm3 = _mm_shuffle_epi8(xmm3, shuf1);
			xmm4 = _mm_shuffle_epi8(xmm4, shuf1);
			xmm5 = _mm_shuffle_epi8(xmm5, shuf1);
			xmm6 = _mm_shuffle_epi8(xmm6, shuf1);
			xmm7 = _mm_shuffle_epi8(xmm7, shuf1);

			xmm0 = _mm_mullo_epi32(xmm0, weight);
			xmm1 = _mm_mullo_epi32(xmm1, weight);
			xmm2 = _mm_mullo_epi32(xmm2, weight);
			xmm3 = _mm_mullo_epi32(xmm3, weight);
			xmm4 = _mm_mullo_epi32(xmm4, weight);
			xmm5 = _mm_mullo_epi32(xmm5, weight);
			xmm6 = _mm_mullo_epi32(xmm6, weight);
			xmm7 = _mm_mullo_epi32(xmm7, weight);

			tmp1 = _mm_add_epi32(tmp1, xmm0);
			tmp2 = _mm_add_epi32(tmp2, xmm1);
			tmp3 = _mm_add_epi32(tmp3, xmm2);
			tmp4 = _mm_add_epi32(tmp4, xmm3);
			tmp5 = _mm_add_epi32(tmp5, xmm4);
			tmp6 = _mm_add_epi32(tmp6, xmm5);
			tmp7 = _mm_add_epi32(tmp7, xmm6);
			tmp8 = _mm_add_epi32(tmp8, xmm7);
		}

		//division
		tmp1 = _mm_srli_epi32(tmp1, shift);
		tmp2 = _mm_srli_epi32(tmp2, shift);
		tmp3 = _mm_srli_epi32(tmp3, shift);
		tmp4 = _mm_srli_epi32(tmp4, shift);
		tmp5 = _mm_srli_epi32(tmp5, shift);
		tmp6 = _mm_srli_epi32(tmp6, shift);
		tmp7 = _mm_srli_epi32(tmp7, shift);
		tmp8 = _mm_srli_epi32(tmp8, shift);

		//store

		tmp1 = _mm_shuffle_epi8(tmp1, shuf2);
		tmp2 = _mm_shuffle_epi8(tmp2, shuf3);
		tmp3 = _mm_shuffle_epi8(tmp3, shuf4);
		tmp4 = _mm_shuffle_epi8(tmp4, shuf5);
		tmp5 = _mm_shuffle_epi8(tmp5, shuf2);
		tmp6 = _mm_shuffle_epi8(tmp6, shuf3);
		tmp7 = _mm_shuffle_epi8(tmp7, shuf4);
		tmp8 = _mm_shuffle_epi8(tmp8, shuf5);

		tmp1 = _mm_or_si128(tmp1, tmp2);
		tmp3 = _mm_or_si128(tmp3, tmp4);
		tmp5 = _mm_or_si128(tmp5, tmp6);
		tmp7 = _mm_or_si128(tmp7, tmp8);

		tmp1 = _mm_or_si128(tmp1, tmp3);
		tmp5 = _mm_or_si128(tmp5, tmp7);

		_mm_stream_si128((__m128i*)(result + i), tmp1);
		_mm_stream_si128((__m128i*)(result + i + 16), tmp5);
	}

	//last elements

	for (; i < block_element_count; i++)
	{
		tmp = 0;

		for (size_t j = 0; j < block_count; j++)
		{
			tmp += blocks[j][i] * weights[j];
		}

		result[i] = (uint8_t)(tmp >> shift);
	}
}

SSE41_TARGET void weightedmean_u8_f32_sse41(uint8_t** blocks, float* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result)
{
	float total_weights = 0.0f;
	for (size_t i = 0; i < block_count; i++)
		total_weights += weights[i];

	total_weights = 1.0f / total_weights;

	//use classic division
	
	__m128i shuf1, shuf2, shuf3, shuf4, shuf5;
	__m128 div;
	__m128 weight;
	__m128 tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
	__m128 xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
	__m128i tmpi1, tmpi2, tmpi3, tmpi4, tmpi5, tmpi6, tmpi7, tmpi8;

	div = _mm_set_ps1(total_weights);

	static int8_t unpack[] = { 0, -1, -1, -1, 1, -1, -1, -1, 2, -1, -1, -1, 3, -1, -1, -1 };
	shuf1 = _mm_lddqu_si128((__m128i*)unpack);
	static int8_t pack1[] = { 0, 4, 8, 12, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
	shuf2 = _mm_lddqu_si128((__m128i*)pack1);
	static int8_t pack2[] = { -1, -1, -1, -1, 0, 4, 8, 12, -1, -1, -1, -1, -1, -1, -1, -1 };
	shuf3 = _mm_lddqu_si128((__m128i*)pack2);
	static int8_t pack3[] = { -1, -1, -1, -1, -1, -1, -1, -1, 0, 4, 8, 12, -1, -1, -1, -1 };
	shuf4 = _mm_lddqu_si128((__m128i*)pack3);
	static int8_t pack4[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 4, 8, 12 };
	shuf5 = _mm_lddqu_si128((__m128i*)pack4);

	for (size_t i = 0; i < (block_element_count & 0xffffffffffffffe0); i += 32)
	{
		// nul
		tmp1 = _mm_set_ps1(0.0f);
		tmp2 = _mm_set_ps1(0.0f);
		tmp3 = _mm_set_ps1(0.0f);
		tmp4 = _mm_set_ps1(0.0f);
		tmp5 = _mm_set_ps1(0.0f);
		tmp6 = _mm_set_ps1(0.0f);
		tmp7 = _mm_set_ps1(0.0f);
		tmp8 = _mm_set_ps1(0.0f);

		for (size_t j = 0; j < block_count; j++)
		{
			//load weights
			weight = _mm_cvtepi32_ps(_mm_shuffle_epi32(_mm_cvtsi32_si128(weights[j]), 0));

			//load data
			xmm0 = _mm_cvtepi32_ps(_mm_shuffle_epi8(_mm_cvtsi32_si128(*(int32_t*)(blocks[j] + i)), shuf1));
			xmm1 = _mm_cvtepi32_ps(_mm_shuffle_epi8(_mm_cvtsi32_si128(*(int32_t*)(blocks[j] + i + 4)), shuf1));
			xmm2 = _mm_cvtepi32_ps(_mm_shuffle_epi8(_mm_cvtsi32_si128(*(int32_t*)(blocks[j] + i + 8)), shuf1));
			xmm3 = _mm_cvtepi32_ps(_mm_shuffle_epi8(_mm_cvtsi32_si128(*(int32_t*)(blocks[j] + i + 12)), shuf1));
			xmm4 = _mm_cvtepi32_ps(_mm_shuffle_epi8(_mm_cvtsi32_si128(*(int32_t*)(blocks[j] + i + 16)), shuf1));
			xmm5 = _mm_cvtepi32_ps(_mm_shuffle_epi8(_mm_cvtsi32_si128(*(int32_t*)(blocks[j] + i + 20)), shuf1));
			xmm6 = _mm_cvtepi32_ps(_mm_shuffle_epi8(_mm_cvtsi32_si128(*(int32_t*)(blocks[j] + i + 24)), shuf1));
			xmm7 = _mm_cvtepi32_ps(_mm_shuffle_epi8(_mm_cvtsi32_si128(*(int32_t*)(blocks[j] + i + 28)), shuf1));

			xmm0 = _mm_mul_ps(xmm0, weight);
			xmm1 = _mm_mul_ps(xmm1, weight);
			xmm2 = _mm_mul_ps(xmm2, weight);
			xmm3 = _mm_mul_ps(xmm3, weight);
			xmm4 = _mm_mul_ps(xmm4, weight);
			xmm5 = _mm_mul_ps(xmm5, weight);
			xmm6 = _mm_mul_ps(xmm6, weight);
			xmm7 = _mm_mul_ps(xmm7, weight);

			tmp1 = _mm_add_ps(tmp1, xmm0);
			tmp2 = _mm_add_ps(tmp2, xmm1);
			tmp3 = _mm_add_ps(tmp3, xmm2);
			tmp4 = _mm_add_ps(tmp4, xmm3);
			tmp5 = _mm_add_ps(tmp5, xmm4);
			tmp6 = _mm_add_ps(tmp6, xmm5);
			tmp7 = _mm_add_ps(tmp7, xmm6);
			tmp8 = _mm_add_ps(tmp8, xmm7);
		}

		//division

		tmp1 = _mm_mul_ps(tmp1, div);
		tmp2 = _mm_mul_ps(tmp2, div);
		tmp3 = _mm_mul_ps(tmp3, div);
		tmp4 = _mm_mul_ps(tmp4, div);
		tmp5 = _mm_mul_ps(tmp5, div);
		tmp6 = _mm_mul_ps(tmp6, div);
		tmp7 = _mm_mul_ps(tmp7, div);
		tmp8 = _mm_mul_ps(tmp8, div);
		
		tmp1 = _mm_floor_ps(tmp1);
		tmp2 = _mm_floor_ps(tmp2);
		tmp3 = _mm_floor_ps(tmp3);
		tmp4 = _mm_floor_ps(tmp4);
		tmp5 = _mm_floor_ps(tmp5);
		tmp6 = _mm_floor_ps(tmp6);
		tmp7 = _mm_floor_ps(tmp7);
		tmp8 = _mm_floor_ps(tmp8);

		tmpi1 = _mm_cvtps_epi32(tmp1);
		tmpi2 = _mm_cvtps_epi32(tmp2);
		tmpi3 = _mm_cvtps_epi32(tmp3);
		tmpi4 = _mm_cvtps_epi32(tmp4);
		tmpi5 = _mm_cvtps_epi32(tmp5);
		tmpi6 = _mm_cvtps_epi32(tmp6);
		tmpi7 = _mm_cvtps_epi32(tmp7);
		tmpi8 = _mm_cvtps_epi32(tmp8);

		//store

		tmpi1 = _mm_shuffle_epi8(tmpi1, shuf2);
		tmpi2 = _mm_shuffle_epi8(tmpi2, shuf3);
		tmpi3 = _mm_shuffle_epi8(tmpi3, shuf4);
		tmpi4 = _mm_shuffle_epi8(tmpi4, shuf5);
		tmpi5 = _mm_shuffle_epi8(tmpi5, shuf2);
		tmpi6 = _mm_shuffle_epi8(tmpi6, shuf3);
		tmpi7 = _mm_shuffle_epi8(tmpi7, shuf4);
		tmpi8 = _mm_shuffle_epi8(tmpi8, shuf5);
		
		tmpi1 = _mm_or_si128(tmpi1, tmpi2);
		tmpi3 = _mm_or_si128(tmpi3, tmpi4);
		tmpi5 = _mm_or_si128(tmpi5, tmpi6);
		tmpi7 = _mm_or_si128(tmpi7, tmpi8);

		tmpi1 = _mm_or_si128(tmpi1, tmpi3);
		tmpi5 = _mm_or_si128(tmpi5, tmpi7);

		_mm_storeu_si128((__m128i*)(result + i), tmpi1);
		_mm_storeu_si128((__m128i*)(result + i + 16), tmpi5);
	}

	float tmp;

	//last elements

	for (size_t i = block_element_count & 0xffffffffffffffe0; i < block_element_count; i++)
	{
		tmp = 0.0f;

		for (size_t j = 0; j < block_count; j++)
		{
			tmp += (float)blocks[j][i] * weights[j];
		}

		result[i] = (uint8_t)(tmp * total_weights);
	}
}

SSE41_TARGET void weightedmean_u8_f32_sse41_nt(uint8_t** blocks, float* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result)
{
	float total_weights = 0.0f;
	for (size_t i = 0; i < block_count; i++)
		total_weights += weights[i];

	total_weights = 1.0f / total_weights;

	//use classic division

	__m128i shuf1, shuf2, shuf3, shuf4, shuf5;
	__m128 div;
	__m128 weight;
	__m128 tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
	__m128 xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
	__m128i tmpi1, tmpi2, tmpi3, tmpi4, tmpi5, tmpi6, tmpi7, tmpi8;

	div = _mm_set_ps1(total_weights);

	static int8_t unpack[] = { 0, -1, -1, -1, 1, -1, -1, -1, 2, -1, -1, -1, 3, -1, -1, -1 };
	shuf1 = _mm_lddqu_si128((__m128i*)unpack);
	static int8_t pack1[] = { 0, 4, 8, 12, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
	shuf2 = _mm_lddqu_si128((__m128i*)pack1);
	static int8_t pack2[] = { -1, -1, -1, -1, 0, 4, 8, 12, -1, -1, -1, -1, -1, -1, -1, -1 };
	shuf3 = _mm_lddqu_si128((__m128i*)pack2);
	static int8_t pack3[] = { -1, -1, -1, -1, -1, -1, -1, -1, 0, 4, 8, 12, -1, -1, -1, -1 };
	shuf4 = _mm_lddqu_si128((__m128i*)pack3);
	static int8_t pack4[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 4, 8, 12 };
	shuf5 = _mm_lddqu_si128((__m128i*)pack4);

	float tmp;

	size_t i;

	// align 16

	for (i = 0; i < (16 - ((size_t)result & 15)); i++)
	{
		tmp = 0.0f;

		for (size_t j = 0; j < block_count; j++)
		{
			tmp += (float)blocks[j][i] * weights[j];
		}

		result[i] = (uint8_t)(tmp * total_weights);
	}

	for (; i < ((block_element_count - 16 + ((size_t)result & 15)) & 0xffffffffffffffe0); i += 32)
	{
		// nul
		tmp1 = _mm_set_ps1(0.0f);
		tmp2 = _mm_set_ps1(0.0f);
		tmp3 = _mm_set_ps1(0.0f);
		tmp4 = _mm_set_ps1(0.0f);
		tmp5 = _mm_set_ps1(0.0f);
		tmp6 = _mm_set_ps1(0.0f);
		tmp7 = _mm_set_ps1(0.0f);
		tmp8 = _mm_set_ps1(0.0f);

		for (size_t j = 0; j < block_count; j++)
		{
			//load weights
			weight = _mm_cvtepi32_ps(_mm_shuffle_epi32(_mm_cvtsi32_si128(weights[j]), 0));

			//load data
			xmm0 = _mm_cvtepi32_ps(_mm_shuffle_epi8(_mm_cvtsi32_si128(*(int32_t*)(blocks[j] + i)), shuf1));
			xmm1 = _mm_cvtepi32_ps(_mm_shuffle_epi8(_mm_cvtsi32_si128(*(int32_t*)(blocks[j] + i + 4)), shuf1));
			xmm2 = _mm_cvtepi32_ps(_mm_shuffle_epi8(_mm_cvtsi32_si128(*(int32_t*)(blocks[j] + i + 8)), shuf1));
			xmm3 = _mm_cvtepi32_ps(_mm_shuffle_epi8(_mm_cvtsi32_si128(*(int32_t*)(blocks[j] + i + 12)), shuf1));
			xmm4 = _mm_cvtepi32_ps(_mm_shuffle_epi8(_mm_cvtsi32_si128(*(int32_t*)(blocks[j] + i + 16)), shuf1));
			xmm5 = _mm_cvtepi32_ps(_mm_shuffle_epi8(_mm_cvtsi32_si128(*(int32_t*)(blocks[j] + i + 20)), shuf1));
			xmm6 = _mm_cvtepi32_ps(_mm_shuffle_epi8(_mm_cvtsi32_si128(*(int32_t*)(blocks[j] + i + 24)), shuf1));
			xmm7 = _mm_cvtepi32_ps(_mm_shuffle_epi8(_mm_cvtsi32_si128(*(int32_t*)(blocks[j] + i + 28)), shuf1));

			xmm0 = _mm_mul_ps(xmm0, weight);
			xmm1 = _mm_mul_ps(xmm1, weight);
			xmm2 = _mm_mul_ps(xmm2, weight);
			xmm3 = _mm_mul_ps(xmm3, weight);
			xmm4 = _mm_mul_ps(xmm4, weight);
			xmm5 = _mm_mul_ps(xmm5, weight);
			xmm6 = _mm_mul_ps(xmm6, weight);
			xmm7 = _mm_mul_ps(xmm7, weight);

			tmp1 = _mm_add_ps(tmp1, xmm0);
			tmp2 = _mm_add_ps(tmp2, xmm1);
			tmp3 = _mm_add_ps(tmp3, xmm2);
			tmp4 = _mm_add_ps(tmp4, xmm3);
			tmp5 = _mm_add_ps(tmp5, xmm4);
			tmp6 = _mm_add_ps(tmp6, xmm5);
			tmp7 = _mm_add_ps(tmp7, xmm6);
			tmp8 = _mm_add_ps(tmp8, xmm7);
		}

		//division

		tmp1 = _mm_mul_ps(tmp1, div);
		tmp2 = _mm_mul_ps(tmp2, div);
		tmp3 = _mm_mul_ps(tmp3, div);
		tmp4 = _mm_mul_ps(tmp4, div);
		tmp5 = _mm_mul_ps(tmp5, div);
		tmp6 = _mm_mul_ps(tmp6, div);
		tmp7 = _mm_mul_ps(tmp7, div);
		tmp8 = _mm_mul_ps(tmp8, div);

		tmp1 = _mm_floor_ps(tmp1);
		tmp2 = _mm_floor_ps(tmp2);
		tmp3 = _mm_floor_ps(tmp3);
		tmp4 = _mm_floor_ps(tmp4);
		tmp5 = _mm_floor_ps(tmp5);
		tmp6 = _mm_floor_ps(tmp6);
		tmp7 = _mm_floor_ps(tmp7);
		tmp8 = _mm_floor_ps(tmp8);

		tmpi1 = _mm_cvtps_epi32(tmp1);
		tmpi2 = _mm_cvtps_epi32(tmp2);
		tmpi3 = _mm_cvtps_epi32(tmp3);
		tmpi4 = _mm_cvtps_epi32(tmp4);
		tmpi5 = _mm_cvtps_epi32(tmp5);
		tmpi6 = _mm_cvtps_epi32(tmp6);
		tmpi7 = _mm_cvtps_epi32(tmp7);
		tmpi8 = _mm_cvtps_epi32(tmp8);

		//store

		tmpi1 = _mm_shuffle_epi8(tmpi1, shuf2);
		tmpi2 = _mm_shuffle_epi8(tmpi2, shuf3);
		tmpi3 = _mm_shuffle_epi8(tmpi3, shuf4);
		tmpi4 = _mm_shuffle_epi8(tmpi4, shuf5);
		tmpi5 = _mm_shuffle_epi8(tmpi5, shuf2);
		tmpi6 = _mm_shuffle_epi8(tmpi6, shuf3);
		tmpi7 = _mm_shuffle_epi8(tmpi7, shuf4);
		tmpi8 = _mm_shuffle_epi8(tmpi8, shuf5);

		tmpi1 = _mm_or_si128(tmpi1, tmpi2);
		tmpi3 = _mm_or_si128(tmpi3, tmpi4);
		tmpi5 = _mm_or_si128(tmpi5, tmpi6);
		tmpi7 = _mm_or_si128(tmpi7, tmpi8);

		tmpi1 = _mm_or_si128(tmpi1, tmpi3);
		tmpi5 = _mm_or_si128(tmpi5, tmpi7);

		_mm_stream_si128((__m128i*)(result + i), tmpi1);
		_mm_stream_si128((__m128i*)(result + i + 16), tmpi5);
	}

	//last elements

	for (; i < block_element_count; i++)
	{
		tmp = 0.0f;

		for (size_t j = 0; j < block_count; j++)
		{
			tmp += (float)blocks[j][i] * weights[j];
		}

		result[i] = (uint8_t)(tmp * total_weights);
	}
}

AVX2_TARGET void weightedmean_u8_i32_avx2_shift(uint8_t** blocks, int32_t* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result)
{
	int64_t total_weights = 0;
	for (size_t i = 0; i < block_count; i++)
		total_weights += weights[i];

	uint64_t shift = shift_division(total_weights);

	__m256i weight;
	__m256i tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
	__m256i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
	__m256i shuf1, shuf2, shuf3, shuf4, shuf5;
	__m256i div;
	__m256i perm;

	static int8_t unpack[] = {
			0, -1, -1, -1,
			1, -1, -1, -1,
			2, -1, -1, -1,
			3, -1, -1, -1,
			4, -1, -1, -1,
			5, -1, -1, -1,
			6, -1, -1, -1,
			7, -1, -1, -1
	};
	shuf1 = _mm256_lddqu_si256((__m256i*)unpack);
	static int8_t pack[] = {
		0, 4, 8, 12,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		0, 4, 8, 12,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		-1, -1, -1, -1
	};
	shuf2 = _mm256_lddqu_si256((__m256i*)pack);
	static int8_t pack1[] = {
		-1, -1, -1, -1,
		0, 4, 8, 12,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		0, 4, 8, 12,
		-1, -1, -1, -1,
		-1, -1, -1, -1
	};
	shuf3 = _mm256_lddqu_si256((__m256i*)pack1);
	static int8_t pack2[] = {
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		0, 4, 8, 12,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		0, 4, 8, 12,
		-1, -1, -1, -1
	};
	shuf4 = _mm256_lddqu_si256((__m256i*)pack2);
	static int8_t pack3[] = {
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		0, 4, 8, 12,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		0, 4, 8, 12
	};
	shuf5 = _mm256_lddqu_si256((__m256i*)pack3);
	static int32_t pack4[] = {
		0, 4, 1, 5, 2, 6, 3, 7
	};
	perm = _mm256_lddqu_si256((__m256i*)pack4);

	// build
	tmp1 = _mm256_setzero_si256();
	tmp2 = _mm256_setzero_si256();
	tmp3 = _mm256_setzero_si256();
	tmp4 = _mm256_setzero_si256();
	tmp5 = _mm256_setzero_si256();
	tmp6 = _mm256_setzero_si256();
	tmp7 = _mm256_setzero_si256();
	tmp8 = _mm256_setzero_si256();

	for (size_t i = 0; i < (block_element_count & 0xffffffffffffffc0); i += 64)
	{
		// nul
		tmp1 = _mm256_xor_si256(tmp1, tmp1);
		tmp2 = _mm256_xor_si256(tmp2, tmp2);
		tmp3 = _mm256_xor_si256(tmp3, tmp3);
		tmp4 = _mm256_xor_si256(tmp4, tmp4);
		tmp5 = _mm256_xor_si256(tmp5, tmp5);
		tmp6 = _mm256_xor_si256(tmp6, tmp6);
		tmp7 = _mm256_xor_si256(tmp7, tmp7);
		tmp8 = _mm256_xor_si256(tmp8, tmp8);

		for (size_t j = 0; j < block_count; j++)
		{
			weight = _mm256_set1_epi32(weights[j]);

			xmm0 = _mm256_shuffle_epi8(_mm256_castpd_si256(_mm256_broadcast_sd((double*)(blocks[j] + i))), shuf1);
			xmm1 = _mm256_shuffle_epi8(_mm256_castpd_si256(_mm256_broadcast_sd((double*)(blocks[j] + i + 8))), shuf1);
			xmm2 = _mm256_shuffle_epi8(_mm256_castpd_si256(_mm256_broadcast_sd((double*)(blocks[j] + i + 16))), shuf1);
			xmm3 = _mm256_shuffle_epi8(_mm256_castpd_si256(_mm256_broadcast_sd((double*)(blocks[j] + i + 24))), shuf1);
			xmm4 = _mm256_shuffle_epi8(_mm256_castpd_si256(_mm256_broadcast_sd((double*)(blocks[j] + i + 32))), shuf1);
			xmm5 = _mm256_shuffle_epi8(_mm256_castpd_si256(_mm256_broadcast_sd((double*)(blocks[j] + i + 40))), shuf1);
			xmm6 = _mm256_shuffle_epi8(_mm256_castpd_si256(_mm256_broadcast_sd((double*)(blocks[j] + i + 48))), shuf1);
			xmm7 = _mm256_shuffle_epi8(_mm256_castpd_si256(_mm256_broadcast_sd((double*)(blocks[j] + i + 56))), shuf1);

			xmm0 = _mm256_mullo_epi32(xmm0, weight);
			xmm1 = _mm256_mullo_epi32(xmm1, weight);
			xmm2 = _mm256_mullo_epi32(xmm2, weight);
			xmm3 = _mm256_mullo_epi32(xmm3, weight);
			xmm4 = _mm256_mullo_epi32(xmm4, weight);
			xmm5 = _mm256_mullo_epi32(xmm5, weight);
			xmm6 = _mm256_mullo_epi32(xmm6, weight);
			xmm7 = _mm256_mullo_epi32(xmm7, weight);

			tmp1 = _mm256_add_epi32(tmp1, xmm0);
			tmp2 = _mm256_add_epi32(tmp2, xmm1);
			tmp3 = _mm256_add_epi32(tmp3, xmm2);
			tmp4 = _mm256_add_epi32(tmp4, xmm3);
			tmp5 = _mm256_add_epi32(tmp5, xmm4);
			tmp6 = _mm256_add_epi32(tmp6, xmm5);
			tmp7 = _mm256_add_epi32(tmp7, xmm6);
			tmp8 = _mm256_add_epi32(tmp8, xmm7);
		}

		//division

		tmp1 = _mm256_srli_epi32(tmp1, shift);
		tmp2 = _mm256_srli_epi32(tmp2, shift);
		tmp3 = _mm256_srli_epi32(tmp3, shift);
		tmp4 = _mm256_srli_epi32(tmp4, shift);
		tmp5 = _mm256_srli_epi32(tmp5, shift);
		tmp6 = _mm256_srli_epi32(tmp6, shift);
		tmp7 = _mm256_srli_epi32(tmp7, shift);
		tmp8 = _mm256_srli_epi32(tmp8, shift);

		//store

		tmp1 = _mm256_shuffle_epi8(tmp1, shuf2);
		tmp2 = _mm256_shuffle_epi8(tmp2, shuf3);
		tmp3 = _mm256_shuffle_epi8(tmp3, shuf4);
		tmp4 = _mm256_shuffle_epi8(tmp4, shuf5);
		tmp5 = _mm256_shuffle_epi8(tmp5, shuf2);
		tmp6 = _mm256_shuffle_epi8(tmp6, shuf3);
		tmp7 = _mm256_shuffle_epi8(tmp7, shuf4);
		tmp8 = _mm256_shuffle_epi8(tmp8, shuf5);

		tmp1 = _mm256_or_si256(tmp1, tmp2);
		tmp3 = _mm256_or_si256(tmp3, tmp4);
		tmp5 = _mm256_or_si256(tmp5, tmp6);
		tmp7 = _mm256_or_si256(tmp7, tmp8);

		tmp1 = _mm256_or_si256(tmp1, tmp3);
		tmp5 = _mm256_or_si256(tmp5, tmp7);

		tmp1 = _mm256_permutevar8x32_epi32(tmp1, perm);
		tmp5 = _mm256_permutevar8x32_epi32(tmp5, perm);

		_mm256_storeu_si256((__m256i*)(result + i), tmp1);
		_mm256_storeu_si256((__m256i*)(result + i + 32), tmp5);
	}

	uint64_t tmp;

	//last elements
	
	for (size_t i = block_element_count & 0xffffffffffffffc0; i < block_element_count; i++)
	{
		tmp = 0;

		for (size_t j = 0; j < block_count; j++)
		{
			tmp += blocks[j][i] * weights[j];
		}

		result[i] = (uint8_t)(tmp >> shift);
	}
}

AVX2_TARGET void weightedmean_u8_i32_avx2_nt_shift(uint8_t** blocks, int32_t* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result)
{
	int64_t total_weights = 0;
	for (size_t i = 0; i < block_count; i++)
		total_weights += weights[i];

	uint64_t shift = shift_division(total_weights);

	__m256i weight;
	__m256i tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
	__m256i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
	__m256i shuf1, shuf2, shuf3, shuf4, shuf5;
	__m256i div;
	__m256i perm;

	static int8_t unpack[] = {
		0, -1, -1, -1,
		1, -1, -1, -1,
		2, -1, -1, -1,
		3, -1, -1, -1,
		4, -1, -1, -1,
		5, -1, -1, -1,
		6, -1, -1, -1,
		7, -1, -1, -1
	};
	shuf1 = _mm256_lddqu_si256((__m256i*)unpack);
	static int8_t pack[] = {
		0, 4, 8, 12,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		0, 4, 8, 12,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		-1, -1, -1, -1
	};
	shuf2 = _mm256_lddqu_si256((__m256i*)pack);
	static int8_t pack1[] = {
		-1, -1, -1, -1,
		0, 4, 8, 12,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		0, 4, 8, 12,
		-1, -1, -1, -1,
		-1, -1, -1, -1
	};
	shuf3 = _mm256_lddqu_si256((__m256i*)pack1);
	static int8_t pack2[] = {
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		0, 4, 8, 12,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		0, 4, 8, 12,
		-1, -1, -1, -1
	};
	shuf4 = _mm256_lddqu_si256((__m256i*)pack2);
	static int8_t pack3[] = {
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		0, 4, 8, 12,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		0, 4, 8, 12
	};
	shuf5 = _mm256_lddqu_si256((__m256i*)pack3);
	static int32_t pack4[] = {
		0, 4, 1, 5, 2, 6, 3, 7
	};
	perm = _mm256_lddqu_si256((__m256i*)pack4);

	uint64_t tmp;

	size_t i;

	// align 32

	for (i = 0; i < (32 - ((size_t)result & 31)); i++)
	{
		tmp = 0;

		for (size_t j = 0; j < block_count; j++)
		{
			tmp += blocks[j][i] * weights[j];
		}

		result[i] = (uint8_t)(tmp >> shift);
	}

	// build
	tmp1 = _mm256_setzero_si256();
	tmp2 = _mm256_setzero_si256();
	tmp3 = _mm256_setzero_si256();
	tmp4 = _mm256_setzero_si256();
	tmp5 = _mm256_setzero_si256();
	tmp6 = _mm256_setzero_si256();
	tmp7 = _mm256_setzero_si256();
	tmp8 = _mm256_setzero_si256();

	size_t test = ((block_element_count + 32 - ((size_t)result & 31)) & 0xffffffffffffffc0);

	for (; i < ((block_element_count - 32 + ((size_t)result & 31)) & 0xffffffffffffffc0); i += 64)
	{
		// nul
		tmp1 = _mm256_xor_si256(tmp1, tmp1);
		tmp2 = _mm256_xor_si256(tmp2, tmp2);
		tmp3 = _mm256_xor_si256(tmp3, tmp3);
		tmp4 = _mm256_xor_si256(tmp4, tmp4);
		tmp5 = _mm256_xor_si256(tmp5, tmp5);
		tmp6 = _mm256_xor_si256(tmp6, tmp6);
		tmp7 = _mm256_xor_si256(tmp7, tmp7);
		tmp8 = _mm256_xor_si256(tmp8, tmp8);

		for (size_t j = 0; j < block_count; j++)
		{
			weight = _mm256_set1_epi32(weights[j]);

			xmm0 = _mm256_shuffle_epi8(_mm256_castpd_si256(_mm256_broadcast_sd((double*)(blocks[j] + i))), shuf1);
			xmm1 = _mm256_shuffle_epi8(_mm256_castpd_si256(_mm256_broadcast_sd((double*)(blocks[j] + i + 8))), shuf1);
			xmm2 = _mm256_shuffle_epi8(_mm256_castpd_si256(_mm256_broadcast_sd((double*)(blocks[j] + i + 16))), shuf1);
			xmm3 = _mm256_shuffle_epi8(_mm256_castpd_si256(_mm256_broadcast_sd((double*)(blocks[j] + i + 24))), shuf1);
			xmm4 = _mm256_shuffle_epi8(_mm256_castpd_si256(_mm256_broadcast_sd((double*)(blocks[j] + i + 32))), shuf1);
			xmm5 = _mm256_shuffle_epi8(_mm256_castpd_si256(_mm256_broadcast_sd((double*)(blocks[j] + i + 40))), shuf1);
			xmm6 = _mm256_shuffle_epi8(_mm256_castpd_si256(_mm256_broadcast_sd((double*)(blocks[j] + i + 48))), shuf1);
			xmm7 = _mm256_shuffle_epi8(_mm256_castpd_si256(_mm256_broadcast_sd((double*)(blocks[j] + i + 56))), shuf1);

			xmm0 = _mm256_mullo_epi32(xmm0, weight);
			xmm1 = _mm256_mullo_epi32(xmm1, weight);
			xmm2 = _mm256_mullo_epi32(xmm2, weight);
			xmm3 = _mm256_mullo_epi32(xmm3, weight);
			xmm4 = _mm256_mullo_epi32(xmm4, weight);
			xmm5 = _mm256_mullo_epi32(xmm5, weight);
			xmm6 = _mm256_mullo_epi32(xmm6, weight);
			xmm7 = _mm256_mullo_epi32(xmm7, weight);

			tmp1 = _mm256_add_epi32(tmp1, xmm0);
			tmp2 = _mm256_add_epi32(tmp2, xmm1);
			tmp3 = _mm256_add_epi32(tmp3, xmm2);
			tmp4 = _mm256_add_epi32(tmp4, xmm3);
			tmp5 = _mm256_add_epi32(tmp5, xmm4);
			tmp6 = _mm256_add_epi32(tmp6, xmm5);
			tmp7 = _mm256_add_epi32(tmp7, xmm6);
			tmp8 = _mm256_add_epi32(tmp8, xmm7);
		}

		//division

		tmp1 = _mm256_srli_epi32(tmp1, shift);
		tmp2 = _mm256_srli_epi32(tmp2, shift);
		tmp3 = _mm256_srli_epi32(tmp3, shift);
		tmp4 = _mm256_srli_epi32(tmp4, shift);
		tmp5 = _mm256_srli_epi32(tmp5, shift);
		tmp6 = _mm256_srli_epi32(tmp6, shift);
		tmp7 = _mm256_srli_epi32(tmp7, shift);
		tmp8 = _mm256_srli_epi32(tmp8, shift);

		//store

		tmp1 = _mm256_shuffle_epi8(tmp1, shuf2);
		tmp2 = _mm256_shuffle_epi8(tmp2, shuf3);
		tmp3 = _mm256_shuffle_epi8(tmp3, shuf4);
		tmp4 = _mm256_shuffle_epi8(tmp4, shuf5);
		tmp5 = _mm256_shuffle_epi8(tmp5, shuf2);
		tmp6 = _mm256_shuffle_epi8(tmp6, shuf3);
		tmp7 = _mm256_shuffle_epi8(tmp7, shuf4);
		tmp8 = _mm256_shuffle_epi8(tmp8, shuf5);

		tmp1 = _mm256_or_si256(tmp1, tmp2);
		tmp3 = _mm256_or_si256(tmp3, tmp4);
		tmp5 = _mm256_or_si256(tmp5, tmp6);
		tmp7 = _mm256_or_si256(tmp7, tmp8);

		tmp1 = _mm256_or_si256(tmp1, tmp3);
		tmp5 = _mm256_or_si256(tmp5, tmp7);

		tmp1 = _mm256_permutevar8x32_epi32(tmp1, perm);
		tmp5 = _mm256_permutevar8x32_epi32(tmp5, perm);

		_mm256_stream_si256((__m256i*)(result + i), tmp1);
		_mm256_stream_si256((__m256i*)(result + i + 32), tmp5);
	}

	//last elements

	for (; i < block_element_count; i++)
	{
		tmp = 0;

		for (size_t j = 0; j < block_count; j++)
		{
			tmp += blocks[j][i] * weights[j];
		}

		result[i] = (uint8_t)(tmp >> shift);
	}
}

AVX2_TARGET void weightedmean_u8_f32_avx2(uint8_t** blocks, float* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result)
{
	float total_weights = 0;
	for (size_t i = 0; i < block_count; i++)
		total_weights += weights[i];

	__m256i shuf1, shuf2, shuf3, shuf4, shuf5;
	__m256i perm;
	__m256 div;
	__m256 weight;
	__m256 tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
	__m256i tmpi1, tmpi2, tmpi3, tmpi4, tmpi5, tmpi6, tmpi7, tmpi8;
	__m256 xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;

	total_weights = 1.0f / total_weights;

	div = _mm256_broadcast_ss(&total_weights);

	static int8_t unpack[] = {
		0, -1, -1, -1,
		1, -1, -1, -1,
		2, -1, -1, -1,
		3, -1, -1, -1,
		4, -1, -1, -1,
		5, -1, -1, -1,
		6, -1, -1, -1,
		7, -1, -1, -1
	};
	shuf1 = _mm256_lddqu_si256((__m256i*)unpack);
	static int8_t pack[] = {
		0, 4, 8, 12,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		0, 4, 8, 12,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		-1, -1, -1, -1
	};
	shuf2 = _mm256_lddqu_si256((__m256i*)pack);
	static int8_t pack1[] = {
		-1, -1, -1, -1,
		0, 4, 8, 12,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		0, 4, 8, 12,
		-1, -1, -1, -1,
		-1, -1, -1, -1
	};
	shuf3 = _mm256_lddqu_si256((__m256i*)pack1);
	static int8_t pack2[] = {
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		0, 4, 8, 12,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		0, 4, 8, 12,
		-1, -1, -1, -1
	};
	shuf4 = _mm256_lddqu_si256((__m256i*)pack2);
	static int8_t pack3[] = {
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		0, 4, 8, 12,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		0, 4, 8, 12
	};
	shuf5 = _mm256_lddqu_si256((__m256i*)pack3);
	static int32_t pack4[] = {
		0, 4, 1, 5, 2, 6, 3, 7
	};
	perm = _mm256_lddqu_si256((__m256i*)pack4);

	for (size_t i = 0; i < (block_element_count & 0xffffffffffffffc0); i += 64)
	{
		float nul = 0.0f;
		tmp1 = _mm256_broadcast_ss(&nul);
		tmp2 = _mm256_broadcast_ss(&nul);
		tmp3 = _mm256_broadcast_ss(&nul);
		tmp4 = _mm256_broadcast_ss(&nul);
		tmp5 = _mm256_broadcast_ss(&nul);
		tmp6 = _mm256_broadcast_ss(&nul);
		tmp7 = _mm256_broadcast_ss(&nul);
		tmp8 = _mm256_broadcast_ss(&nul);

		for (size_t j = 0; j < block_count; j++)
		{
			//load weights
			weight = _mm256_broadcast_ss((float*)(weights + j));

			//load data
			xmm0 = _mm256_cvtepi32_ps(_mm256_shuffle_epi8(_mm256_castpd_si256(_mm256_broadcast_sd((double*)(blocks[j] + i))), shuf1));
			xmm1 = _mm256_cvtepi32_ps(_mm256_shuffle_epi8(_mm256_castpd_si256(_mm256_broadcast_sd((double*)(blocks[j] + i + 8))), shuf1));
			xmm2 = _mm256_cvtepi32_ps(_mm256_shuffle_epi8(_mm256_castpd_si256(_mm256_broadcast_sd((double*)(blocks[j] + i + 16))), shuf1));
			xmm3 = _mm256_cvtepi32_ps(_mm256_shuffle_epi8(_mm256_castpd_si256(_mm256_broadcast_sd((double*)(blocks[j] + i + 24))), shuf1));
			xmm4 = _mm256_cvtepi32_ps(_mm256_shuffle_epi8(_mm256_castpd_si256(_mm256_broadcast_sd((double*)(blocks[j] + i + 32))), shuf1));
			xmm5 = _mm256_cvtepi32_ps(_mm256_shuffle_epi8(_mm256_castpd_si256(_mm256_broadcast_sd((double*)(blocks[j] + i + 40))), shuf1));
			xmm6 = _mm256_cvtepi32_ps(_mm256_shuffle_epi8(_mm256_castpd_si256(_mm256_broadcast_sd((double*)(blocks[j] + i + 48))), shuf1));
			xmm7 = _mm256_cvtepi32_ps(_mm256_shuffle_epi8(_mm256_castpd_si256(_mm256_broadcast_sd((double*)(blocks[j] + i + 56))), shuf1));

			xmm0 = _mm256_mul_ps(xmm0, weight);
			xmm1 = _mm256_mul_ps(xmm1, weight);
			xmm2 = _mm256_mul_ps(xmm2, weight);
			xmm3 = _mm256_mul_ps(xmm3, weight);
			xmm4 = _mm256_mul_ps(xmm4, weight);
			xmm5 = _mm256_mul_ps(xmm5, weight);
			xmm6 = _mm256_mul_ps(xmm6, weight);
			xmm7 = _mm256_mul_ps(xmm7, weight);

			tmp1 = _mm256_add_ps(tmp1, xmm0);
			tmp2 = _mm256_add_ps(tmp2, xmm1);
			tmp3 = _mm256_add_ps(tmp3, xmm2);
			tmp4 = _mm256_add_ps(tmp4, xmm3);
			tmp5 = _mm256_add_ps(tmp5, xmm4);
			tmp6 = _mm256_add_ps(tmp6, xmm5);
			tmp7 = _mm256_add_ps(tmp7, xmm6);
			tmp8 = _mm256_add_ps(tmp8, xmm7);
		}

		//division
		tmp1 = _mm256_mul_ps(tmp1, div);
		tmp2 = _mm256_mul_ps(tmp2, div);
		tmp3 = _mm256_mul_ps(tmp3, div);
		tmp4 = _mm256_mul_ps(tmp4, div);
		tmp5 = _mm256_mul_ps(tmp5, div);
		tmp6 = _mm256_mul_ps(tmp6, div);
		tmp7 = _mm256_mul_ps(tmp7, div);
		tmp8 = _mm256_mul_ps(tmp8, div);

		tmp1 = _mm256_floor_ps(tmp1);
		tmp2 = _mm256_floor_ps(tmp2);
		tmp3 = _mm256_floor_ps(tmp3);
		tmp4 = _mm256_floor_ps(tmp4);
		tmp5 = _mm256_floor_ps(tmp5);
		tmp6 = _mm256_floor_ps(tmp6);
		tmp7 = _mm256_floor_ps(tmp7);
		tmp8 = _mm256_floor_ps(tmp8);

		tmpi1 = _mm256_cvtps_epi32(tmp1);
		tmpi2 = _mm256_cvtps_epi32(tmp2);
		tmpi3 = _mm256_cvtps_epi32(tmp3);
		tmpi4 = _mm256_cvtps_epi32(tmp4);
		tmpi5 = _mm256_cvtps_epi32(tmp5);
		tmpi6 = _mm256_cvtps_epi32(tmp6);
		tmpi7 = _mm256_cvtps_epi32(tmp7);
		tmpi8 = _mm256_cvtps_epi32(tmp8);

		//store 
		tmpi1 = _mm256_shuffle_epi8(tmpi1, shuf2);
		tmpi2 = _mm256_shuffle_epi8(tmpi2, shuf3);
		tmpi3 = _mm256_shuffle_epi8(tmpi3, shuf4);
		tmpi4 = _mm256_shuffle_epi8(tmpi4, shuf5);
		tmpi5 = _mm256_shuffle_epi8(tmpi5, shuf2);
		tmpi6 = _mm256_shuffle_epi8(tmpi6, shuf3);
		tmpi7 = _mm256_shuffle_epi8(tmpi7, shuf4);
		tmpi8 = _mm256_shuffle_epi8(tmpi8, shuf5);

		tmpi1 = _mm256_or_si256(tmpi1, tmpi2);
		tmpi3 = _mm256_or_si256(tmpi3, tmpi4);
		tmpi5 = _mm256_or_si256(tmpi5, tmpi6);
		tmpi7 = _mm256_or_si256(tmpi7, tmpi8);

		tmpi1 = _mm256_or_si256(tmpi1, tmpi3);
		tmpi5 = _mm256_or_si256(tmpi5, tmpi7);

		tmpi1 = _mm256_permutevar8x32_epi32(tmpi1, perm);
		tmpi5 = _mm256_permutevar8x32_epi32(tmpi5, perm);

		_mm256_storeu_si256((__m256i*)(result + i), tmpi1);
		_mm256_storeu_si256((__m256i*)(result + i + 32), tmpi5);
	}

	float tmp;

	//last elements
	
	for (size_t i = block_element_count & 0xffffffffffffffc0; i < block_element_count; i++)
	{
		tmp = 0.0f;

		for (size_t j = 0; j < block_count; j++)
		{
			tmp += (float)blocks[j][i] * weights[j];
		}

		result[i] = (uint8_t)(tmp * total_weights);
	}
}

AVX2_TARGET void weightedmean_u8_f32_avx2_nt(uint8_t** blocks, float* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result)
{
	float total_weights = 0;
	for (size_t i = 0; i < block_count; i++)
		total_weights += weights[i];

	__m256i shuf1, shuf2, shuf3, shuf4, shuf5;
	__m256i perm;
	__m256 div;
	__m256 weight;
	__m256 tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
	__m256i tmpi1, tmpi2, tmpi3, tmpi4, tmpi5, tmpi6, tmpi7, tmpi8;
	__m256 xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;

	total_weights = 1.0f / total_weights;

	div = _mm256_broadcast_ss(&total_weights);

	static int8_t unpack[] = {
		0, -1, -1, -1,
		1, -1, -1, -1,
		2, -1, -1, -1,
		3, -1, -1, -1,
		4, -1, -1, -1,
		5, -1, -1, -1,
		6, -1, -1, -1,
		7, -1, -1, -1
	};
	shuf1 = _mm256_lddqu_si256((__m256i*)unpack);
	static int8_t pack[] = {
		0, 4, 8, 12,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		0, 4, 8, 12,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		-1, -1, -1, -1
	};
	shuf2 = _mm256_lddqu_si256((__m256i*)pack);
	static int8_t pack1[] = {
		-1, -1, -1, -1,
		0, 4, 8, 12,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		0, 4, 8, 12,
		-1, -1, -1, -1,
		-1, -1, -1, -1
	};
	shuf3 = _mm256_lddqu_si256((__m256i*)pack1);
	static int8_t pack2[] = {
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		0, 4, 8, 12,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		0, 4, 8, 12,
		-1, -1, -1, -1
	};
	shuf4 = _mm256_lddqu_si256((__m256i*)pack2);
	static int8_t pack3[] = {
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		0, 4, 8, 12,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		0, 4, 8, 12
	};
	shuf5 = _mm256_lddqu_si256((__m256i*)pack3);
	static int32_t pack4[] = {
		0, 4, 1, 5, 2, 6, 3, 7
	};
	perm = _mm256_lddqu_si256((__m256i*)pack4);

	// align 32

	float tmp;

	size_t i;

	for (i = 0; i < (32 - ((size_t)result & 31)); i++)
	{
		tmp = 0.0f;

		for (size_t j = 0; j < block_count; j++)
		{
			tmp += (float)blocks[j][i] * weights[j];
		}

		result[i] = (uint8_t)(tmp * total_weights);
	}

	for (; i < ((block_element_count - (32 - ((size_t)result & 31))) & 0xffffffffffffffc0); i += 64)
	{
		float nul = 0.0f;
		tmp1 = _mm256_broadcast_ss(&nul);
		tmp2 = _mm256_broadcast_ss(&nul);
		tmp3 = _mm256_broadcast_ss(&nul);
		tmp4 = _mm256_broadcast_ss(&nul);
		tmp5 = _mm256_broadcast_ss(&nul);
		tmp6 = _mm256_broadcast_ss(&nul);
		tmp7 = _mm256_broadcast_ss(&nul);
		tmp8 = _mm256_broadcast_ss(&nul);

		for (size_t j = 0; j < block_count; j++)
		{
			//load weights
			weight = _mm256_broadcast_ss((float*)(weights + j));

			//load data
			xmm0 = _mm256_cvtepi32_ps(_mm256_shuffle_epi8(_mm256_castpd_si256(_mm256_broadcast_sd((double*)(blocks[j] + i))), shuf1));
			xmm1 = _mm256_cvtepi32_ps(_mm256_shuffle_epi8(_mm256_castpd_si256(_mm256_broadcast_sd((double*)(blocks[j] + i + 8))), shuf1));
			xmm2 = _mm256_cvtepi32_ps(_mm256_shuffle_epi8(_mm256_castpd_si256(_mm256_broadcast_sd((double*)(blocks[j] + i + 16))), shuf1));
			xmm3 = _mm256_cvtepi32_ps(_mm256_shuffle_epi8(_mm256_castpd_si256(_mm256_broadcast_sd((double*)(blocks[j] + i + 24))), shuf1));
			xmm4 = _mm256_cvtepi32_ps(_mm256_shuffle_epi8(_mm256_castpd_si256(_mm256_broadcast_sd((double*)(blocks[j] + i + 32))), shuf1));
			xmm5 = _mm256_cvtepi32_ps(_mm256_shuffle_epi8(_mm256_castpd_si256(_mm256_broadcast_sd((double*)(blocks[j] + i + 40))), shuf1));
			xmm6 = _mm256_cvtepi32_ps(_mm256_shuffle_epi8(_mm256_castpd_si256(_mm256_broadcast_sd((double*)(blocks[j] + i + 48))), shuf1));
			xmm7 = _mm256_cvtepi32_ps(_mm256_shuffle_epi8(_mm256_castpd_si256(_mm256_broadcast_sd((double*)(blocks[j] + i + 56))), shuf1));

			xmm0 = _mm256_mul_ps(xmm0, weight);
			xmm1 = _mm256_mul_ps(xmm1, weight);
			xmm2 = _mm256_mul_ps(xmm2, weight);
			xmm3 = _mm256_mul_ps(xmm3, weight);
			xmm4 = _mm256_mul_ps(xmm4, weight);
			xmm5 = _mm256_mul_ps(xmm5, weight);
			xmm6 = _mm256_mul_ps(xmm6, weight);
			xmm7 = _mm256_mul_ps(xmm7, weight);

			tmp1 = _mm256_add_ps(tmp1, xmm0);
			tmp2 = _mm256_add_ps(tmp2, xmm1);
			tmp3 = _mm256_add_ps(tmp3, xmm2);
			tmp4 = _mm256_add_ps(tmp4, xmm3);
			tmp5 = _mm256_add_ps(tmp5, xmm4);
			tmp6 = _mm256_add_ps(tmp6, xmm5);
			tmp7 = _mm256_add_ps(tmp7, xmm6);
			tmp8 = _mm256_add_ps(tmp8, xmm7);
		}

		//division
		tmp1 = _mm256_mul_ps(tmp1, div);
		tmp2 = _mm256_mul_ps(tmp2, div);
		tmp3 = _mm256_mul_ps(tmp3, div);
		tmp4 = _mm256_mul_ps(tmp4, div);
		tmp5 = _mm256_mul_ps(tmp5, div);
		tmp6 = _mm256_mul_ps(tmp6, div);
		tmp7 = _mm256_mul_ps(tmp7, div);
		tmp8 = _mm256_mul_ps(tmp8, div);

		tmp1 = _mm256_floor_ps(tmp1);
		tmp2 = _mm256_floor_ps(tmp2);
		tmp3 = _mm256_floor_ps(tmp3);
		tmp4 = _mm256_floor_ps(tmp4);
		tmp5 = _mm256_floor_ps(tmp5);
		tmp6 = _mm256_floor_ps(tmp6);
		tmp7 = _mm256_floor_ps(tmp7);
		tmp8 = _mm256_floor_ps(tmp8);

		tmpi1 = _mm256_cvtps_epi32(tmp1);
		tmpi2 = _mm256_cvtps_epi32(tmp2);
		tmpi3 = _mm256_cvtps_epi32(tmp3);
		tmpi4 = _mm256_cvtps_epi32(tmp4);
		tmpi5 = _mm256_cvtps_epi32(tmp5);
		tmpi6 = _mm256_cvtps_epi32(tmp6);
		tmpi7 = _mm256_cvtps_epi32(tmp7);
		tmpi8 = _mm256_cvtps_epi32(tmp8);

		//store 
		tmpi1 = _mm256_shuffle_epi8(tmpi1, shuf2);
		tmpi2 = _mm256_shuffle_epi8(tmpi2, shuf3);
		tmpi3 = _mm256_shuffle_epi8(tmpi3, shuf4);
		tmpi4 = _mm256_shuffle_epi8(tmpi4, shuf5);
		tmpi5 = _mm256_shuffle_epi8(tmpi5, shuf2);
		tmpi6 = _mm256_shuffle_epi8(tmpi6, shuf3);
		tmpi7 = _mm256_shuffle_epi8(tmpi7, shuf4);
		tmpi8 = _mm256_shuffle_epi8(tmpi8, shuf5);

		tmpi1 = _mm256_or_si256(tmpi1, tmpi2);
		tmpi3 = _mm256_or_si256(tmpi3, tmpi4);
		tmpi5 = _mm256_or_si256(tmpi5, tmpi6);
		tmpi7 = _mm256_or_si256(tmpi7, tmpi8);

		tmpi1 = _mm256_or_si256(tmpi1, tmpi3);
		tmpi5 = _mm256_or_si256(tmpi5, tmpi7);

		tmpi1 = _mm256_permutevar8x32_epi32(tmpi1, perm);
		tmpi5 = _mm256_permutevar8x32_epi32(tmpi5, perm);

		_mm256_stream_si256((__m256i*)(result + i), tmpi1);
		_mm256_stream_si256((__m256i*)(result + i + 32), tmpi5);
	}

	//last elements

	for (; i < block_element_count; i++)
	{
		tmp = 0.0f;

		for (size_t j = 0; j < block_count; j++)
		{
			tmp += (float)blocks[j][i] * weights[j];
		}

		result[i] = (uint8_t)(tmp * total_weights);
	}
}

#endif // x86