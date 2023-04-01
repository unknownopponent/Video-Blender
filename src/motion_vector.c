
#include <string.h> // memset
#include <stdlib.h> //abs
#include <float.h> //FLT_MAX

#include "motion_vector.h"

#include "sad.h"

#define MOTION_VECTORS_LOOP_RGB8(BLOCK_SIZE, MOTION_SIZE, BLOCK_MOTION_DIFFERENCE) \
size_t left = 0; \
if (x - BLOCK_MOTION_DIFFERENCE > range) \
{ \
	left = x - BLOCK_MOTION_DIFFERENCE - range; \
} \
size_t right = frame_width - BLOCK_SIZE - 1; \
if (x + range + BLOCK_SIZE - BLOCK_MOTION_DIFFERENCE < frame_width) \
{ \
	right = x - BLOCK_MOTION_DIFFERENCE + range; \
} \
size_t top = 0; \
if (y - BLOCK_MOTION_DIFFERENCE > range) \
{ \
	top = y - BLOCK_MOTION_DIFFERENCE - range; \
} \
size_t bottom = frame_height - BLOCK_SIZE - 1; \
if (y + range + BLOCK_SIZE - BLOCK_MOTION_DIFFERENCE < frame_height) \
{ \
	bottom = y - BLOCK_MOTION_DIFFERENCE + range; \
} \
\
for (size_t k = 0; k < BLOCK_SIZE * BLOCK_SIZE; k++) \
{ \
	best_sad[k] = FLT_MAX; \
} \
\
for (size_t k = 0; k < MOTION_SIZE; k++) \
{ \
	memset(vectors + ((y + k) * frame_width + x), 0, sizeof(MotionVector) * MOTION_SIZE); \
} \
\
for (size_t k = top; k < bottom; k++) \
{ \
	for (size_t l = left; l < right; l++) \
	{ \
		size_t group_count = 0; \
		size_t total_cost = 0; \
		\
		for (size_t m = 0; m < BLOCK_SIZE; m++) \
		{ \
			for (size_t n = 0; n < BLOCK_SIZE; n++) \
			{ \
				size_t index = m * BLOCK_SIZE + n; \
				size_t tmp = (y + m - BLOCK_MOTION_DIFFERENCE) * rgb_width + (x + n - BLOCK_MOTION_DIFFERENCE) * 3; \
				size_t tmp2 = (k + m) * rgb_width + (l + n) * 3; \
				current_sad[index] = \
					abs((int16_t)frame1[tmp] - (int16_t)frame2[tmp2]) \
					+ abs((int16_t)frame1[tmp + 1] - (int16_t)frame2[tmp2 + 1]) \
					+ abs((int16_t)frame1[tmp + 2] - (int16_t)frame2[tmp2 + 2]) \
				; \
				\
				if (current_sad[index] <= max_cost) \
				{ \
					group_count += 1; \
					total_cost += current_sad[index]; \
				} \
			} \
		} \
		\
		if (group_count < min_group_pixel_count) \
		{ \
			continue; \
		} \
		\
		MotionVector mv = { .x = (int64_t)l - (int64_t)x + BLOCK_MOTION_DIFFERENCE, .y = (int64_t)k - (int64_t)y + BLOCK_MOTION_DIFFERENCE }; \
		float tmp_sad = (total_cost + 1) / (double)group_count; \
		float tmp_distance = abs(mv.x) * abs(mv.x) + abs(mv.y) * abs(mv.y); \
		\
		for (size_t m = BLOCK_MOTION_DIFFERENCE; m < BLOCK_SIZE - BLOCK_MOTION_DIFFERENCE; m++) \
		{ \
			for (size_t n = BLOCK_MOTION_DIFFERENCE; n < BLOCK_SIZE - BLOCK_MOTION_DIFFERENCE; n++) \
			{ \
				size_t index = m * BLOCK_SIZE + n; \
				if (current_sad[index] > max_cost) \
				{ \
					continue; \
				} \
				\
				size_t vector_index = (y + m - BLOCK_MOTION_DIFFERENCE) * frame_width + (x + n - BLOCK_MOTION_DIFFERENCE); \
				\
				if (best_sad[index] > tmp_sad) \
				{ \
					vectors[vector_index] = mv; \
					best_sad[index] = tmp_sad; \
					continue; \
				} \
				\
				MotionVector tmp = vectors[vector_index]; \
				\
				if (best_sad[index] == tmp_sad \
					&& tmp_distance < abs(tmp.x) * abs(tmp.x) + abs(tmp.y) * abs(tmp.y)) \
				{ \
					vectors[vector_index] = mv; \
					best_sad[index] = tmp_sad; \
				} \
			} \
		} \
	} \
}

