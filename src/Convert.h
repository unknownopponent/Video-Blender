#pragma once

#include <libswscale/swscale.h>

typedef struct RGBFrame
{
	int id;
	unsigned char* data[8];
	int linesize[8];

}RGBFrame;

int alloc_RGBFrame(RGBFrame** frame, int id, int width, int height);
int free_RGBFrame(RGBFrame* frame);

