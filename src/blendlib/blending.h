#pragma once

#include <stdint.h>
typedef uint16_t fp16;
typedef float fp32;
typedef double fp64;

#if defined(__x86_64__) || defined(__x86_64) || defined(__amd64__) || defined(__amd64) || defined(_M_X64) || defined(_M_AMD64)
#define x86_64
#elif defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)
#define x86_32
#else
#error
#endif

/*
types
1 int8
2 int16
3 int32
4 int64

5 uint8
6 uint16
7 uint32
8 uint64

9 fp16
10 fp32
11 fp64
*/
void* get_blend_function(int frame_type, int tmp_type);
#ifdef x86_64
void* get_blend_function_x86(int frame_type, int tmp_type, char* x86_flags);
#endif

//blend functions
//naming : blend_frametype_internaltype(_additional)

//ints

//void blend_uint8_int16(uint8_t* result, uint8_t** in_frames, int16_t* weights, uint64_t nb, uint64_t element_count);
void blend_uint8_int32(uint8_t* result, uint8_t** in_frames, int32_t* weights, uint64_t nb, uint64_t element_count);
void blend_uint8_int64(uint8_t* result, uint8_t** in_frames, int64_t* weights, uint64_t nb, uint64_t element_count);

//void blend_uint16_int16(uint16_t* result, uint16_t** in_frames, int16_t* weights, uint64_t nb, uint64_t element_count);
void blend_uint16_int32(uint16_t* result, uint16_t** in_frames, int32_t* weights, uint64_t nb, uint64_t element_count);
void blend_uint16_int64(uint16_t* result, uint16_t** in_frames, int64_t* weights, uint64_t nb, uint64_t element_count);

//void blend_uint32_int32(uint32_t* result, uint32_t** in_frames, int32_t* weights, uint64_t nb, uint64_t element_count);
//void blend_uint32_int64(uint32_t* result, uint32_t** in_frames, int64_t* weights, uint64_t nb, uint64_t element_count);

//void blend_uint64_int64(uint64_t* result, uint64_t** in_frames, int64_t* weights, uint64_t nb, uint64_t element_count);


//floats

//void blend_float16_float16(fp16* result, fp16** in_frames, fp16* weights, uint64_t nb, uint64_t element_count);

void blend_float32_float32(fp32* result, fp32** in_frames, fp32* weights, uint64_t nb, uint64_t element_count);

void blend_float64_float64(fp64* result, fp64** in_frames, fp64* weights, uint64_t nb, uint64_t element_count);


#ifdef x86_64

//ints x86_64

void blend_uint8_int32_avx2(uint8_t* result, uint8_t** in_frames, int32_t* weights, uint64_t nb, uint64_t element_count);
extern void blend_uint8_int32_avx2_asm(uint8_t* result, uint8_t** in_frames, int32_t* weights, uint64_t nb, uint64_t element_count, int32_t* tmp_frame);


//floats x86_64

void blend_float32_float32_avx(fp32* result, fp32** in_frames, fp32* weights, uint64_t nb, uint64_t element_count);
extern void blend_float32_float32_avx_asm(fp32* result, fp32** in_frames, fp32* weights, uint64_t nb, uint64_t element_count, fp32* tmp_frame);

void blend_float32_float32_fma3avx(fp32* result, fp32** in_frames, fp32* weights, uint64_t nb, uint64_t element_count);
extern void blend_float32_float32_fma3avx_asm(fp32* result, fp32** in_frames, fp32* weights, uint64_t nb, uint64_t element_count, fp32* tmp_frame);

#endif