#define MOTION_VECTORS_LOOP_RGB8_SSE42_VARIABLES \
static int8_t unpack_bytes[] = { 0, -1, 1, -1, 2, -1, -1, -1, 3, -1, 4, -1, 5, -1, -1, -1 }; \
__m128i unpack = _mm_loadu_si128((__m128i*) & unpack_bytes); \
static int8_t pack_bytes[] = { 0, 1, 8, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 }; \
__m128i pack = _mm_loadu_si128((__m128i*) & pack_bytes); \
uint64_t max_costs_array[] = { max_cost, max_cost }; \
__m128i max_costs = _mm_loadu_si128((__m128i*) & max_costs_array); \
uint32_t max_costs_array2[] = { max_cost, max_cost , max_cost, max_cost }; \
__m128i max_costs2 = _mm_loadu_si128((__m128i*) & max_costs_array2); \
static uint64_t one_mask_array[] = { 1, 1 }; \
__m128i one_mask = _mm_loadu_si128((__m128i*) & one_mask_array); \
static int8_t mvx_shuffle_array[] = { 0, 1, -1, -1, 4, 5, -1, -1, 8, 9, -1, -1, 12, 13, -1, -1 }; \
__m128i mvx_shuffle = _mm_loadu_si128((__m128i*) & mvx_shuffle_array); \
static int8_t mvy_shuffle_array[] = { 2, 3, -1, -1, 6, 7, -1, -1, 10, 11, -1, -1, 14, 15, -1, -1 }; \
__m128i mvy_shuffle = _mm_loadu_si128((__m128i*) & mvy_shuffle_array);

