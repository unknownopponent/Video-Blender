#include "Coding.h"

#include <Windows.h> // __try

int open_input(CodingContext* in_ctx, Settings* settings)
{
    if (avformat_open_input(&in_ctx->format_ctx, settings->input_file, NULL, NULL) < 0)
    {
        printf("can't avformat_open_input\n");
        return 1;
    }

    if (avformat_find_stream_info(in_ctx->format_ctx, NULL) < 0)
    {
        printf("can't avformat_find_stream_info\n");
        return 1;
    }

    AVStream* stream = 0;
    in_ctx->stream_i = -1;

    for (int i = 0; i < in_ctx->format_ctx->nb_streams; i++)
        if (in_ctx->format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            in_ctx->stream_i = i;
            stream = in_ctx->format_ctx->streams[i];
            break;
        }
    if (in_ctx->stream_i == -1)
    {
        printf("can't find video stream in file\n");
        return 1;
    }

    in_ctx->packet = av_packet_alloc();
    if (!in_ctx->packet)
    {
        printf("can't allocate packet\n");
        return 1;
    }

    char codec_open = 0;
    
    if (settings->hardware_decoding)
    {
        const AVCodec* p = NULL;
        void* i = 0;

        while ((p = av_codec_iterate(&i))) {
            if (p == NULL)
                break;

            if (p->id == stream->codecpar->codec_id)
            {
                if (av_codec_is_decoder(p))
                {
                    if (strstr(p->long_name, settings->hardware_decoding))
                    {
                        if (open_decoder(
                            &in_ctx->codec_ctx,
                            p,
                            stream->codecpar))
                        {
                            printf("can't open hardware decoder\n");
                            return 1;
                        }
                        codec_open = 1;
                        break;
                    }
                }
            }
        }
    }

    if (!codec_open)
    {
        if (settings->hardware_decoding)
            printf("did not found compatible hardware decoder\n");

        if (open_decoder(
            &in_ctx->codec_ctx,
            avcodec_find_decoder(stream->codecpar->codec_id),
            stream->codecpar))
        {
            printf("can't open decoder\n");
            return 1;
        }
    }

	return 0;
}

int open_decoder(AVCodecContext** enc_ctx, AVCodec* codec, AVCodecParameters* param)
{
    *enc_ctx = avcodec_alloc_context3(codec);
    if (*enc_ctx == NULL)
    {
        printf("failled to avcodec_alloc_context3 for ");
        printf(codec->long_name);
        printf("\n");
        return 1;
    }

    if ( param != NULL )
        if (avcodec_parameters_to_context(*enc_ctx, param) < 0)
        {
            printf("failled to avcodec_parameters_to_context for ");
            printf(codec->long_name);
            printf("\n");
            avcodec_free_context(*enc_ctx);
            return 1;
        }

    char failed = 0;

    __try
    {
        if (avcodec_open2(*enc_ctx, codec, NULL) < 0)
        {
            failed = 1;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        printf("failled to avcodec_open2 for ");
        printf(codec->long_name);
        printf("\n");
        
        __try // gives me random exception i don't understand, doing this fixed...
        {
            avcodec_close(*enc_ctx);
            avcodec_free_context(enc_ctx);
            failed = 1;
            return 1;
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            return 1;
        }
    }

    if (failed)
    {
        printf("failled to avcodec_open2 for ");
        printf(codec->long_name);
        printf("\n");
        avcodec_free_context(*enc_ctx);
        return 1;
    }

    return 0;
}


int contains(enum AVPixelFormat target, enum AVPixelFormat* src)
{
    if (src == 0 || src[0] == -1)
        return 0;

    int i = 1;

    while (src[i] != -1)
    {
        if (src[i] == target)
        {
            return 1;
        }
        i += 1;
    }

    return 0;
}

int read(CodingContext* in_ctx, AVFrame* frame)
{
    int response;
    while (av_read_frame(in_ctx->format_ctx, in_ctx->packet) >= 0) {

        if (in_ctx->packet->stream_index != in_ctx->stream_i)
            continue;

        response = avcodec_send_packet(in_ctx->codec_ctx, in_ctx->packet);
        if (response < 0)
        {
            printf("failled to avcodec_send_packet\n");
            return 1;
        }

        response = avcodec_receive_frame(in_ctx->codec_ctx, frame);
        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
            av_packet_unref(in_ctx->packet);
            continue;
        }
        else if (response < 0)
        {
            printf("failled to avcodec_receive_frame\n");
            return 1;
        }

        av_packet_unref(in_ctx->packet);
        return 0;
    }

    return -1;
}

enum AVPixelFormat get_lossless_format(int bit_per_channel)
{
    switch (bit_per_channel)
    {
    case 8: return AV_PIX_FMT_YUV444P;
    case 9: return AV_PIX_FMT_YUV444P9LE;
    case 10: return AV_PIX_FMT_YUV444P10LE;
    case 12: return AV_PIX_FMT_YUV444P12LE;
    case 14: return AV_PIX_FMT_YUV444P14LE;
    case 16: return AV_PIX_FMT_YUV444P16LE;
    }

    return 0;
}

