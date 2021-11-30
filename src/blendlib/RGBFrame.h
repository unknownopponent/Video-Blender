#pragma once

#include "utils/Types.h"

typedef enum RGBFrameType
{
	FUINT8 = 5,
	FUINT16 = 6,
	FUINT32 = 7,
	FUINT64 = 8,
	FFLOAT32 = 10,
	FFLOAT64 = 11,
	FTYPES_COUNT= 12
} RGBFrameType;

static uint8_t type_sizes[FTYPES_COUNT] = {
	0,
	1,
	2,
	4,
	8,
	1,
	2,
	4,
	8,
	2,
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