#include "RGBFrame.h"

#include <string.h>

#include "../utils/Error.h"

int alloc_RGBFrame(RGBFrame** frame, uint64_t id, uint64_t width, uint64_t height, uint64_t data_size)
{

	RGBFrame* tmp_frame = malloc(sizeof(RGBFrame));
	if (!tmp_frame)
	{
		oom();
	}

	tmp_frame->id = id;
	
	tmp_frame->frame_data = malloc(width * height * 3 * data_size);
	if (!tmp_frame->frame_data)
	{
		oom();
	}
	
	*frame = tmp_frame;

	return 0;
}

void free_RGBFrame(RGBFrame* frame)
{
	if (frame->frame_data)
		free(frame->frame_data);
	free(frame);
}