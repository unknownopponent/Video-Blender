
#include "motion_interpolation.h"

#include <stdlib.h> // abs
#include <math.h> //
#include <string.h>

void motion_blend_rgb_u8_linear(uint8_t** frames, MotionVector** vectors, float* weights, float* durations, size_t count, uint64_t frame_width, uint64_t frame_height, uint8_t* result, float* tmp_buffer, float* tmp_count, int8_t* tmp_vector_destination)
{
	for (size_t i = 0; i < frame_width * frame_height * 3; i++)
	{
		tmp_buffer[i] = 0.0f;
	}
	for (size_t i = 0; i < frame_width * frame_height; i++)
	{
		tmp_count[i] = 0.0f;
	}

	size_t rgb_width = frame_width * 3;

	for (size_t i = 0; i < count -1; i++)
	{
		uint8_t* frame1 = frames[i];
		uint8_t* frame2 = frames[i +1];
		MotionVector* vector = vectors[i];

		memset(tmp_vector_destination, 0, frame_width * frame_height);
		
		//raster motion vectors
		for (size_t j = 0; j < frame_height; j++)
		{
			for (size_t k = 0; k < frame_width; k++)
			{
				MotionVector mv = vector[j * frame_width + k];
				if (mv.x == 0 && mv.y == 0)
				{
					continue;
				}

				int64_t destination_x = (int64_t)k + (int64_t)mv.x;
				int64_t destination_y = (int64_t)j + (int64_t)mv.y;

				//check vector out of bound
				if (destination_x < 0 || destination_x >= frame_width || destination_y < 0 || destination_y >= frame_height)
				{
					continue;
				}

				tmp_vector_destination[destination_y * frame_width + destination_x] = 1;
				
				size_t current_index = j * rgb_width + k * 3;
				size_t destination_index = destination_y * rgb_width + destination_x * 3;

				uint8_t current_red = frame1[current_index];
				uint8_t current_green = frame1[current_index + 1];
				uint8_t current_blue = frame1[current_index + 2];

				uint8_t destination_red = frame2[destination_index];
				uint8_t destination_green = frame2[destination_index + 1];
				uint8_t destination_blue = frame2[destination_index + 2];

				size_t dx = abs(mv.x);
				size_t dy = abs(mv.y);

				float distance = sqrt(dx * dx + dy * dy);
				float step = 1 / distance;

				size_t left;
				size_t right;
				size_t top;
				size_t bottom;

				if (k < destination_x)
				{
					left = k;
					right = destination_x;
				}
				else
				{
					left = destination_x;
					right = k;
				}
				if (j < destination_y)
				{
					top = j;
					bottom = destination_y;
				}
				else
				{
					top = destination_y;
					bottom = j;
				}

				float a = ((float)destination_y - (float)j) / ((float)destination_x - (float)k);
				float b = (float)j - a * (float)k;

				for (size_t l = top; l <= bottom; l++)
				{
					for (size_t m = left; m <= right; m++)
					{
						float nearestx = (2.0f * (float)m + 2.0f * (float)l * a - 2 * a * b) / (2 + 2 * a * a);
						float nearesty = a * nearestx + b;

						float nearest_distance = sqrt(fabs(nearestx - (float)m) + fabs(nearesty - (float)l));

						if (nearest_distance < 1.0f)
						{
							float tmpy = (float)l - (float)j;
							float tmpx = (float)m - (float)k;
							float progression = sqrt(tmpy * tmpy + tmpx * tmpx) / distance;

							//linear color interpolation
							float tmp_red = ((float)current_red * progression + (float)destination_red * (1.0f - progression));
							float tmp_green = ((float)current_green * progression + (float)destination_green * (1.0f - progression));
							float tmp_blue = ((float)current_blue * progression + (float)destination_blue * (1.0f - progression));

							size_t tmp_index = l * rgb_width + m * 3;

							//float intensity = 1.0f - nearest_distance;
							float intensity = 1.0f;

							//linear weight interpolation
							float tmp_weight = weights[i] * (1.0f - progression) + weights[i + 1] * progression;

							float tmp = intensity * step * tmp_weight * durations[i];

							tmp_buffer[tmp_index] += tmp_red * tmp;
							tmp_buffer[tmp_index + 1] += tmp_green * tmp;
							tmp_buffer[tmp_index + 2] += tmp_blue * tmp;
							
							tmp_count[l * frame_width + m] += tmp;
						}
					}
				}
			}
		}

		//add non moving pixels
		for (size_t j = 0; j < frame_height; j++)
		{
			for (size_t k = 0; k < frame_width; k++)
			{
				MotionVector mv = vector[j * frame_width + k];
				if (mv.x != 0 || mv.y != 0 || tmp_vector_destination[j * frame_width + k])
				{
					continue;
				}

				size_t index = j * rgb_width + k * 3;

				float tmp_weight = (weights[i] + weights[i + 1]) / 2.0f * durations[i];

				tmp_buffer[index] += (frame1[index] + frame2[index]) / 2.0f * tmp_weight;
				tmp_buffer[index + 1] += (frame1[index + 1] + frame2[index + 1]) / 2.0f * tmp_weight;
				tmp_buffer[index + 2] += (frame1[index + 2] + frame2[index + 2]) / 2.0f * tmp_weight;
				
				tmp_count[j * frame_width + k] += tmp_weight;
			}
		}
	}

	for (size_t i = 0; i < frame_height * frame_width; i++)
	{
		result[i * 3] = tmp_buffer[i * 3] / tmp_count[i];
		result[i * 3 + 1] = tmp_buffer[i * 3 + 1] / tmp_count[i];
		result[i * 3 + 2] = tmp_buffer[i * 3 + 2] / tmp_count[i];
	}
}