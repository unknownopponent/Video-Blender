#include "convert.h"

void* get_convert_function(int input_type, int output_type)
{
	if (input_type == 5)
	{
		if (output_type == 10)
			return uint8_float32;
		if (output_type == 11)
			return uint8_float64;
	}
	if (input_type == 6)
	{
		if (output_type == 10)
			return uint16_float32;
		if (output_type == 11)
			return uint16_float64;
	}
	if (input_type == 10)
	{
		if (output_type == 5)
			return float32_uint8;
		if (output_type == 6)
			return float32_uint16;
	}
	if (input_type == 11)
	{
		if (output_type == 5)
			return float64_uint8;
		if (output_type == 6)
			return float64_uint16;
	}

	return 0;
}

void uint8_float32(uint8_t* in, fp32* out, uint64_t element_count)
{
	for (uint64_t i = 0; i < element_count; i++)
	{
		out[i] = (fp32)in[i];
	}
}

void uint8_float64(uint8_t* in, fp64* out, uint64_t element_count)
{
	for (uint64_t i = 0; i < element_count; i++)
	{
		out[i] = (fp64)in[i];
	}
}

void uint16_float32(uint16_t* in, fp32* out, uint64_t element_count)
{
	for (uint64_t i = 0; i < element_count; i++)
	{
		out[i] = (fp32)in[i];
	}
}

void uint16_float64(uint16_t* in, fp64* out, uint64_t element_count)
{
	for (uint64_t i = 0; i < element_count; i++)
	{
		out[i] = (fp64)in[i];
	}
}

void float32_uint8(fp32* in, uint8_t* out, uint64_t element_count)
{
	for (uint64_t i = 0; i < element_count; i++)
	{
		out[i] = (uint8_t)in[i];
	}
}

void float32_uint16(fp32* in, uint16_t* out, uint64_t element_count)
{
	for (uint64_t i = 0; i < element_count; i++)
	{
		out[i] = (uint16_t)in[i];
	}
}

void float64_uint8(fp64* in, uint8_t* out, uint64_t element_count)
{
	for (uint64_t i = 0; i < element_count; i++)
	{
		out[i] = (uint8_t)in[i];
	}
}

void float64_uint16(fp64* in, uint16_t* out, uint64_t element_count)
{
	for (uint64_t i = 0; i < element_count; i++)
	{
		out[i] = (uint16_t)in[i];
	}
}