#define MOTION_VECTORS_LOOP_RGB8_SSE42(BLOCK_SIZE, MOTION_SIZE, BLOCK_MOTION_DIFFERENCE) \
size_t left = 0; \
if (x - BLOCK_MOTION_DIFFERENCE > range) \
{ \
	left = x - BLOCK_MOTION_DIFFERENCE - range; \
} \
size_t right = frame_width - BLOCK_SIZE - 1; \
if (x + range + BLOCK_SIZE - BLOCK_MOTION_DIFFERENCE < frame_width) \
{ \
	right = x - BLOCK_MOTION_DIFFERENCE + range; \
} \
size_t top = 0; \
if (y - BLOCK_MOTION_DIFFERENCE > range) \
{ \
	top = y - BLOCK_MOTION_DIFFERENCE - range; \
} \
size_t bottom = frame_height - BLOCK_SIZE - 1; \
if (y + range + BLOCK_SIZE - BLOCK_MOTION_DIFFERENCE < frame_height) \
{ \
	bottom = y - BLOCK_MOTION_DIFFERENCE + range; \
} \
\
for (size_t k = 0; k < BLOCK_SIZE * BLOCK_SIZE; k++) \
{ \
	best_sad[k] = FLT_MAX; \
} \
\
for (size_t k = 0; k < MOTION_SIZE; k++) \
{ \
	memset(vectors + ((y + k) * frame_width + x), 0, sizeof(MotionVector) * MOTION_SIZE); \
} \
\
for (size_t k = top; k < bottom; k++) \
{ \
	for (size_t l = left; l < right; l++) \
	{ \
		size_t group_count = 0; \
		size_t total_cost = 0; \
		\
		__m128i group_counts = _mm_setzero_si128(); \
		__m128i total_costs = _mm_setzero_si128(); \
		\
		for (size_t m = 0; m < BLOCK_SIZE; m++) \
		{ \
			for (size_t n = 0; n < BLOCK_SIZE; n += 2) \
			{ \
				size_t index = m * BLOCK_SIZE + n; \
				size_t tmp = (y + m - BLOCK_MOTION_DIFFERENCE) * rgb_width + (x + n - BLOCK_MOTION_DIFFERENCE) * 3; \
				size_t tmp2 = (k + m) * rgb_width + (l + n) * 3; \
				\
				__m128i xmm0 = _mm_loadu_si128((__m128i*)(frame1 + tmp)); \
				__m128i xmm1 = _mm_loadu_si128((__m128i*)(frame2 + tmp2)); \
				\
				xmm0 = _mm_shuffle_epi8(xmm0, unpack); \
				xmm1 = _mm_shuffle_epi8(xmm1, unpack); \
				\
				__m128i sads = _mm_sad_epu8(xmm0, xmm1); \
				\
				*(int32_t*)(current_sad + index) = _mm_cvtsi128_si32(_mm_shuffle_epi8(sads, pack)); \
				\
				__m128i test = _mm_cmpgt_epi64(max_costs, sads); \
				\
				total_costs = _mm_add_epi64(total_costs, _mm_and_si128(sads, test)); \
				group_counts = _mm_add_epi64(group_counts, _mm_and_si128(test, one_mask)); \
			} \
		} \
		\
		uint64_t tmp_sum[2]; \
		_mm_storeu_si128((__m128i*) & tmp_sum, total_costs); \
		total_cost = tmp_sum[0] + tmp_sum[1]; \
		\
		uint64_t tmp_sum2[2]; \
		_mm_storeu_si128((__m128i*) & tmp_sum2, group_counts); \
		group_count = tmp_sum2[0] + tmp_sum2[1]; \
		\
		if (group_count < min_group_pixel_count) \
		{ \
			continue; \
		} \
		\
		MotionVector mv = { .x = (int64_t)l - (int64_t)x + BLOCK_MOTION_DIFFERENCE, .y = (int64_t)k - (int64_t)y + BLOCK_MOTION_DIFFERENCE }; \
		\
		float tmp_sad = (float)(total_cost / group_count); \
		__m128 tmp_sads = _mm_set_ps1((float)((total_cost + 1) / (double)group_count)); \
		float tmp_distance = abs(mv.x) * abs(mv.x) + abs(mv.y) * abs(mv.y); \
		__m128 tmp_distances = _mm_set_ps1(tmp_distance); \
		MotionVector mvs_array[] = { mv, mv, mv, mv }; \
		__m128 mvs = _mm_loadu_ps((float*)&mvs_array); \
		\
		for (size_t m = BLOCK_MOTION_DIFFERENCE; m < BLOCK_SIZE - BLOCK_MOTION_DIFFERENCE; m++) \
		{ \
			for (size_t n = BLOCK_MOTION_DIFFERENCE; n < BLOCK_SIZE - BLOCK_MOTION_DIFFERENCE; n += 4) \
			{ \
				size_t index = m * BLOCK_SIZE + n; \
				size_t vector_index = (y + m - BLOCK_MOTION_DIFFERENCE) * frame_width + (x + n - BLOCK_MOTION_DIFFERENCE); \
				\
				__m128 tmp_vectors = _mm_loadu_ps((float*)(vectors + vector_index)); \
				\
				__m128i current_sads = _mm_loadl_epi64((__m128i*)(current_sad + index)); \
				__m128i test1 = _mm_cmpgt_epi32(_mm_unpacklo_epi16(current_sads, _mm_setzero_si128()), max_costs2); \
				test1 = _mm_xor_si128(_mm_cmpeq_epi32(test1, test1), test1);/*not*/ \
				\
				__m128 best_sads = _mm_loadu_ps((float*)(best_sad + index)); \
				__m128i test2 = _mm_castps_si128(_mm_cmpgt_ps(best_sads, tmp_sads)); \
				\
				__m128i test3 = _mm_cmpeq_epi32(_mm_castps_si128(best_sads), _mm_castps_si128(tmp_sads)); \
				\
				if (_mm_movemask_epi8(test3)) \
				{ \
					__m128 tmpmvx = _mm_cvtepi32_ps(_mm_shuffle_epi8(_mm_castps_si128(tmp_vectors), mvx_shuffle)); \
					__m128 tmpmvy = _mm_cvtepi32_ps(_mm_shuffle_epi8(_mm_castps_si128(tmp_vectors), mvy_shuffle)); \
					\
					tmpmvx = _mm_mul_ps(tmpmvx, tmpmvx); \
					tmpmvy = _mm_mul_ps(tmpmvy, tmpmvy); \
					\
					__m128 test4 = _mm_cmpgt_ps(_mm_add_ps(tmpmvx, tmpmvy), tmp_distances); \
					\
					test2 = _mm_or_si128(test2, _mm_and_si128(test3, _mm_castps_si128(test4))); \
				} \
				\
				__m128 test5 = _mm_castsi128_ps(_mm_and_si128(test1, test2)); \
				\
				__m128 result = _mm_blendv_ps(tmp_vectors, mvs, test5); \
				__m128 sad_result = _mm_blendv_ps(best_sads, tmp_sads, test5); \
				\
				_mm_storeu_ps((float*)(vectors + vector_index), result); \
				_mm_storeu_ps((float*)(best_sad + index), sad_result); \
			} \
		} \
	} \
}

