
#include "weighted_mean.h"

void weightedmean_u8_f32(uint8_t** blocks, float* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result)
{
	float total_weights = 0.0;
	for (size_t i = 0; i < block_count; i++)
		total_weights += (float)weights[i];

	float tmp;

	for (size_t i = 0; i < block_element_count; i++)
	{
		tmp = 0.0;

		for (size_t j = 0; j < block_count; j++)
		{
			tmp += (float)blocks[j][i] * weights[j];
		}

		result[i] = (uint8_t)(tmp / total_weights);
	}
}

#if defined(X86_32) || defined(X86_64)

SSE41_TARGET void weightedmean_u8_f32_sse41(uint8_t** blocks, float* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result)
{
	float total_weights = 0.0f;
	for (size_t i = 0; i < block_count; i++)
		total_weights += weights[i];

	total_weights = 1.0f / total_weights;

	__m128i shuf1, shuf2;
	__m128 div;
	__m128 weight;
	__m128 xmm0;
	__m128i tmpi;

	div = _mm_set_ps1(total_weights);

	static int8_t unpack[] = { 0, -1, -1, -1, 1, -1, -1, -1, 2, -1, -1, -1, 3, -1, -1, -1 };
	shuf1 = _mm_loadu_si128((__m128i*)unpack);
	static int8_t pack[] = { 0, 4, 8, 12, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
	shuf2 = _mm_loadu_si128((__m128i*)pack);

	for (size_t i = 0; i < (block_element_count & 0xffffffffffffffe0); i += 4)
	{
		// nul
		__m128 tmp = _mm_set_ps1(0.0f);

		for (size_t j = 0; j < block_count; j++)
		{
			//load weights
			weight = _mm_set_ps1(weights[j]);

			//load data
			xmm0 = _mm_cvtepi32_ps(_mm_shuffle_epi8(_mm_cvtsi32_si128(*(int32_t*)(blocks[j] + i)), shuf1));

			xmm0 = _mm_mul_ps(xmm0, weight);

			tmp = _mm_add_ps(tmp, xmm0);
		}

		//division
		tmp = _mm_mul_ps(tmp, div);
		tmp = _mm_round_ps(tmp, _MM_FROUND_FLOOR);
		tmpi = _mm_cvtps_epi32(tmp);

		//store
		tmpi = _mm_shuffle_epi8(tmpi, shuf2);
		*(int32_t*)(result + i) = _mm_cvtsi128_si32(tmpi);
	}

	//last elements

	for (size_t i = block_element_count & 0xffffffffffffffe0; i < block_element_count; i++)
	{
		float tmp = 0.0f;

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

	__m128i shuf1, shuf2;
	__m128 div;
	__m128 weight;
	__m128 xmm0;
	__m128i tmpi;

	div = _mm_set_ps1(total_weights);

	static int8_t unpack[] = { 0, -1, -1, -1, 1, -1, -1, -1, 2, -1, -1, -1, 3, -1, -1, -1 };
	shuf1 = _mm_loadu_si128((__m128i*)unpack);
	static int8_t pack[] = { 0, 4, 8, 12, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
	shuf2 = _mm_loadu_si128((__m128i*)pack);

	for (size_t i = 0; i < (block_element_count & 0xffffffffffffffe0); i += 4)
	{
		// nul
		__m128 tmp = _mm_set_ps1(0.0f);

		for (size_t j = 0; j < block_count; j++)
		{
			//load weights
			weight = _mm_set_ps1(weights[j]);

			//load data
			xmm0 = _mm_cvtepi32_ps(_mm_shuffle_epi8(_mm_cvtsi32_si128(*(int32_t*)(blocks[j] + i)), shuf1));

			xmm0 = _mm_mul_ps(xmm0, weight);

			tmp = _mm_add_ps(tmp, xmm0);
		}

		//division
		tmp = _mm_mul_ps(tmp, div);
		tmp = _mm_round_ps(tmp, _MM_FROUND_FLOOR);
		tmpi = _mm_cvtps_epi32(tmp);

		//store
		tmpi = _mm_shuffle_epi8(tmpi, shuf2);
		_mm_stream_si32(result + i, _mm_cvtsi128_si32(tmpi));
	}

	//last elements

	for (size_t i = block_element_count & 0xffffffffffffffe0; i < block_element_count; i++)
	{
		float tmp = 0.0f;

		for (size_t j = 0; j < block_count; j++)
		{
			tmp += (float)blocks[j][i] * weights[j];
		}

		result[i] = (uint8_t)(tmp * total_weights);
	}
}

AVX2_TARGET void weightedmean_u8_f32_avx2(uint8_t** blocks, float* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result)
{
	float total_weights = 0;
	for (size_t i = 0; i < block_count; i++)
		total_weights += weights[i];

	__m256i shuf1, shuf2;
	__m256i perm;
	__m256 div;
	__m256 weight;
	__m256i tmpi;
	__m256 xmm0;

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
	shuf1 = _mm256_loadu_si256((__m256i*)unpack);
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
	shuf2 = _mm256_loadu_si256((__m256i*)pack);
	static int32_t permute_array[] = {
		0, 4, -1, -1, -1, -1, -1, -1
	};
	perm = _mm256_loadu_si256((__m256i*)permute_array);

	for (size_t i = 0; i < (block_element_count & 0xffffffffffffffc0); i += 8)
	{
		static float nul = 0.0f;
		__m256 tmp = _mm256_broadcast_ss(&nul);

		for (size_t j = 0; j < block_count; j++)
		{
			//load weights
			weight = _mm256_broadcast_ss((float*)(weights + j));

			//load data
			xmm0 = _mm256_cvtepi32_ps(_mm256_shuffle_epi8(_mm256_castsi128_si256(_mm_loadl_epi64((__m128i*)(blocks[j] + i))), shuf1));
			xmm0 = _mm256_mul_ps(xmm0, weight);
			tmp = _mm256_add_ps(tmp, xmm0);
		}

		//division
		tmp = _mm256_mul_ps(tmp, div);
		tmp = _mm256_round_ps(tmp, _MM_FROUND_FLOOR);
		tmpi = _mm256_cvtps_epi32(tmp);

		//store
		tmpi = _mm256_shuffle_epi8(tmpi, shuf2);
		tmpi = _mm256_permutevar8x32_epi32(tmpi, perm);
		*(int64_t*)(result + i) =  _mm_cvtsi128_si64(_mm256_castsi256_si128(tmpi));
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

	__m256i shuf1, shuf2;
	__m256i perm;
	__m256 div;
	__m256 weight;
	__m256i tmpi;
	__m256 xmm0;

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
	shuf1 = _mm256_loadu_si256((__m256i*)unpack);
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
	shuf2 = _mm256_loadu_si256((__m256i*)pack);
	static int32_t permute_array[] = {
		0, 4, -1, -1, -1, -1, -1, -1
	};
	perm = _mm256_loadu_si256((__m256i*)permute_array);

	for (size_t i = 0; i < (block_element_count & 0xffffffffffffffc0); i += 8)
	{
		static float nul = 0.0f;
		__m256 tmp = _mm256_broadcast_ss(&nul);

		for (size_t j = 0; j < block_count; j++)
		{
			//load weights
			weight = _mm256_broadcast_ss((float*)(weights + j));

			//load data
			xmm0 = _mm256_cvtepi32_ps(_mm256_shuffle_epi8(_mm256_castsi128_si256(_mm_loadl_epi64((__m128i*)(blocks[j] + i))), shuf1));
			xmm0 = _mm256_mul_ps(xmm0, weight);
			tmp = _mm256_add_ps(tmp, xmm0);
		}

		//division
		tmp = _mm256_mul_ps(tmp, div);
		tmp = _mm256_round_ps(tmp, _MM_FROUND_FLOOR);
		tmpi = _mm256_cvtps_epi32(tmp);

		//store
		tmpi = _mm256_shuffle_epi8(tmpi, shuf2);
		tmpi = _mm256_permutevar8x32_epi32(tmpi, perm);
		_mm_stream_si64x(result + i, _mm_cvtsi128_si64(_mm256_castsi256_si128(tmpi)));
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

FMA_AVX2_TARGET void weightedmean_u8_f32_fma_avx2(uint8_t** blocks, float* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result)
{
	float total_weights = 0;
	for (size_t i = 0; i < block_count; i++)
		total_weights += weights[i];

	__m256i shuf1, shuf2;
	__m256i perm;
	__m256 div;
	__m256 weight;
	__m256i tmpi;
	__m256 xmm0;

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
	shuf1 = _mm256_loadu_si256((__m256i*)unpack);
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
	shuf2 = _mm256_loadu_si256((__m256i*)pack);
	static int32_t permute_array[] = {
		0, 4, -1, -1, -1, -1, -1, -1
	};
	perm = _mm256_loadu_si256((__m256i*)permute_array);

	for (size_t i = 0; i < (block_element_count & 0xffffffffffffffc0); i += 8)
	{
		static float nul = 0.0f;
		__m256 tmp = _mm256_broadcast_ss(&nul);

		for (size_t j = 0; j < block_count; j++)
		{
			//load weights
			weight = _mm256_broadcast_ss((float*)(weights + j));

			//load data
			xmm0 = _mm256_cvtepi32_ps(_mm256_shuffle_epi8(_mm256_castsi128_si256(_mm_loadl_epi64((__m128i*)(blocks[j] + i))), shuf1));
			tmp = _mm256_fmadd_ps(xmm0, weight, tmp);
		}

		//division
		tmp = _mm256_mul_ps(tmp, div);
		tmp = _mm256_round_ps(tmp, _MM_FROUND_FLOOR);
		tmpi = _mm256_cvtps_epi32(tmp);

		//store
		tmpi = _mm256_shuffle_epi8(tmpi, shuf2);
		tmpi = _mm256_permutevar8x32_epi32(tmpi, perm);
		*(int64_t*)(result + i) = _mm_cvtsi128_si64(_mm256_castsi256_si128(tmpi));
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

FMA_AVX2_TARGET void weightedmean_u8_f32_fma_avx2_nt(uint8_t** blocks, float* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result)
{
	float total_weights = 0;
	for (size_t i = 0; i < block_count; i++)
		total_weights += weights[i];

	__m256i shuf1, shuf2;
	__m256i perm;
	__m256 div;
	__m256 weight;
	__m256i tmpi;
	__m256 xmm0;

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
	shuf1 = _mm256_loadu_si256((__m256i*)unpack);
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
	shuf2 = _mm256_loadu_si256((__m256i*)pack);
	static int32_t permute_array[] = {
		0, 4, -1, -1, -1, -1, -1, -1
	};
	perm = _mm256_loadu_si256((__m256i*)permute_array);

	for (size_t i = 0; i < (block_element_count & 0xffffffffffffffc0); i += 8)
	{
		static float nul = 0.0f;
		__m256 tmp = _mm256_broadcast_ss(&nul);

		for (size_t j = 0; j < block_count; j++)
		{
			//load weights
			weight = _mm256_broadcast_ss((float*)(weights + j));

			//load data
			xmm0 = _mm256_cvtepi32_ps(_mm256_shuffle_epi8(_mm256_castsi128_si256(_mm_loadl_epi64((__m128i*)(blocks[j] + i))), shuf1));
			tmp = _mm256_fmadd_ps(xmm0, weight, tmp);
		}

		//division
		tmp = _mm256_mul_ps(tmp, div);
		tmp = _mm256_round_ps(tmp, _MM_FROUND_FLOOR);
		tmpi = _mm256_cvtps_epi32(tmp);

		//store
		tmpi = _mm256_shuffle_epi8(tmpi, shuf2);
		tmpi = _mm256_permutevar8x32_epi32(tmpi, perm);
		_mm_stream_si64x(result + i, _mm_cvtsi128_si64(_mm256_castsi256_si128(tmpi)));
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

#endif // x86