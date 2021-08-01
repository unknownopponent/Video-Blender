#pragma comment( lib, "avcodec.lib" )
#pragma comment( lib, "avutil.lib" )
#pragma comment( lib, "avformat.lib" )

#include "Coding.h"
#include "Convert.h"
#include "utils/Queue.h"
#include "utils/Stock.h"

#include "OpenGL.h"

#include "windows.h"

typedef struct Thread_Parameters
{
    HANDLE thread;

    RGBFrame** in_frames;
    int* weights;
    int nb;

    CodingContext* output;
    int* nb_frame_created;
    int pts;

    int frame_id;

    RGBFrame* rgb_result;
    struct SwsContext* rgb_to_yuv;
    AVFrame* out_frame;

    char opengl;
    char* opengl_encode;
    HANDLE* thread_encode;
    Programs* programs;

    HDC dc;
    HGLRC gl_ctx;

} t_param;

void encode_thread(t_param* params)
{
    params->out_frame->pts = params->pts;
    sws_scale(params->rgb_to_yuv,
        params->rgb_result->data,
        params->rgb_result->linesize,
        0,
        params->output->codec_ctx->height,
        params->out_frame->data,
        params->out_frame->linesize);

    if (!params->opengl)
    {
        while (*params->nb_frame_created != params->frame_id)
            Sleep(1);
    }

    if (encode(params->output, params->out_frame))
    {
        printf("encoding error\n");
    }

    *params->opengl_encode = 0;

    printf("%d frames\r", params->frame_id);
    *params->nb_frame_created += 1;
}

