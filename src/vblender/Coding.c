#include "Coding.h"

#include "utils/Error.h"

char open_input(CodingContext* ctx, const char* file, const char* decoder_name)
{
    int ret;

    if ((ret = avformat_open_input(&ctx->format_ctx, file, NULL, NULL)) < 0)
    {
        fprintf(stderr, "failled to format open input\n");
        print_av_error(ret);
        close_coding_context(ctx);
        return 1;
    }
    if ((ret = avformat_find_stream_info(ctx->format_ctx, NULL)) < 0)
    {
        fprintf(stderr, "failled to find stream infos\n");
        print_av_error(ret);
        close_coding_context(ctx);
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
        close_coding_context(ctx);
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
        close_coding_context(ctx);
        return 1;
    }

    if ((ret = open_decoder(&ctx->codec_ctx, decoder, ctx->format_ctx->streams[ctx->video_stream_index]->codecpar)))
    {
        fprintf(stderr, "failled to open decoder\n");
        print_av_error(ret);
        close_coding_context(ctx);
        return 1;
    }

	return 0;
}

char open_decoder(AVCodecContext** ctx, AVCodec* codec, AVCodecParameters* param)
{
    int ret;

    *ctx = avcodec_alloc_context3(codec);
    if (!*ctx)
    {
        fprintf(stderr, "failled to alloc decoder context\n");
        return 1;
    }

    if (param)
    {
        if ((ret = avcodec_parameters_to_context(*ctx, param)) < 0)
        {
            fprintf(stderr, "failled to set parameters to decoder context\n");
            print_av_error(ret);
            avcodec_free_context(ctx);
            *ctx = 0;
            return 1;
        }
    }

    if ((ret = avcodec_open2(*ctx, codec, NULL)) < 0)
    {
        fprintf(stderr, "failled to open decoder\n");
        print_av_error(ret);
        avcodec_free_context(ctx);
        *ctx = 0;
        return 1;
    }

    return 0;
}

char decode_frame(CodingContext* ctx, AVFrame* frame, AVPacket* packet)
{
    int ret;
    if ((ret = avcodec_send_packet(ctx->codec_ctx, packet)) < 0)
    {
        fprintf(stderr, "failled to send packet to decoder\n");
        print_av_error(ret);
        return -1;
    }

    ret = avcodec_receive_frame(ctx->codec_ctx, frame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        return 1;
    }
    else if (ret < 0)
    {
        fprintf(stderr, "failled to receive frame\n");
        print_av_error(ret);
        return -1;
    }

    return 0;
}

char open_output(AVFormatContext* input, CodingContext* output, const char* file)
{
    int ret = 0;

    if ((ret = avformat_alloc_output_context2(&output->format_ctx, NULL, NULL, file)) < 0)
    {
        fprintf(stderr, "failled to alloc output context\n");
        print_av_error(ret);
        close_coding_context(output);
        return 1;
    }

    AVStream* tmp_stream;

    for (int i = 0; i < input->nb_streams; i++)
    {
        tmp_stream = avformat_new_stream(output->format_ctx, 0);
        if (!tmp_stream)
        {
            fprintf(stderr, "failled to make new output stream\n");
            close_coding_context(output);
            return 1;
        }

        if ((ret = avcodec_parameters_copy(tmp_stream->codecpar, input->streams[i]->codecpar)) < 0)
        {
            fprintf(stderr, "failled copy input codec parameters to output\n");
            print_av_error(ret);
            close_coding_context(output);
            return 1;
        }
    }

    if (!(output->format_ctx->oformat->flags & AVFMT_NOFILE))
    {
        if ((ret = avio_open(&output->format_ctx->pb, file, AVIO_FLAG_WRITE)) < 0) {
            fprintf(stderr, "failled to io open output %s\n", file);
            print_av_error(ret);
            close_coding_context(output);
            return 1;
        }
    }

    if ((ret = avformat_write_header(output->format_ctx, NULL)) < 0)
    {
        fprintf(stderr, "failled to write output file headers\n");
        print_av_error(ret);
        close_coding_context(output);
        return 1;
    }

    return 0;
}

char open_encoder(CodingContext* ctx, char* encoder_name, AVDictionary** codec_options, AVRational timebase, AVCodecParameters* input_params)
{
    int ret;
    AVCodec* codec = avcodec_find_encoder_by_name(encoder_name);
    ctx->codec_ctx = avcodec_alloc_context3(codec);
    if (!ctx->codec_ctx)
    {
        fprintf(stderr, "failled to alloc encoder context\n");
        return 1;
    }

    ctx->codec_ctx->profile = FF_PROFILE_UNKNOWN;
    ctx->codec_ctx->time_base = timebase;
    ctx->codec_ctx->width = input_params->width;
    ctx->codec_ctx->height = input_params->height;
    ctx->codec_ctx->sample_aspect_ratio = input_params->sample_aspect_ratio;
    ctx->codec_ctx->pix_fmt = input_params->format;

    if (ctx->format_ctx->oformat->flags & AVFMT_GLOBALHEADER)
        ctx->codec_ctx->flags |= AVFMT_GLOBALHEADER;

    if ((ret = avcodec_open2(ctx->codec_ctx, codec, codec_options)) < 0)
    {
        fprintf(stderr, "failled to open encoder\n");
        print_av_error(ret);
        avcodec_free_context(&ctx->codec_ctx);
        ctx->codec_ctx = 0;
        return 1;
    }

    return 0;
}

char close_output_file(AVFormatContext* format)
{
    int ret;

    if ((ret = av_write_trailer(format)))
    {
        fprintf(stderr, "failled to write output trailler\n");
        print_av_error(ret);
        return 1;
    }

    if (!(format->oformat->flags & AVFMT_NOFILE)) {
        if ((ret = avio_close(format->pb) < 0))
        {
            fprintf(stderr, "failled to io close output\n");
            print_av_error(ret);
            return 1;
        }
    }

    return 0;
}

void close_coding_context(CodingContext* ctx)
{
    if (ctx->codec_ctx)
    {
        avcodec_close(ctx->codec_ctx);
        avcodec_free_context(&ctx->codec_ctx);
        ctx->codec_ctx = 0;
    }
    if (ctx->format_ctx)
    {
        avformat_free_context(ctx->format_ctx);
        ctx->format_ctx = 0;
    }
    ctx->video_stream_index = 0;
}

void print_av_error(int error)
{
    char buffer[1024] = { 0 };
    if (av_strerror(error, buffer, 1024) < 0)
    {
        fprintf(stderr, "can't read av error\n");
    }
    else
    {
        fprintf(stderr, "%s\n", buffer);
    }
}