#define MOTION_VECTORS_LOOP_RGB8_AVX2_VARIABLES \
static int8_t unpack_bytes[] = { 10, -1, 11, -1, 12, -1, -1, -1, 13, -1, 14, -1, 15, -1, -1, -1, 0, -1, 1, -1, 2, -1, -1, -1, 3, -1, 4, -1, 5, -1, -1, -1 }; \
__m256i unpack = _mm256_loadu_si256((__m256i*) & unpack_bytes); \
static int8_t pack_bytes[] = { 0, 1, 8, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 1, 8, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 }; \
__m256i pack = _mm256_loadu_si256((__m256i*) & pack_bytes); \
static int32_t permute_array[] = { 0, 4, -1, -1, -1, -1, -1, -1 }; \
__m256i permute = _mm256_loadu_si256((__m256i*) & permute_array); \
static int32_t permute_array2[] = { 0, 1, -1, -1, 2, 3, -1, -1 }; \
__m256i permute2 = _mm256_loadu_si256((__m256i*) & permute_array2); \
uint64_t max_costs_array[] = { max_cost, max_cost, max_cost, max_cost }; \
__m256i max_costs = _mm256_loadu_si256((__m256i*) & max_costs_array); \
uint32_t max_costs_array2[] = { max_cost, max_cost , max_cost, max_cost, max_cost, max_cost , max_cost, max_cost }; \
__m256i max_costs2 = _mm256_loadu_si256((__m256i*) & max_costs_array2); \
static uint64_t one_mask_array[] = { 1, 1, 1, 1 }; \
__m256i one_mask = _mm256_loadu_si256((__m256i*) & one_mask_array); \
static int8_t mvx_shuffle_array[] = { 0, 1, -1, -1, 4, 5, -1, -1, 8, 9, -1, -1, 12, 13, -1, -1, 0, 1, -1, -1, 4, 5, -1, -1, 8, 9, -1, -1, 12, 13, -1, -1 }; \
__m256i mvx_shuffle = _mm256_loadu_si256((__m256i*) & mvx_shuffle_array); \
static int8_t mvy_shuffle_array[] = { 2, 3, -1, -1, 6, 7, -1, -1, 10, 11, -1, -1, 14, 15, -1, -1, 2, 3, -1, -1, 6, 7, -1, -1, 10, 11, -1, -1, 14, 15, -1, -1 }; \
__m256i mvy_shuffle = _mm256_loadu_si256((__m256i*) & mvy_shuffle_array);

