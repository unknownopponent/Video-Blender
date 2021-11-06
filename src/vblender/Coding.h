#pragma once

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

typedef struct CodingContext
{
	AVFormatContext* format_ctx;
	AVCodecContext* codec_ctx;
	
	unsigned int video_stream_index;
}CodingContext;

char open_input(CodingContext* ctx, const char* file, const char* decoder_name);
char open_decoder(AVCodecContext** ctx, AVCodec* codec, AVCodecParameters* param);
char decode_frame(CodingContext* ctx, AVFrame* frame, AVPacket* packet);

char open_output(AVFormatContext* input, CodingContext* output, const char* file);
char open_encoder(CodingContext* ctx, char* encoder_name, AVDictionary** codec_options, AVRational timebase, AVCodecParameters* input_params);

char encode(AVCodecContext* ctx, AVFrame* frame, AVPacket* packet);
char write_packet(AVFormatContext* ctx, AVPacket* packet);

char close_output_file(AVFormatContext* format);

void close_coding_context(CodingContext* ctx);
