#pragma comment( lib, "avcodec.lib" )
#pragma comment( lib, "avutil.lib" )
#pragma comment( lib, "avformat.lib" )

#include "Coding.h"
#include "Convert.h"
#include "utils/Queue.h"

#include "OpenGL.h"

#include "windows.h"

typedef struct Thread_Parameters
{
    RGBFrame** in_frames;
    int* weigths;
    int nb;

    CodingContext* output;
    int* nb_frame_created;
    int pts;

    int frame_id;
} t_param;

void blend_and_encode(t_param* params)
{
    int width = params->output->codec_ctx->width;
    int height = params->output->codec_ctx->height;
    int frame_size = width * height * 3;
    RGBFrame* rgb_result;
    if (alloc_RGBFrame(&rgb_result, *params->nb_frame_created, width, height))
    {
        ;
    }
    blend(rgb_result, params->in_frames, params->weigths, params->nb, frame_size);
    struct SwsContext* rgb_to_yuv =
        sws_getContext(width,
            height,
            AV_PIX_FMT_RGB24,
            width,
            height,
            params->output->codec_ctx->pix_fmt,
            SWS_BILINEAR,
            NULL, NULL, NULL);
    if (!rgb_to_yuv)
    {
        printf("can't sws_getContext\n");
        return 7;
    }
    AVFrame* out_frame = av_frame_alloc();
    if (!out_frame)
    {
        printf("can't av_frame_alloc\n");
        return 8;
    }
    out_frame->data[0] = malloc(frame_size);
    out_frame->data[1] = out_frame->data[0] + width * height;
    out_frame->data[2] = out_frame->data[0] + width * height * 2;
    out_frame->data[3] = 0;
    out_frame->linesize[0] = width;
    out_frame->linesize[1] = width;
    out_frame->linesize[2] = width;
    out_frame->linesize[4] = 0;
    out_frame->format = AV_PIX_FMT_YUV444P;
    out_frame->width = width;
    out_frame->height = height;
    out_frame->pts = params->pts;
    sws_scale(rgb_to_yuv,
        rgb_result->data,
        rgb_result->linesize,
        0,
        height,
        out_frame->data,
        out_frame->linesize);

    while (*params->nb_frame_created != params->frame_id)
        Sleep(1);

    if (encode(params->output, out_frame))
    {
        printf("encoding error\n");
    }

    free_RGBFrame(rgb_result);
    sws_freeContext(rgb_to_yuv);
    free(out_frame->data[0]);
    av_frame_free(&out_frame);
    printf("%d frames\r", params->frame_id);
    *params->nb_frame_created += 1;
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

    Programs programs;
    float* gl_frame = 0;
    unsigned int gl_count;
    unsigned int gl_size;
    float total_weights = 0.0f;
    RGBFrame* rgb_result = 0;
    AVFrame* gl_out_frame = 0;
    struct SwsContext* rgb_to_yuv = 0;

    if (settings.opengl)
    {
        if (set_up_opengl_context(&programs, in_frame->width, in_frame->height))
        {
            printf("can't set up opengl\n");
            return 1;
        }
        gl_count = in_frame->width * in_frame->height * 3;
        gl_size = gl_count * sizeof(float);
        gl_frame = malloc(gl_size);
        if (!gl_frame)
        {
            printf("can't malloc\n");
            return 1;
        }
        for (int i = 0; i < settings.w_count; i++)
        {
            total_weights += (float)settings.weights[i];
        }

        gl_out_frame = av_frame_alloc();
        if (!gl_out_frame)
        {
            printf("can't av_frame_alloc\n");
            return 8;
        }
        gl_out_frame->data[0] = malloc(gl_count * sizeof(unsigned char));
        gl_out_frame->data[1] = gl_out_frame->data[0] + in_frame->width * in_frame->height;
        gl_out_frame->data[2] = gl_out_frame->data[0] + in_frame->width * in_frame->height * 2;
        gl_out_frame->data[3] = 0;
        gl_out_frame->linesize[0] = in_frame->width;
        gl_out_frame->linesize[1] = in_frame->width;
        gl_out_frame->linesize[2] = in_frame->width;
        gl_out_frame->linesize[4] = 0;
        gl_out_frame->format = AV_PIX_FMT_YUV444P;
        gl_out_frame->width = in_frame->width;
        gl_out_frame->height = in_frame->height;

        rgb_to_yuv =
            sws_getContext(in_frame->width,
                in_frame->height,
                AV_PIX_FMT_RGB24,
                in_frame->width,
                in_frame->height,
                output.codec_ctx->pix_fmt,
                SWS_BILINEAR,
                NULL, NULL, NULL);
        if (!rgb_to_yuv)
        {
            printf("can't sws_getContext\n");
            return 7;
        }

        if (alloc_RGBFrame(&rgb_result, 0, in_frame->width, in_frame->height))
        {
            printf("can't alloc rgbframe\n");
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

    //test(rgb, in_frame->width, in_frame->height);

    Queue q;
    init(&q, sizeof(RGBFrame*));
    add(&q, rgb);
    rgb = 0;

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
    Queue params;
    init(&params, sizeof(t_param*));

    HANDLE tmp_handle;
    t_param* tmp_params;
    int nb_thread_launched = 0;

    int last_frames_id = 0;

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
        if (alloc_RGBFrame(&rgb, nb_frame_read, input.codec_ctx->width, input.codec_ctx->height))
        {
            printf("can't alloc_RGBFrame\n");
            return 10;
        }
        sws_scale(yuv_to_rgb,
            in_frame->data,
            in_frame->linesize,
            0,
            in_frame->height,
            rgb->data,
            rgb->linesize);
        if (add(&q, rgb))
        {
            printf("can't add to queue\n");
            return 11;
        }
        nb_frame_read += 1;

        if (nb_frame_target != -1)
        {
            if (nb_frame_read >= nb_frame_target)
            {
                if (settings.opengl)
                {
                    //initialize
                    //for each frame
                    //  convert frame to float frame
                    //  set uniform weight
                    //  add
                    //set uniform total weights
                    //finish
                    //download frame
                    //encode frame
                    //free frames



                    GLenum err;

                    glUseProgram(programs.init);
                    glDispatchCompute(programs.nb_x, programs.nb_y, 1);
                    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

                    for (int i = 0; i < settings.w_count; i++)
                    {
                        get(&q, i, &rgb);
                        for (int j = 0; j < gl_count; j++)
                            gl_frame[j] = (float)rgb->data[0][j];
                        glUseProgram(programs.add);
                        glUniform1f(programs.w_location, (float)settings.weights[i]);
                        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, in_frame->width, in_frame->height, 0, GL_RGB, GL_FLOAT, gl_frame);
                        glDispatchCompute(programs.nb_x, programs.nb_y, 1);
                        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
                    }

                    glUseProgram(programs.finish);
                    glUniform1f(programs.tw_location, total_weights);
                    glDispatchCompute(programs.nb_x, programs.nb_y, 1);
                    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

                    //glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, gl_frame);
                    glGetTextureImage(programs.texture,
                        0,
                        GL_RGB,
                        GL_FLOAT,
                        gl_size,
                        gl_frame);

                    while ((err = glGetError()) != GL_NO_ERROR)
                    {
                        printf("%d\n", err);
                    }

                    for (int j = 0; j < gl_count; j++)
                        rgb_result->data[0][j] = (unsigned char)gl_frame[j];

                    sws_scale(rgb_to_yuv,
                        rgb_result->data,
                        rgb_result->linesize,
                        0,
                        in_frame->height,
                        gl_out_frame->data,
                        gl_out_frame->linesize);

                    gl_out_frame->pts = (float)last_frames_id * pts_step;

                    if (encode(&output, gl_out_frame))
                    {
                        printf("encoding error\n");
                    }

                    int tmp = nb_frame_read - oldest_read_frame_id - settings.w_count;
                    for (int i = 0; i < tmp; i++)
                    {
                        if (front(&q, &rgb))
                        {
                            printf("can't front on q\n");
                            return 1;
                        }
                        if (qremove(&q))
                        {
                            printf("can't remove on q\n");
                            return 1;
                        }
                        free_RGBFrame(rgb);
                        oldest_read_frame_id += 1;
                    }

                    last_frames_id += 1;
                    nb_thread_launched += 1;
                    nb_frame_target = -1;
                    continue;
                }

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
                    CloseHandle(tmp_handle);
                    if (front(&params, &tmp_params))
                    {
                        printf("error in thread queue\n");
                        return 14;
                    }
                    if (qremove(&params))
                    {
                        printf("error in thread queue\n");
                        return 15;
                    }
                    free(tmp_params->in_frames);
                    free(tmp_params);
                }

                if ( params.size > 0)
                {
                    if (front(&params, &tmp_params))
                    {
                        printf("error in thread queue\n");
                        return 16;
                    }
                    int tmp_id = tmp_params->in_frames[0]->id;
                    char flag = 0;
                    do
                    {
                        front(&q, &rgb);
                        if (rgb->id < tmp_id)
                        {
                            qremove(&q);
                            free_RGBFrame(rgb);
                            oldest_read_frame_id += 1;
                            flag = 1;
                        }
                        else
                        {
                            flag = 0;
                        }
                    } while (flag);
                }
                else
                {
                    int tmp = nb_frame_read - oldest_read_frame_id - settings.w_count;
                    for (int i = 0; i < tmp; i++)
                    {
                        if (front(&q, &rgb))
                        {
                            printf("can't front on q\n");
                            return 1;
                        }
                        if (qremove(&q))
                        {
                            printf("can't remove on q\n");
                            return 1;
                        }
                        free_RGBFrame(rgb);
                        oldest_read_frame_id += 1;
                    }
                }

                tmp_params = malloc(sizeof(t_param));
                if (!tmp_params)
                {
                    printf("can't alloc\n");
                    return 16;
                }
                tmp_params->frame_id = last_frames_id;
                tmp_params->nb_frame_created = &nb_frame_created;
                tmp_params->output = &output;
                tmp_params->pts = (float)last_frames_id * pts_step;

                tmp_params->nb = q.size > settings.w_count ? settings.w_count : q.size;
                tmp_params->in_frames = malloc(sizeof(RGBFrame**) * tmp_params->nb);
                if (!tmp_params->in_frames)
                {
                    printf("can't malloc\n");
                    return 1;
                }
                for (int i = 0; i < tmp_params->nb; i++)
                {
                    get(&q, q.size - 1 - i, &rgb);
                    tmp_params->in_frames[tmp_params->nb - 1 - i] = rgb;
                }
                tmp_params->weigths = &settings.weights[settings.w_count - tmp_params->nb];

                add(&params, tmp_params);

                last_frames_id += 1;

                tmp_handle = CreateThread(NULL, 0, blend_and_encode, tmp_params, 0, NULL);
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
        if (front(&params, &tmp_params))
        {
            printf("error in thread queue\n");
            return 14;
        }
        if (qremove(&params))
        {
            printf("error in thread queue\n");
            return 15;
        }
        free(tmp_params->in_frames);
        free(tmp_params);
    }

    while (q.size > 0)
    {
        if (front(&q, &rgb))
        {
            printf("");
            break;
        }
        if (qremove(&q))
        {
            printf("");
            break;
        }
        free_RGBFrame(rgb);
    }

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

//init settings
//init input output
//read first frame
//init opengl
//