#define MOTION_VECTORS_LOOP_RGB8_AVX2(BLOCK_SIZE, MOTION_SIZE, BLOCK_MOTION_DIFFERENCE) \
size_t left = 0; \
if (x - BLOCK_MOTION_DIFFERENCE > range) \
{ \
	left = x - BLOCK_MOTION_DIFFERENCE - range; \
} \
size_t right = frame_width - BLOCK_SIZE - 1; \
if (x + range + BLOCK_SIZE - BLOCK_MOTION_DIFFERENCE < frame_width) \
{ \
	right = x - BLOCK_MOTION_DIFFERENCE + range; \
} \
size_t top = 0; \
if (y - BLOCK_MOTION_DIFFERENCE > range) \
{ \
	top = y - BLOCK_MOTION_DIFFERENCE - range; \
} \
size_t bottom = frame_height - BLOCK_SIZE - 1; \
if (y + range + BLOCK_SIZE - BLOCK_MOTION_DIFFERENCE < frame_height) \
{ \
	bottom = y - BLOCK_MOTION_DIFFERENCE + range; \
} \
\
for (size_t k = 0; k < BLOCK_SIZE * BLOCK_SIZE; k++) \
{ \
	best_sad[k] = FLT_MAX; \
} \
\
for (size_t k = 0; k < MOTION_SIZE; k++) \
{ \
	memset(vectors + ((y + k) * frame_width + x), 0, sizeof(MotionVector) * MOTION_SIZE); \
} \
\
for (size_t k = top; k < bottom; k++) \
{ \
	for (size_t l = left; l < right; l++) \
	{ \
		size_t group_count = 0; \
		size_t total_cost = 0; \
		\
		__m256i group_counts = _mm256_setzero_si256(); \
		__m256i total_costs = _mm256_setzero_si256(); \
		\
		for (size_t m = 0; m < BLOCK_SIZE; m++) \
		{ \
			for (size_t n = 0; n < BLOCK_SIZE; n += 4) \
			{ \
				size_t index = m * BLOCK_SIZE + n; \
				size_t tmp = (y + m - BLOCK_MOTION_DIFFERENCE) * rgb_width + (x + n - BLOCK_MOTION_DIFFERENCE) * 3; \
				size_t tmp2 = (k + m) * rgb_width + (l + n) * 3; \
				\
				__m256i ymm0 = _mm256_loadu_si256((__m256i*)(frame1 + tmp - 16 + 6)); \
				__m256i ymm1 = _mm256_loadu_si256((__m256i*)(frame2 + tmp2 - 16 + 6)); \
				\
				ymm0 = _mm256_shuffle_epi8(ymm0, unpack); \
				ymm1 = _mm256_shuffle_epi8(ymm1, unpack); \
				\
				__m256i sads = _mm256_sad_epu8(ymm0, ymm1); \
				\
				*(int64_t*)(current_sad + index) = _mm_cvtsi128_si64(_mm256_castsi256_si128(_mm256_permutevar8x32_epi32(_mm256_shuffle_epi8(sads, pack), permute))); \
				\
				__m256i test = _mm256_cmpgt_epi64(max_costs, sads); \
				\
				total_costs = _mm256_add_epi64(total_costs, _mm256_and_si256(sads, test)); \
				group_counts = _mm256_add_epi64(group_counts, _mm256_and_si256(test, one_mask)); \
			} \
		} \
		\
		uint64_t tmp_sum[4]; \
		_mm256_storeu_si256((__m256i*) & tmp_sum, total_costs); \
		total_cost = tmp_sum[0] + tmp_sum[1] + tmp_sum[2] + tmp_sum[3]; \
		\
		uint64_t tmp_sum2[4]; \
		_mm256_storeu_si256((__m256i*) & tmp_sum2, group_counts); \
		group_count = tmp_sum2[0] + tmp_sum2[1] + tmp_sum2[2] + tmp_sum2[3]; \
		\
		if (group_count < min_group_pixel_count) \
		{ \
			continue; \
		} \
		\
		MotionVector mv = { .x = (int64_t)l - (int64_t)x + BLOCK_MOTION_DIFFERENCE, .y = (int64_t)k - (int64_t)y + BLOCK_MOTION_DIFFERENCE }; \
		\
		float tmp_sad = (float)(total_cost / group_count); \
		__m256 tmp_sads = _mm256_set1_ps((float)((total_cost + 1) / (double)group_count)); \
		float tmp_distance = abs(mv.x) * abs(mv.x) + abs(mv.y) * abs(mv.y); \
		__m256 tmp_distances = _mm256_set1_ps(tmp_distance); \
		MotionVector mvs_array[] = { mv, mv, mv, mv, mv, mv, mv, mv }; \
		__m256 mvs = _mm256_loadu_ps((float*)&mvs_array); \
		\
		for (size_t m = BLOCK_MOTION_DIFFERENCE; m < BLOCK_SIZE - BLOCK_MOTION_DIFFERENCE; m++) \
		{ \
			for (size_t n = BLOCK_MOTION_DIFFERENCE; n < BLOCK_SIZE - BLOCK_MOTION_DIFFERENCE; n += 8) \
			{ \
				size_t index = m * BLOCK_SIZE + n; \
				size_t vector_index = (y + m - BLOCK_MOTION_DIFFERENCE) * frame_width + (x + n - BLOCK_MOTION_DIFFERENCE); \
				\
				__m256 tmp_vectors = _mm256_loadu_ps((float*)(vectors + vector_index)); \
				\
				__m256i current_sads = _mm256_castsi128_si256(_mm_loadu_si128((__m128i*)(current_sad + index))); \
				current_sads = _mm256_unpacklo_epi16(_mm256_permutevar8x32_epi32(current_sads, permute2), _mm256_setzero_si256()); \
				__m256i test1 = _mm256_cmpgt_epi32(current_sads, max_costs2); \
				test1 = _mm256_xor_si256(_mm256_cmpeq_epi32(test1, test1), test1);/*not*/  \
				\
				__m256 best_sads = _mm256_loadu_ps((float*)(best_sad + index)); \
				__m256i test2 = _mm256_castps_si256(_mm256_cmp_ps(best_sads, tmp_sads, _CMP_GT_OQ)); \
				\
				__m256i test3 = _mm256_cmpeq_epi32(_mm256_castps_si256(best_sads), _mm256_castps_si256(tmp_sads)); \
				\
				if (_mm256_movemask_epi8(test3)) \
				{ \
					__m256 tmpmvx = _mm256_cvtepi32_ps(_mm256_shuffle_epi8(_mm256_castps_si256(tmp_vectors), mvx_shuffle)); \
					__m256 tmpmvy = _mm256_cvtepi32_ps(_mm256_shuffle_epi8(_mm256_castps_si256(tmp_vectors), mvy_shuffle)); \
					\
					tmpmvx = _mm256_mul_ps(tmpmvx, tmpmvx); \
					tmpmvy = _mm256_mul_ps(tmpmvy, tmpmvy); \
					\
					__m256 test4 = _mm256_cmp_ps(_mm256_add_ps(tmpmvx, tmpmvy), tmp_distances, _CMP_GT_OQ); \
					\
					test2 = _mm256_or_si256(test2, _mm256_and_si256(test3, _mm256_castps_si256(test4))); \
				} \
				\
				__m256 test5 = _mm256_castsi256_ps(_mm256_and_si256(test1, test2)); \
				\
				__m256 result = _mm256_blendv_ps(tmp_vectors, mvs, test5); \
				__m256 sad_result = _mm256_blendv_ps(best_sads, tmp_sads, test5); \
				\
				_mm256_storeu_ps((float*)(vectors + vector_index), result); \
				_mm256_storeu_ps((float*)(best_sad + index), sad_result); \
			} \
		} \
	} \
}