int open_output(CodingContext* in_ctx, CodingContext* out_ctx, Settings* settings)
{
    if (avformat_alloc_output_context2(&out_ctx->format_ctx, NULL, NULL, settings->output_file) < 0)
    {
        printf("can't avformat_alloc_output_context2\n");
        return 1;
    }

    if (avformat_new_stream(out_ctx->format_ctx, 0) == NULL)
    {
        printf("can't avformat_new_stream\n");
        return 1;
    }

    char codec_open = 0;
    enum AVPixelFormat format = get_lossless_format(in_ctx->codec_ctx->bits_per_raw_sample);
    if (!format)
    {
        printf("pixel format not supported for encoding\n");
        return 1;
    }

    const AVCodec* c = NULL;

    if (settings->hardware_encoding)
    {
        
        void* i = 0;

        while ((c = av_codec_iterate(&i))) {
            if (c == NULL)
                break;

            if (c->id == AV_CODEC_ID_H265)
            {
                if (av_codec_is_encoder(c))
                {
                    if (strstr(c->long_name, settings->hardware_encoding))
                    {
                        if (contains(format, c->pix_fmts))
                        {
                            if (open_encoder(
                                out_ctx,
                                in_ctx->codec_ctx,
                                c,
                                format,
                                settings->num,
                                settings->den))
                            {
                                printf("can't open hardware decoder\n");
                                return;
                            }
                            codec_open = 1;
                            break;
                        }
                    }
                }
            }
        }
    }

    if (!codec_open)
    {
        if (settings->hardware_encoding)
            printf("did not found compatible hardware encoder\n");

        c = avcodec_find_encoder(AV_CODEC_ID_H265);

        if (!contains(format, c->pix_fmts))
        {
            printf("did not found compatible software decoder\n");
            return 1;
        }

        if (open_encoder(
            out_ctx,
            in_ctx->codec_ctx,
            c,
            format,
            settings->num,
            settings->den))
        {
            printf("can't open hardware decoder\n");
            return 1;
        }
    }

    if (!(out_ctx->format_ctx->oformat->flags & AVFMT_NOFILE)) 
    {
        if (avio_open(&out_ctx->format_ctx->pb, settings->output_file, AVIO_FLAG_WRITE) < 0) {
            printf("can't avio_open\n");
            return 1;
        }
    }

    if (avformat_write_header(out_ctx->format_ctx, NULL) < 0) 
    {
        printf("can't avformat_write_header\n");
        return 1;
    }

    out_ctx->packet = av_packet_alloc();

    return 0;
}

int open_encoder(CodingContext* out_ctx, AVCodecContext* dec_ctx, AVCodec* codec, enum AVPixelFormat format, int num, int den)
{
    out_ctx->codec_ctx = avcodec_alloc_context3(codec);
    if (!out_ctx->codec_ctx)
    {
        printf("can't avcodec_alloc_context3\n");
        return 1;
    }

    avcodec_parameters_to_context(out_ctx->codec_ctx, out_ctx->format_ctx->streams[0]->codecpar);
    out_ctx->codec_ctx->height = dec_ctx->height;
    out_ctx->codec_ctx->width = dec_ctx->width;
    out_ctx->codec_ctx->sample_aspect_ratio = dec_ctx->sample_aspect_ratio;
    out_ctx->codec_ctx->pix_fmt = AV_PIX_FMT_YUV444P;
    AVRational timebase = (AVRational){ num,den };
    out_ctx->codec_ctx->time_base = timebase;

    if (out_ctx->format_ctx->oformat->flags & AVFMT_GLOBALHEADER)
        out_ctx->codec_ctx->flags |= AVFMT_GLOBALHEADER;

    if (avcodec_open2(out_ctx->codec_ctx, codec, NULL) < 0)
    {
        printf("can't avcodec_open2\n");
        return 1;
    }

    avcodec_parameters_from_context(out_ctx->format_ctx->streams[0]->codecpar, out_ctx->codec_ctx);

    out_ctx->format_ctx->streams[0]->time_base = timebase;

    return 0;
}

int encode(CodingContext* out_ctx, AVFrame* frame)
{
    av_packet_unref(out_ctx->packet);

    int ret = avcodec_send_frame(out_ctx->codec_ctx, frame);

    if (ret < 0)
        return ret;

    while (ret >= 0) {
        ret = avcodec_receive_packet(out_ctx->codec_ctx, out_ctx->packet);

        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return 0;

        out_ctx->packet->stream_index = out_ctx->stream_i;

        ret = av_interleaved_write_frame(out_ctx->format_ctx, out_ctx->packet);
    }

    return ret;
}

int close_output_file(AVFormatContext* format)
{
    if (av_write_trailer(format))
    {
        printf("can't av_write_trailer\n");
        return 1;
    }

    if (!(format->oformat->flags & AVFMT_NOFILE)) {
        if (avio_close(format->pb) < 0)
        {
            printf("can't avio_close\n");
            return 1;
        }
    }

    return 0;
}

int close_coding_context(CodingContext* ctx)
{
    av_packet_free(&ctx->packet);
    avcodec_close(ctx->codec_ctx);
    avcodec_free_context(&ctx->codec_ctx);
    avformat_free_context(ctx->format_ctx);
}