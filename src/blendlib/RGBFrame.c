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

uint8_t convert_frame_data(void* result, uint8_t result_type, void* in, uint8_t in_type, uint64_t count)
{
	if (result_type == in_type)
	{
		memcpy(result, in, count);
		return 0;
	}

	switch (in_type)
	{
	case FINT8:
	{
		switch (result_type)
		{
		case FFLOAT32:
		{
			uint8_t* tmp = in;
			fp32* tmp2 = result;
			for (uint64_t i = 0; i < count; i++)
			{
				tmp2[i] = (fp32)tmp[i];
			}
			return 0;
		}
		case FFLOAT64:
		{
			uint8_t* tmp = in;
			fp64* tmp2 = result;
			for (uint64_t i = 0; i < count; i++)
			{
				tmp2[i] = (fp64)tmp[i];
			}
			return 0;
		}
		default:
			return 1;
		}
	}
	case FINT16:
	{
		switch (result_type)
		{
		case FFLOAT32:
		{
			uint16_t* tmp = in;
			fp32* tmp2 = result;
			for (uint64_t i = 0; i < count; i++)
			{
				tmp2[i] = (fp32)tmp[i];
			}
			return 0;
		}
		case FFLOAT64:
		{
			uint16_t* tmp = in;
			fp64* tmp2 = result;
			for (uint64_t i = 0; i < count; i++)
			{
				tmp2[i] = (fp64)tmp[i];
			}
			return 0;
		}
		default:
			return 1;
		}
	}
	case FFLOAT32:
	{
		switch (result_type)
		{
		case FINT8:
		{
			fp32* tmp = in;
			uint8_t* tmp2 = result;
			for (uint64_t i = 0; i < count; i++)
			{
				tmp2[i] = (uint8_t)tmp[i];
			}
			return 0;
		}
		case FINT16:
		{
			fp32* tmp = in;
			uint16_t* tmp2 = result;
			for (uint64_t i = 0; i < count; i++)
			{
				tmp2[i] = (uint16_t)tmp[i];
			}
			return 0;
		}
		default:
			return 1;
		}
	}
	case FFLOAT64:
	{
		switch (result_type)
		{
		case FINT8:
		{
			fp64* tmp = in;
			uint8_t* tmp2 = result;
			for (uint64_t i = 0; i < count; i++)
			{
				tmp2[i] = (uint8_t)tmp[i];
			}
			return 0;
		}
		case FINT16:
		{
			fp64* tmp = in;
			uint16_t* tmp2 = result;
			for (uint64_t i = 0; i < count; i++)
			{
				tmp2[i] = (uint16_t)tmp[i];
			}
			return 0;
		}
		default:
			return 1;
		}
	}
	default:
		return 1;
	}
}