
#pragma once

#include <stdint.h>
#include <stddef.h>

void weightedmean_u8_i64(uint8_t** blocks, int64_t* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result);

void weightedmean_u8_f64(uint8_t** blocks, double* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result);


#include "../x86.h"

#if defined(X86_32) || defined(X86_64)

void weightedmean_u8_i64_x86(uint8_t** blocks, int64_t* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result);

void weightedmean_u8_f64_x86(uint8_t** blocks, double* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result);

SSE2_TARGET void weightedmean_u8_i64_sse2_nt(uint8_t** blocks, int64_t* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result);

SSE2_TARGET void weightedmean_u8_f64_sse2_nt(uint8_t** blocks, double* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result);

SSE41_TARGET void weightedmean_u8_i32_sse41_shift(uint8_t** blocks, int32_t* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result);

SSE41_TARGET void weightedmean_u8_i32_sse41_nt_shift(uint8_t** blocks, int32_t* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result);

SSE41_TARGET void weightedmean_u8_f32_sse41(uint8_t** blocks, float* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result);

SSE41_TARGET void weightedmean_u8_f32_sse41_nt(uint8_t** blocks, float* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result);

FMA_TARGET void weightedmean_u8_f32_fma128(uint8_t** blocks, float* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result);

FMA_TARGET void weightedmean_u8_f32_fma128_nt(uint8_t** blocks, float* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result);

AVX2_TARGET void weightedmean_u8_i32_avx2_shift(uint8_t** blocks, int32_t* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result);

AVX2_TARGET void weightedmean_u8_i32_avx2_nt_shift(uint8_t** blocks, int32_t* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result);

AVX2_TARGET void weightedmean_u8_f32_avx2(uint8_t** blocks, float* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result);

AVX2_TARGET void weightedmean_u8_f32_avx2_nt(uint8_t** blocks, float* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result);

FMA_AVX2_TARGET void weightedmean_u8_f32_fma_avx2(uint8_t** blocks, float* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result);

FMA_AVX2_TARGET void weightedmean_u8_f32_fma_avx2_nt(uint8_t** blocks, float* weights, uint64_t block_count, uint64_t block_element_count, uint8_t* result);

#endif