#define MOTION_VECTORS_RGB8(BLOCK_SIZE, RGB_BLOCK_SIZE, MOTION_SIZE, BLOCK_MOTION_DIFFERENCE, SAD_U8_RECTANGLE, MOTION_LOOP, SIMD_VARIABLES, SUFFIX) \
void motion_vectors_rgb8_m##MOTION_SIZE##_##RGB_BLOCK_SIZE##x##BLOCK_SIZE##SUFFIX(uint8_t* frame1, uint8_t* frame2, MotionVector* vectors, size_t frame_width, size_t frame_height, size_t range, size_t max_cost, size_t current_block_max_cost, size_t min_group_pixel_count) \
{ \
	SIMD_VARIABLES \
	\
	uint64_t rgb_width = frame_width * 3; \
	\
	float best_sad[BLOCK_SIZE * BLOCK_SIZE]; \
	uint16_t current_sad[BLOCK_SIZE * BLOCK_SIZE]; \
	\
	size_t column_count = frame_width / MOTION_SIZE + frame_width % MOTION_SIZE; \
	size_t row_count = frame_height / MOTION_SIZE + frame_height % MOTION_SIZE; \
	\
	for (size_t i = 0; i < row_count; i++) \
	{ \
		for (size_t j = 0; j < column_count; j++) \
		{ \
			size_t x = j * MOTION_SIZE; \
			size_t y = i * MOTION_SIZE; \
			\
			size_t testy = y + BLOCK_SIZE - BLOCK_MOTION_DIFFERENCE; \
			size_t testx = x + BLOCK_SIZE - BLOCK_MOTION_DIFFERENCE; \
			\
			if (testy >= frame_height) /*move x,y for bottom and right because frame_width % MOTION_SIZE or frame_height % MOTION_SIZE may be not null*/ \
			{ \
				y -= testy - frame_height - 1; \
			} \
			if (testx >= frame_width) \
			{ \
				x -= testx - frame_width - 1; \
			} \
			\
			size_t pixel_index = y * rgb_width + x * 3; \
			\
			if (SAD_U8_RECTANGLE(frame1 + pixel_index, frame2 + pixel_index, rgb_width, MOTION_SIZE*3, MOTION_SIZE) / (MOTION_SIZE*3 * MOTION_SIZE) <= current_block_max_cost) \
			{ \
				for (size_t k = 0; k < MOTION_SIZE; k++) \
				{ \
					memset(vectors + ((y + k) * frame_width + x), 0, sizeof(MotionVector) * MOTION_SIZE); \
				} \
				continue; \
			} \
			\
			if (i == 0 || testy >= frame_height || j == 0 || testx >= frame_width) /*use MOTION_SIZE blocks for borders*/ \
			{ \
				MOTION_LOOP(MOTION_SIZE, MOTION_SIZE, 0) \
				continue; \
			} \
			\
			/*use BLOCK_SIZE blocks*/ \
			MOTION_LOOP(BLOCK_SIZE, MOTION_SIZE, BLOCK_MOTION_DIFFERENCE) \
		} \
	} \
}

