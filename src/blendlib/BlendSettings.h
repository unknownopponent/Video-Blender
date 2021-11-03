#pragma once

#include <stdint.h>

#include "RGBFrame.h"

typedef struct BlendSettings
{
	void* blend_funct;

	uint64_t max_blend_threads;

	uint64_t width;
	uint64_t height;
	uint8_t frame_type;

	void* weights;
	uint64_t weights_c;

	uint16_t inum;
	uint16_t iden;
	uint16_t onum;
	uint16_t oden;

	uint8_t internal_size;

} BlendSettings;