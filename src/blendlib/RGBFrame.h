#pragma once

#include "utils/Types.h"

typedef enum RGBFrameType
{
	FINT8 = 0,
	FINT16 = 1,
	FINT32 = 2,
	FINT64 = 3,
	FFLOAT32 = 4,
	FFLOAT64 = 5,
	FTYPES_COUNT
} RGBFrameType;

static uint8_t type_sizes[FTYPES_COUNT] = {
	1,
	2,
	4,
	8,
	4,
	8
};

typedef struct RGBFrame
{
	uint64_t id;
	void* frame_data;
} RGBFrame;

int alloc_RGBFrame(RGBFrame** frame, uint64_t id, uint64_t width, uint64_t height, uint64_t data_size);

void free_RGBFrame(RGBFrame* frame);

uint8_t convert_frame_data(void* result, uint8_t result_type, void* in, uint8_t in_type, uint64_t count);