void blend_and_encode(t_param* params)
{
    int width = params->output->codec_ctx->width;
    int height = params->output->codec_ctx->height;
    int frame_size = width * height * 3;

    if (params->opengl)
    {
        if (!wglMakeCurrent(params->dc, params->gl_ctx))
        {
            printf("can't wglMakeCurrent\n");
            return ;
        }

        //int gl_count = 
        float* gl_frame = malloc(frame_size * sizeof(float));
        if (!gl_frame)
        {
            printf("can't malloc\n");
            return 1;
        }

        float total_weights = 0.0f;
        for (int i = 0; i < params->nb; i++)
        {
            total_weights += (float)params->weights[i];
        }

        GLenum err;

        glUseProgram(params->programs->init);
        glDispatchCompute(params->programs->nb_x, params->programs->nb_y, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        for (int i = 0; i < params->nb; i++)
        {
            for (int j = 0; j < frame_size; j++)
                gl_frame[j] = (float)params->in_frames[i]->data[0][j];
            glUseProgram(params->programs->add);
            glUniform1f(params->programs->w_location, (float)params->weights[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGB, GL_FLOAT, gl_frame);
            glDispatchCompute(params->programs->nb_x, params->programs->nb_y, 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        }

        glUseProgram(params->programs->finish);
        glUniform1f(params->programs->tw_location, total_weights);
        glDispatchCompute(params->programs->nb_x, params->programs->nb_y, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glGetTextureImage(params->programs->texture,
            0,
            GL_RGB,
            GL_FLOAT,
            frame_size * sizeof(float),
            gl_frame);

        while ((err = glGetError()) != GL_NO_ERROR)
        {
            printf("%d\n", err);
        }

        for (int j = 0; j < frame_size; j++)
            params->rgb_result->data[0][j] = (unsigned char)gl_frame[j];

        if (!wglMakeCurrent(params->dc, 0))
        {
            printf("can't wglMakeCurrent\n");
            return ;
        }

        while (*params->opengl_encode)
            Sleep(1);

        if (params->thread_encode)
        {
            WaitForSingleObject(params->thread_encode, INFINITE);
        }

        *params->opengl_encode = 1;
        params->thread_encode = CreateThread(NULL, 0, encode_thread, params, 0, NULL);

        free(gl_frame);

        return;
    }
    
    blend(params->rgb_result, params->in_frames, params->weights, params->nb, frame_size);
    
    encode_thread(params);
}

int blend(RGBFrame* result, RGBFrame** in_frames, int* weights, int nb, int data_size)
{
    int* tmp_frame = calloc(data_size, sizeof(int));
    if (!tmp_frame)
    {
        printf("can't malloc tmp frame\n");
        return 1;
    }

    int total_weights = 0;
    for (int i = 0; i < nb; i++)
    {
        if (weights[i] == 0)
            continue;
        total_weights += weights[i];
        for (int j = 0; j < data_size; j++)
        {
            tmp_frame[j] += in_frames[i]->data[0][j] * weights[i];
        }
    }

    int tmp = 0;
    for (int i = 0; i < data_size; i++)
    {
        tmp = tmp_frame[i] % total_weights;
        result->data[0][i] = tmp_frame[i] / total_weights;
        if (tmp)
        {
            if (tmp * 2 >= total_weights)
            {
                result->data[0][i] += 1;
            }
        }
    }

    free(tmp_frame);
    return 0;
}

int mainc()
{
    Stock stock;
    sinit(&stock, sizeof(float*));
    int* f = 0;
    int index;
    while (f<5)
    {
        if (sget_available(&stock, &f, &index)) //pas de libre
        {
            sadd(&stock, f);
        }
        else
        {
            printf("");
        }
        f += 1;
    }
    sset(&stock, 1, 0);
    if (sget_available(&stock, &f, &index))
    {
        printf("");
    }
    else
    {
        printf("");
    }
}

int main(int argc, char** args)
{
    Settings settings;
    if (parse(&settings, argc, args))
    {
        printf("can't parse args\n");
        return 1;
    }

    printf("options parsed\n");

    CodingContext input = { 0 };
    if (open_input(&input, &settings))
    {
        printf("can't open input\n");
        return 2;
    }

    av_dump_format(input.format_ctx, 0, settings.input_file, 0);

    printf("using ");
    printf(input.codec_ctx->codec->long_name);
    printf(" decoder\n");

    AVFrame* in_frame = av_frame_alloc();
    if (!in_frame)
    {
        printf("can't av_frame_alloc\n");
        return 3;
    }

    int res = read(&input, in_frame);
    if (res == 1)
    {
        printf("failled to read first frame\n");
        return 4;
    }
    if (res == -1)
    {
        printf("end of the stream for the first frame\n");
        return 4;
    }

    if ((float)input.codec_ctx->time_base.den / (float)input.codec_ctx->time_base.num / (float)input.codec_ctx->ticks_per_frame < (float)settings.den / (float)settings.num)
    {
        printf("output framerate is higher than input\n");
        return 1;
    }

    CodingContext output = { 0 };
    if (open_output(&input, &output, &settings))
    {
        printf("can't open input\n");
        return 5;
    }

    av_dump_format(output.format_ctx, 0, settings.output_file, 1);

    printf("using ");
    printf(output.codec_ctx->codec->long_name);
    printf(" decoder\n");

    char opengl_encode = 0;
    HANDLE thread_encode = 0;

    Programs programs;
    float* gl_frame = 0;
    unsigned int gl_count;
    unsigned int gl_size;
    float total_weights = 0.0f;
    RGBFrame* rgb_result = 0;
    AVFrame* gl_out_frame = 0;
    struct SwsContext* rgb_to_yuv = 0;

    HDC dc = 0;
    HGLRC gl_ctx = 0;

    if (settings.opengl)
    {
        if (set_up_opengl_context(&programs, in_frame->width, in_frame->height, &dc, &gl_ctx))
        {
            printf("can't set up opengl\n");
            return 1;
        }
    }

    struct SwsContext* yuv_to_rgb =
        sws_getContext(in_frame->width,
            in_frame->height,
            input.codec_ctx->pix_fmt,
            in_frame->width,
            in_frame->height,
            AV_PIX_FMT_RGB24,
            SWS_BILINEAR,
            NULL, NULL, NULL);
    if (!yuv_to_rgb)
    {
        printf("can't sws_getContext\n");
        return 6;
    }

    RGBFrame* rgb;
    if (alloc_RGBFrame(&rgb, 0, input.codec_ctx->width, input.codec_ctx->height))
    {
        printf("can't alloc_RGBFrame");
        return 8;
    }

    sws_scale(yuv_to_rgb,
        in_frame->data,
        in_frame->linesize,
        0,
        in_frame->height,
        rgb->data,
        rgb->linesize);

    char finished_read = 0;
    int oldest_read_frame_id = 0;
    res = 0;
    int nb_frame_created = 0;
    int nb_frame_read = 1;
    float if_num = input.codec_ctx->framerate.num;
    float if_den = input.codec_ctx->framerate.den;
    char odd = settings.w_count % 2;

    float tmp, tmp2, tmp3;

    int nb_frame_target = -1;

    float pts_step = (float)output.format_ctx->streams[output.stream_i]->time_base.num
        * (float)output.format_ctx->streams[output.stream_i]->time_base.den
        / (float)settings.den
        * (float)settings.num;

    Queue threads;
    init(&threads, sizeof(HANDLE));

    HANDLE tmp_handle;
    t_param* tmp_params;
    int nb_thread_launched = 0;

    int last_frames_id = 0;

    Stock stock;
    sinit(&stock, sizeof(t_param*));

    Stock frames;
    sinit(&frames, sizeof(RGBFrame*));
    sadd(&frames, rgb);

    int index;

    printf("Started to encode\n");

    while (!finished_read)
    {
        res = read(&input, in_frame);
        if (res == 1)
        {
            printf("failled to read frame\n");
            return 9;
        }
        if (res == -1)
        {
            finished_read = 1;
        }
        if (sget_available(&frames, &rgb, &index))
        {
            if (alloc_RGBFrame(&rgb, nb_frame_read, input.codec_ctx->width, input.codec_ctx->height))
            {
                printf("can't alloc_RGBFrame\n");
                return 10;
            }
            if (sadd(&frames, rgb))
            {
                printf("");
                return 1;
            }
        }
        else
            sset(&frames, index, 1);
        rgb->id = nb_frame_read;

        sws_scale(yuv_to_rgb,
            in_frame->data,
            in_frame->linesize,
            0,
            in_frame->height,
            rgb->data,
            rgb->linesize);

        nb_frame_read += 1;

        if (nb_frame_target != -1)
        {
            if (nb_frame_read >= nb_frame_target)
            {
                while ((nb_thread_launched - nb_frame_created) < threads.size || threads.size >= settings.threads)
                {
                    if (front(&threads, &tmp_handle))
                    {
                        printf("error in thread queue\n");
                        return 14;
                    }
                    if (qremove(&threads))
                    {
                        printf("error in thread queue\n");
                        return 15;
                    }
                    WaitForSingleObject(tmp_handle, INFINITE);

                    char found = 0;

                    for (int i = 0; i < stock.elements.size; i++)
                    {
                        if (sget(&stock, i, &tmp_params))
                        {
                            printf("");
                            return 1;
                        }
                        if (tmp_params->thread == tmp_handle)
                        {
                            if (sset(&stock, i, 0))
                            {
                                printf("");
                                return 1;
                            }
                            found = 1;
                            break;
                        }
                    }

                    if (!found)
                    {
                        printf("");
                        return 1;
                    }

                    CloseHandle(tmp_handle);
                }

                int min_id = 0x7fffffff;
                for (int i = 0; i < stock.elements.size; i++)
                {
                    if (sget(&stock, i, &tmp_params))
                    {
                        printf("");
                        return 1;
                    }
                    for (int j = 0; j < tmp_params->nb; j++)
                    {
                        if (tmp_params->in_frames[j]->id < min_id)
                        {
                            min_id = tmp_params->in_frames[j]->id;
                        }
                    }
                }

                for (int i = 0; i < frames.availble.size; i++)
                {
                    if (sget(&frames, i, &rgb))
                    {
                        printf("");
                        return 1;
                    }
                    if (rgb->id < min_id)
                    {
                        sset(&frames, i, 0);
                    }
                }
                
                if (sget_available(&stock, &tmp_params, &index))
                {
                    tmp_params = malloc(sizeof(t_param));
                    if (!tmp_params)
                    {
                        printf("can't alloc\n");
                        return 16;
                    }
                    tmp_params->in_frames = malloc(sizeof(RGBFrame**) * settings.w_count);
                    if (!tmp_params->in_frames)
                    {
                        printf("can't malloc\n");
                        return 1;
                    }
                    if (alloc_RGBFrame(&tmp_params->rgb_result, 0, in_frame->width, in_frame->height))
                    {
                        printf("");
                        return 1;
                    }
                    tmp_params->rgb_to_yuv =
                        sws_getContext(in_frame->width,
                            in_frame->height,
                            AV_PIX_FMT_RGB24,
                            in_frame->width,
                            in_frame->height,
                            output.codec_ctx->pix_fmt,
                            SWS_BILINEAR,
                            NULL, NULL, NULL);
                    if (!tmp_params->rgb_to_yuv)
                    {
                        printf("can't sws_getContext\n");
                        return 7;
                    }
                    tmp_params->out_frame = av_frame_alloc();
                    if (!tmp_params->out_frame)
                    {
                        printf("can't av_frame_alloc\n");
                        return 8;
                    }
                    int width = in_frame->width;
                    int height = in_frame->height;
                    int frame_size = width * height * 3;
                    tmp_params->out_frame->data[0] = malloc(frame_size);
                    tmp_params->out_frame->data[1] = tmp_params->out_frame->data[0] + width * height;
                    tmp_params->out_frame->data[2] = tmp_params->out_frame->data[0] + width * height * 2;
                    tmp_params->out_frame->data[3] = 0;
                    tmp_params->out_frame->linesize[0] = width;
                    tmp_params->out_frame->linesize[1] = width;
                    tmp_params->out_frame->linesize[2] = width;
                    tmp_params->out_frame->linesize[4] = 0;
                    tmp_params->out_frame->format = AV_PIX_FMT_YUV444P;
                    tmp_params->out_frame->width = width;
                    tmp_params->out_frame->height = height;
                    tmp_params->programs = &programs;
                    tmp_params->opengl = settings.opengl;
                    tmp_params->opengl_encode = &opengl_encode;
                    tmp_params->dc = dc;
                    tmp_params->gl_ctx = gl_ctx;
                    tmp_params->thread_encode = &thread_encode;
                    sadd(&stock, tmp_params);
                }
                else
                    sset(&stock, index, 1);

                tmp_params->frame_id = last_frames_id;
                tmp_params->nb_frame_created = &nb_frame_created;
                tmp_params->output = &output;
                tmp_params->pts = (float)last_frames_id * pts_step;
                int max = nb_frame_target;
                int min = nb_frame_target - settings.w_count;
                if (min < 0)
                    min = 0;
                tmp_params->nb = max - min;
                for (int i = 0; i < frames.elements.size; i++)
                {
                    if (sget(&frames, i, &rgb))
                    {
                        printf("");
                        return 1;
                    }
                    if (rgb->id <= max && rgb->id >= min)
                    {
                        tmp_params->in_frames[rgb->id - min] = rgb;
                    }
                }
                tmp_params->weights = &settings.weights[settings.w_count - tmp_params->nb];

                last_frames_id += 1;

                tmp_handle = CreateThread(NULL, 0, blend_and_encode, tmp_params, 0, NULL);
                tmp_params->thread = tmp_handle;
                add(&threads, tmp_handle);
                nb_thread_launched += 1;
                nb_frame_target = -1;
            }
            continue;
        }

        tmp = (float)(nb_frame_read - 1) * if_den / if_num;
        tmp2 = (float)nb_thread_launched / (float)settings.den * (float)settings.num;

        if (tmp < tmp2)
            continue;

        if (odd)
        {
            if (tmp - tmp2 < tmp2 - ((float)(nb_frame_read - 2) * if_den / if_num))
            {
                nb_frame_target = nb_frame_read + (settings.w_count / 2);
            }
            else
            {
                nb_frame_target = nb_frame_read + (settings.w_count / 2) - 1;
            }
        }
        else
        {
            nb_frame_target = nb_frame_read + (settings.w_count / 2) - 1;
        }
    }

    while (threads.size > 0)
    {
        if (front(&threads, &tmp_handle))
        {
            printf("error in thread queue\n");
            return 14;
        }
        if (qremove(&threads))
        {
            printf("error in thread queue\n");
            return 15;
        }
        WaitForSingleObject(tmp_handle, INFINITE);
        CloseHandle(tmp_handle);
    }

    //free memory

    if (encode(&output, NULL))
    {
        printf("encoding error\n");
    }

    if (close_output_file(output.format_ctx))
    {
        printf("can't close_output_file");
        return 1;
    }

    printf("\n");

    close_coding_context(&input);
    close_coding_context(&output);

    return 0;
}