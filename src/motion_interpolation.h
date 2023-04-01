#pragma once

#include <stdint.h>

#include "motion_vector.h"

void motion_blend_rgb_u8_linear(uint8_t** frames, MotionVector** vectors, float* weights, float* durations, size_t count, uint64_t frame_width, uint64_t frame_height, uint8_t* result, float* tmp_buffer, float* tmp_count, int8_t* tmp_vector_destination);
