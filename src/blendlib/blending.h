#pragma once

#include "utils/Types.h"

//blend functions
//naming : blend_frametype_internaltype


//ints

//void blend_int8_int8(uint8_t* result, uint8_t** in_frames, int8_t* weights, uint64_t nb, uint64_t element_count); //makes no sense because 8bit is the minimum standard
//void blend_int8_int16(uint8_t* result, uint8_t** in_frames, int16_t* weights, uint64_t nb, uint64_t element_count);
void blend_int8_int32(uint8_t* result, uint8_t** in_frames, int32_t* weights, uint64_t nb, uint64_t element_count);
void blend_int8_int64(uint8_t* result, uint8_t** in_frames, int64_t* weights, uint64_t nb, uint64_t element_count);

//void blend_int16_int16(uint16_t* result, uint16_t** in_frames, int16_t* weights, uint64_t nb, uint64_t element_count);
void blend_int16_int32(uint16_t* result, uint16_t** in_frames, int32_t* weights, uint64_t nb, uint64_t element_count);
void blend_int16_int64(uint16_t* result, uint16_t** in_frames, int64_t* weights, uint64_t nb, uint64_t element_count);

//void blend_int32_int32(uint32_t* result, uint32_t** in_frames, int32_t* weights, uint64_t nb, uint64_t element_count);
//void blend_int32_int64(uint32_t* result, uint32_t** in_frames, int64_t* weights, uint64_t nb, uint64_t element_count);

//void blend_int64_int64(uint64_t* result, uint64_t** in_frames, int64_t* weights, uint64_t nb, uint64_t element_count);


//floats

//void blend_float16_float16(fp16* result, fp16** in_frames, fp16* weights, uint64_t nb, uint64_t element_count);
//void blend_float16_float32(fp16* result, fp16** in_frames, fp32* weights, uint64_t nb, uint64_t element_count);
//void blend_float16_float64(fp16* result, fp16** in_frames, fp64* weights, uint64_t nb, uint64_t element_count);

void blend_float32_float32(fp32* result, fp32** in_frames, fp32* weights, uint64_t nb, uint64_t element_count);
//void blend_float32_float64(fp32* result, fp32** in_frames, fp64* weights, uint64_t nb, uint64_t element_count);

void blend_float64_float64(fp64* result, fp64** in_frames, fp64* weights, uint64_t nb, uint64_t element_count);
