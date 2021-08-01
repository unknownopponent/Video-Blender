#pragma once

#include "libavcodec/codec_id.h"


typedef struct Settings
{
	char* input_file;
	char* output_file;

	char* hardware_decoding;
	char* hardware_encoding;

	int num;
	int den;

	int w_count;
	int* weights;

	int threads;

	char opengl;

	enum AVCodecID decoder;
	char* preset;
	char* crf;
	enum AVPixelFormat pix_fmt;

} Settings;

int set_default_values(Settings* settings);
int parse(Settings* settings, int argc, char** args);

void print_usage();