MOTION_VECTORS_RGB8(8, 24, 4, 2, sad_u8_rectangle, MOTION_VECTORS_LOOP_RGB8, , )
MOTION_VECTORS_RGB8(16, 48, 8, 4, sad_u8_rectangle, MOTION_VECTORS_LOOP_RGB8, , )
MOTION_VECTORS_RGB8(32, 96, 16, 8, sad_u8_rectangle, MOTION_VECTORS_LOOP_RGB8, , )
MOTION_VECTORS_RGB8(64, 192, 32, 16, sad_u8_rectangle, MOTION_VECTORS_LOOP_RGB8, , )
MOTION_VECTORS_RGB8(128, 384, 64, 32, sad_u8_rectangle, MOTION_VECTORS_LOOP_RGB8, , )
MOTION_VECTORS_RGB8(256, 768, 128, 64, sad_u8_rectangle, MOTION_VECTORS_LOOP_RGB8, , )

SSE42_TARGET MOTION_VECTORS_RGB8(8, 24, 4, 2, sad_u8_rectangle_sse2, MOTION_VECTORS_LOOP_RGB8_SSE42, MOTION_VECTORS_LOOP_RGB8_SSE42_VARIABLES, _sse42)
SSE42_TARGET MOTION_VECTORS_RGB8(16, 48, 8, 4, sad_u8_rectangle_sse2, MOTION_VECTORS_LOOP_RGB8_SSE42, MOTION_VECTORS_LOOP_RGB8_SSE42_VARIABLES, _sse42)
SSE42_TARGET MOTION_VECTORS_RGB8(32, 96, 16, 8, sad_u8_rectangle_sse2, MOTION_VECTORS_LOOP_RGB8_SSE42, MOTION_VECTORS_LOOP_RGB8_SSE42_VARIABLES, _sse42)
SSE42_TARGET MOTION_VECTORS_RGB8(64, 192, 32, 16, sad_u8_rectangle_sse2, MOTION_VECTORS_LOOP_RGB8_SSE42, MOTION_VECTORS_LOOP_RGB8_SSE42_VARIABLES, _sse42)
SSE42_TARGET MOTION_VECTORS_RGB8(128, 384, 64, 32, sad_u8_rectangle_sse2, MOTION_VECTORS_LOOP_RGB8_SSE42, MOTION_VECTORS_LOOP_RGB8_SSE42_VARIABLES, _sse42)
SSE42_TARGET MOTION_VECTORS_RGB8(256, 768, 128, 64, sad_u8_rectangle_sse2, MOTION_VECTORS_LOOP_RGB8_SSE42, MOTION_VECTORS_LOOP_RGB8_SSE42_VARIABLES, _sse42)

