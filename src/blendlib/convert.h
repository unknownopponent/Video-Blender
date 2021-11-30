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
void* get_convert_function(int input_type, int output_type);

//input_type_output_type

void uint8_float32(uint8_t* in, fp32* out, uint64_t element_count);
void uint8_float64(uint8_t* in, fp64* out, uint64_t element_count);

void uint16_float32(uint16_t* in, fp32* out, uint64_t element_count);
void uint16_float64(uint16_t* in, fp64* out, uint64_t element_count);

void float32_uint8(fp32* in, uint8_t* out, uint64_t element_count);
void float32_uint16(fp32* in, uint16_t* out, uint64_t element_count);

void float64_uint8(fp64* in, uint8_t* out, uint64_t element_count);
void float64_uint16(fp64* in, uint16_t* out, uint64_t element_count);