#include "Coding.h"

#include "utils/Error.h"

char open_input(CodingContext* ctx, char* file, const char* decoder_name)
{
    if (avformat_open_input(&ctx->format_ctx, file, NULL, NULL) < 0)
    {
        fprintf(stderr, "failled to format open input\n");
        return 1;
    }
    if (avformat_find_stream_info(ctx->format_ctx, NULL) < 0)
    {
        fprintf(stderr, "failled to find stream infos\n");
        return 1;
    }

    char flag = 0;

    for (int i = 0; i < ctx->format_ctx->nb_streams; i++)
    {
        if (ctx->format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            ctx->video_stream_index = i;
            flag = 1;
            break;
        }
    }

    if (!flag)
    {
        fprintf(stderr, "failled to find video stream\n");
        return 1;
    }

    AVCodec* decoder = 0;

    if (decoder_name)
    {
        decoder = avcodec_find_decoder_by_name(decoder_name);
    }
    else
    {
        decoder = avcodec_find_decoder(ctx->format_ctx->streams[ctx->video_stream_index]->codecpar->codec_id);
    }

    if (!decoder)
    {
        fprintf(stderr, "failled to find decoder\n");
        return 1;
    }

    if (open_decoder(&ctx->codec_ctx, decoder, ctx->format_ctx->streams[ctx->video_stream_index]->codecpar))
    {
        fprintf(stderr, "failled to open decoder\n");
        return 1;
    }

	return 0;
}

char open_decoder(AVCodecContext** ctx, AVCodec* codec, AVCodecParameters* param)
{
    *ctx = avcodec_alloc_context3(codec);
    if (!*ctx)
    {
        fprintf(stderr, "failled to alloc decoder context\n");
        return 1;
    }

    if (param)
    {
        if (avcodec_parameters_to_context(*ctx, param) < 0)
        {
            fprintf(stderr, "failled to set parameters to decoder context\n");
            avcodec_free_context(*ctx);
            return 1;
        }
    }

    if (avcodec_open2(*ctx, codec, NULL) < 0)
    {
        fprintf(stderr, "failled to open decoder\n");
        return 1;
    }

    return 0;
}

char decode_frame(CodingContext* ctx, AVFrame* frame, AVPacket* packet)
{
    int response;
    response = avcodec_send_packet(ctx->codec_ctx, packet);
    if (response < 0)
    {
        fprintf(stderr, "failled to send packet to decoder\n");
        return -1;
    }

    response = avcodec_receive_frame(ctx->codec_ctx, frame);
    if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
        return 1;
    }
    else if (response < 0)
    {
        fprintf(stderr, "failled to receive frame\n");
        return -1;
    }

    return 0;
}

char open_output(AVFormatContext* input, CodingContext* output, const char* file)
{
    if (avformat_alloc_output_context2(&output->format_ctx, NULL, NULL, file) < 0)
    {
        fprintf(stderr, "failled to alloc output context\n");
        return 1;
    }

    AVStream* tmp_stream;

    for (int i = 0; i < input->nb_streams; i++)
    {
        tmp_stream = avformat_new_stream(output->format_ctx, 0);
        if (!tmp_stream)
        {
            fprintf(stderr, "failled to make new output stream\n");
            return 1;
        }

        if (avcodec_parameters_copy(tmp_stream->codecpar, input->streams[i]->codecpar) < 0)
        {
            fprintf(stderr, "failled copy input codec parameters to output\n");
            return 1;
        }
    }

    if (!(output->format_ctx->oformat->flags & AVFMT_NOFILE))
    {
        if (avio_open(&output->format_ctx->pb, file, AVIO_FLAG_WRITE) < 0) {
            fprintf(stderr, "failled to io open output\n");
            return 1;
        }
    }

    if (avformat_write_header(output->format_ctx, NULL) < 0)
    {
        fprintf(stderr, "failled to write output file headers\n");
        return 1;
    }

    return 0;
}

char open_encoder(CodingContext* ctx, char* encoder_name, AVDictionary** codec_options, AVRational timebase, AVCodecParameters* input_params)
{
    AVCodec* codec = avcodec_find_encoder_by_name(encoder_name);
    ctx->codec_ctx = avcodec_alloc_context3(codec);
    if (!ctx->codec_ctx)
    {
        fprintf(stderr, "failled to alloc encoder context\n");
        return 1;
    }

    avcodec_parameters_to_context(ctx->codec_ctx, input_params);

    ctx->codec_ctx->profile = FF_PROFILE_UNKNOWN;
    ctx->codec_ctx->time_base = timebase;

    if (ctx->format_ctx->oformat->flags & AVFMT_GLOBALHEADER)
        ctx->codec_ctx->flags |= AVFMT_GLOBALHEADER;

    if (avcodec_open2(ctx->codec_ctx, codec, codec_options) < 0)
    {
        fprintf(stderr, "failled to open encoder\n");
        return 1;
    }

    return 0;
}

char close_output_file(AVFormatContext* format)
{
    if (av_write_trailer(format))
    {
        fprintf(stderr, "failled to write output trailler\n");
        return 1;
    }

    if (!(format->oformat->flags & AVFMT_NOFILE)) {
        if (avio_close(format->pb) < 0)
        {
            fprintf(stderr, "failled to io close output\n");
            return 1;
        }
    }

    return 0;
}

void close_coding_context(CodingContext* ctx)
{
    if (ctx->codec_ctx)
        avcodec_close(ctx->codec_ctx);
    if (ctx->codec_ctx)
        avcodec_free_context(&ctx->codec_ctx);
    if (ctx->format_ctx)
        avformat_free_context(ctx->format_ctx);
}