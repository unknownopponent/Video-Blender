
#pragma once

#include <stdint.h>
#include <stddef.h>

void weightedmean_u8_f32(uint8_t** blocks, float* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result);

#include "x86.h"

#if defined(X86_32) || defined(X86_64)

SSE41_TARGET void weightedmean_u8_f32_sse41(uint8_t** blocks, float* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result);

SSE41_TARGET void weightedmean_u8_f32_sse41_nt(uint8_t** blocks, float* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result);

AVX2_TARGET void weightedmean_u8_f32_avx2(uint8_t** blocks, float* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result);

AVX2_TARGET void weightedmean_u8_f32_avx2_nt(uint8_t** blocks, float* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result);

FMA_AVX2_TARGET void weightedmean_u8_f32_fma_avx2(uint8_t** blocks, float* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result);

FMA_AVX2_TARGET void weightedmean_u8_f32_fma_avx2_nt(uint8_t** blocks, float* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result);

#endif