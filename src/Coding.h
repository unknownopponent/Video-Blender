#pragma once

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#include "Settings.h"

typedef struct CodingContext
{
	AVFormatContext* format_ctx;
	int stream_i;
	AVCodecContext* codec_ctx;
	AVPacket* packet;
}CodingContext;

int open_input(CodingContext* in_ctx, Settings* settings);
int open_decoder(AVCodecContext** enc_ctx, AVCodec* codec, AVCodecParameters* param);

int contains(enum AVPixelFormat target, enum AVPixelFormat* src);

int read(CodingContext* in_ctx, AVFrame* frame);

enum AVPixelFormat get_lossless_format(int bit_per_channel);

int open_output(CodingContext* in_ctx, CodingContext* out_ctx, Settings* settings);
int open_encoder(CodingContext* out_ctx, AVCodecContext* dec_ctx, AVCodec* codec, enum AVPixelFormat format, int num, int den);

int encode(CodingContext* out_ctx, AVFrame* frame);

int close_output_file(AVFormatContext* format);

int close_coding_context(CodingContext* ctx);