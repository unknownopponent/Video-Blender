#include "Convert.h"

#include <stdlib.h>

int alloc_RGBFrame(RGBFrame** frame, int id, int width, int height)
{
	RGBFrame* tmp = malloc(sizeof(RGBFrame));
	if (!tmp)
	{
		printf("can't alloc rgbframe\n");
		return 1;
	}
	*frame = tmp;
	tmp->data[0] = malloc(sizeof(unsigned char) * width * height * 3);
	if (!malloc)
	{
		printf("failled to alloc frame data\n");
		return 1;
	}
	tmp->data[1] = 0;
	tmp->id = id;
	tmp->linesize[0] = width * 3;
	tmp->linesize[1] = 0;
	return 0;
}

int free_RGBFrame(RGBFrame* frame)
{
	free(frame->data[0]);
	free(frame);
	return 0;
}