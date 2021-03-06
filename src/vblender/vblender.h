#pragma once

#ifdef _WIN32 
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#endif

#include "Coding.h"

#include "blendlib/blendlib.h"

typedef struct VBlenderSettings
{
	char* input_file;
	char* output_file;

	char* decoder;

	char* encoder;
	char** encoder_options;
	int encoder_options_count;

	//timebase
	short num;
	short den;

	//32 or 64
	char internal_data_bits;

	char internal_floating;

} VBlenderSettings;

typedef struct VBlenderAddSettings
{
	char exit;
	struct SwsContext* yuv_to_rgb;
	AVFrame* in_frame;
	uint64_t height;
	void* tmp_array[2];
	int* linesize;
	void* converted_rgb_data;
	void (*convert_funct)(void*, void*, uint64_t);
	uint64_t rgb_size;
	BlendContext* blend_ctx;
} VBlenderAddSettings;

typedef struct VBlenderEncodeSettings
{
	char finished_read_input;
	char exit;

	CodingContext* output;

	Queue in_packets;
	Mutex packet_mutex;

	BlendContext* blend_ctx;

	uint64_t rgb_count;
	char output_rgb_type;
	char final_rgb_type;

	struct SwsContext* rgb_to_yuv;
	int* linesize;
	uint64_t height;
	AVFrame* out_frame;
	float pts_step;

	uint64_t* encoded_frames;

} VBlenderEncodeSettings;

char vblend_parse(char** args, int argc, VBlenderSettings* vsettings, BlendSettings* bsettings, char*** input_files, int* input_files_count, char** output_folder);

//parse settings and call vblend_funct
int vblend(char** args, int argsc);

char vblend_funct(VBlenderSettings* vsettings, BlendSettings* bsettings); 

void vblender_add(VBlenderAddSettings* asettings);

void vblender_encode(VBlenderEncodeSettings* esettings);