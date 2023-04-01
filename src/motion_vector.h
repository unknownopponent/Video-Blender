
#pragma once

#include <stdint.h>

#include "x86.h"

typedef struct MotionVector 
{
	int16_t x;
	int16_t y;
} MotionVector;

void motion_vectors_rgb8_m4_24x8(uint8_t* frame1, uint8_t* frame2, MotionVector* vectors, size_t frame_width, size_t frame_height, size_t range, size_t max_cost, size_t current_block_max_cost, size_t min_group_pixel_count);
void motion_vectors_rgb8_m8_48x16(uint8_t* frame1, uint8_t* frame2, MotionVector* vectors, size_t frame_width, size_t frame_height, size_t range, size_t max_cost, size_t current_block_max_cost, size_t min_group_pixel_count);
void motion_vectors_rgb8_m16_96x32(uint8_t* frame1, uint8_t* frame2, MotionVector* vectors, size_t frame_width, size_t frame_height, size_t range, size_t max_cost, size_t current_block_max_cost, size_t min_group_pixel_count);
void motion_vectors_rgb8_m32_192x64(uint8_t* frame1, uint8_t* frame2, MotionVector* vectors, size_t frame_width, size_t frame_height, size_t range, size_t max_cost, size_t current_block_max_cost, size_t min_group_pixel_count);
void motion_vectors_rgb8_m64_384x128(uint8_t* frame1, uint8_t* frame2, MotionVector* vectors, size_t frame_width, size_t frame_height, size_t range, size_t max_cost, size_t current_block_max_cost, size_t min_group_pixel_count);
void motion_vectors_rgb8_m128_768x256(uint8_t* frame1, uint8_t* frame2, MotionVector* vectors, size_t frame_width, size_t frame_height, size_t range, size_t max_cost, size_t current_block_max_cost, size_t min_group_pixel_count);

SSE42_TARGET void motion_vectors_rgb8_m4_24x8_sse42(uint8_t* frame1, uint8_t* frame2, MotionVector* vectors, size_t frame_width, size_t frame_height, size_t range, size_t max_cost, size_t current_block_max_cost, size_t min_group_pixel_count);
SSE42_TARGET void motion_vectors_rgb8_m8_48x16_sse42(uint8_t* frame1, uint8_t* frame2, MotionVector* vectors, size_t frame_width, size_t frame_height, size_t range, size_t max_cost, size_t current_block_max_cost, size_t min_group_pixel_count);
SSE42_TARGET void motion_vectors_rgb8_m16_96x32_sse42(uint8_t* frame1, uint8_t* frame2, MotionVector* vectors, size_t frame_width, size_t frame_height, size_t range, size_t max_cost, size_t current_block_max_cost, size_t min_group_pixel_count);
SSE42_TARGET void motion_vectors_rgb8_m32_192x64_sse42(uint8_t* frame1, uint8_t* frame2, MotionVector* vectors, size_t frame_width, size_t frame_height, size_t range, size_t max_cost, size_t current_block_max_cost, size_t min_group_pixel_count);
SSE42_TARGET void motion_vectors_rgb8_m64_384x128_sse42(uint8_t* frame1, uint8_t* frame2, MotionVector* vectors, size_t frame_width, size_t frame_height, size_t range, size_t max_cost, size_t current_block_max_cost, size_t min_group_pixel_count);
SSE42_TARGET void motion_vectors_rgb8_m128_768x256_sse42(uint8_t* frame1, uint8_t* frame2, MotionVector* vectors, size_t frame_width, size_t frame_height, size_t range, size_t max_cost, size_t current_block_max_cost, size_t min_group_pixel_count);

AVX2_TARGET void motion_vectors_rgb8_m4_24x8_avx2(uint8_t* frame1, uint8_t* frame2, MotionVector* vectors, size_t frame_width, size_t frame_height, size_t range, size_t max_cost, size_t current_block_max_cost, size_t min_group_pixel_count);
AVX2_TARGET void motion_vectors_rgb8_m8_48x16_avx2(uint8_t* frame1, uint8_t* frame2, MotionVector* vectors, size_t frame_width, size_t frame_height, size_t range, size_t max_cost, size_t current_block_max_cost, size_t min_group_pixel_count);
AVX2_TARGET void motion_vectors_rgb8_m16_96x32_avx2(uint8_t* frame1, uint8_t* frame2, MotionVector* vectors, size_t frame_width, size_t frame_height, size_t range, size_t max_cost, size_t current_block_max_cost, size_t min_group_pixel_count);
AVX2_TARGET void motion_vectors_rgb8_m32_192x64_avx2(uint8_t* frame1, uint8_t* frame2, MotionVector* vectors, size_t frame_width, size_t frame_height, size_t range, size_t max_cost, size_t current_block_max_cost, size_t min_group_pixel_count);
AVX2_TARGET void motion_vectors_rgb8_m64_384x128_avx2(uint8_t* frame1, uint8_t* frame2, MotionVector* vectors, size_t frame_width, size_t frame_height, size_t range, size_t max_cost, size_t current_block_max_cost, size_t min_group_pixel_count);
AVX2_TARGET void motion_vectors_rgb8_m128_768x256_avx2(uint8_t* frame1, uint8_t* frame2, MotionVector* vectors, size_t frame_width, size_t frame_height, size_t range, size_t max_cost, size_t current_block_max_cost, size_t min_group_pixel_count);