AVX2_TARGET MOTION_VECTORS_RGB8(8, 24, 4, 2, sad_u8_rectangle_avx2, MOTION_VECTORS_LOOP_RGB8_AVX2, MOTION_VECTORS_LOOP_RGB8_AVX2_VARIABLES, _avx2)
AVX2_TARGET MOTION_VECTORS_RGB8(16, 48, 8, 4, sad_u8_rectangle_avx2, MOTION_VECTORS_LOOP_RGB8_AVX2, MOTION_VECTORS_LOOP_RGB8_AVX2_VARIABLES, _avx2)
AVX2_TARGET MOTION_VECTORS_RGB8(32, 96, 16, 8, sad_u8_rectangle_avx2, MOTION_VECTORS_LOOP_RGB8_AVX2, MOTION_VECTORS_LOOP_RGB8_AVX2_VARIABLES, _avx2)
AVX2_TARGET MOTION_VECTORS_RGB8(64, 192, 32, 16, sad_u8_rectangle_avx2, MOTION_VECTORS_LOOP_RGB8_AVX2, MOTION_VECTORS_LOOP_RGB8_AVX2_VARIABLES, _avx2)
AVX2_TARGET MOTION_VECTORS_RGB8(128, 384, 64, 32, sad_u8_rectangle_avx2, MOTION_VECTORS_LOOP_RGB8_AVX2, MOTION_VECTORS_LOOP_RGB8_AVX2_VARIABLES, _avx2)
AVX2_TARGET MOTION_VECTORS_RGB8(256, 768, 128, 64, sad_u8_rectangle_avx2, MOTION_VECTORS_LOOP_RGB8_AVX2, MOTION_VECTORS_LOOP_RGB8_AVX2_VARIABLES, _avx2)
