#include "blending.h"

#include <stdlib.h>
#include <stdio.h>

inline void oom(uint64_t size)
{
    fprintf(stderr, "out of memory %lld\n", size);
    exit(1);
}

void* get_blend_function(int frame_type, int tmp_type)
{
    if (frame_type == 5)
    {
        if (tmp_type == 3)
            return blend_uint8_int32;
        if (tmp_type == 4)
            return blend_uint8_int64;
    }
    if (frame_type == 6)
    {
        if (tmp_type == 3)
            return blend_uint16_int32;
        if (tmp_type == 4)
            return blend_uint16_int64;
    }
    if (frame_type == 10)
        if (tmp_type == 10)
            return blend_float32_float32;
    if (frame_type == 11)
        if (tmp_type == 11)
            return blend_float64_float64;

    return 0;
}

void* get_blend_function_x86(int frame_type, int tmp_type, char* x86_flags)
{
    //todo
    return get_blend_function(frame_type, tmp_type);
}


void blend_uint8_int32(uint8_t* result, uint8_t** in_frames, int32_t* weights, uint64_t nb, uint64_t element_count)
{
    int32_t* tmp_frame = calloc(element_count, sizeof(int32_t));
    if (!tmp_frame)
        oom(element_count * sizeof(int32_t));
    
    int32_t total_weights = 0;
    for (uint64_t i = 0; i < nb; i++)
    {
        total_weights += weights[i];
    }
    int32_t tmpa, tmpb, tmpc, tmpd;
    for (uint64_t i = 0; i < element_count; i += 4)
    {
        tmpa = 0;
        tmpb = 0;
        tmpc = 0;
        tmpd = 0;
        for (uint64_t j = 0; j < nb; j++)
        {
            tmpa += weights[j] * (int32_t)(in_frames[j][i]);
            tmpb += weights[j] * (int32_t)(in_frames[j][i + 1]);
            tmpc += weights[j] * (int32_t)(in_frames[j][i + 2]);
            tmpd += weights[j] * (int32_t)(in_frames[j][i + 3]);
        }
        result[i] = tmpa / total_weights;
        result[i + 1] = tmpb / total_weights;
        result[i + 2] = tmpc / total_weights;
        result[i + 3] = tmpd / total_weights;
    }
    tmpb = element_count % 4;
    tmpc = element_count - tmpb;
    for (uint64_t i = 0; i < tmpb; i++)
    {
        tmpa = 0;
        for (uint64_t j = 0; j < nb; j++)
        {
            tmpa += weights[j] * (int32_t)in_frames[j][tmpc + i];
        }
        result[tmpc + i] = (uint8_t)tmpa / total_weights;
    }

    free(tmp_frame);
}

void blend_uint8_int64(uint8_t* result, uint8_t** in_frames, int64_t* weights, uint64_t nb, uint64_t element_count)
{
    int64_t* tmp_frame = calloc(element_count, sizeof(int64_t));
    if (!tmp_frame)
        oom(element_count * sizeof(int64_t));

    int64_t total_weights = 0;
    for (uint64_t i = 0; i < nb; i++)
    {
        total_weights += weights[i];
    }
    int64_t tmpa, tmpb, tmpc, tmpd;
    for (uint64_t i = 0; i < element_count; i += 4)
    {
        tmpa = 0;
        tmpb = 0;
        tmpc = 0;
        tmpd = 0;
        for (uint64_t j = 0; j < nb; j++)
        {
            tmpa += weights[j] * (int64_t)in_frames[j][i];
            tmpb += weights[j] * (int64_t)in_frames[j][i + 1];
            tmpc += weights[j] * (int64_t)in_frames[j][i + 2];
            tmpd += weights[j] * (int64_t)in_frames[j][i + 3];
        }
        result[i] = tmpa / total_weights;
        result[i + 1] = tmpb / total_weights;
        result[i + 2] = tmpc / total_weights;
        result[i + 3] = tmpd / total_weights;
    }
    tmpb = element_count % 4;
    tmpc = element_count - tmpb;
    for (uint64_t i = 0; i < tmpb; i++)
    {
        tmpa = 0;
        for (uint64_t j = 0; j < nb; j++)
        {
            tmpa += weights[j] * (int64_t)in_frames[j][tmpc + i];
        }
        result[tmpc + i] = (uint8_t)tmpa / total_weights;
    }

    free(tmp_frame);
}

void blend_uint16_int32(uint16_t* result, uint16_t** in_frames, int32_t* weights, uint64_t nb, uint64_t element_count)
{
    int32_t* tmp_frame = calloc(element_count, sizeof(int32_t));
    if (!tmp_frame)
        oom(element_count * sizeof(int32_t));

    int32_t total_weights = 0;
    for (uint64_t i = 0; i < nb; i++)
    {
        total_weights += weights[i];
    }

    int32_t tmpa, tmpb, tmpc, tmpd;
    for (uint64_t i = 0; i < element_count; i += 4)
    {
        tmpa = 0;
        tmpb = 0;
        tmpc = 0;
        tmpd = 0;
        for (uint64_t j = 0; j < nb; j++)
        {
            tmpa += weights[j] * (int32_t)in_frames[j][i];
            tmpb += weights[j] * (int32_t)in_frames[j][i + 1];
            tmpc += weights[j] * (int32_t)in_frames[j][i + 2];
            tmpd += weights[j] * (int32_t)in_frames[j][i + 3];
        }
        result[i] = tmpa / total_weights;
        result[i + 1] = tmpb / total_weights;
        result[i + 2] = tmpc / total_weights;
        result[i + 3] = tmpd / total_weights;
    }
    tmpb = element_count % 4;
    tmpc = element_count - tmpb;
    for (uint64_t i = 0; i < tmpb; i++)
    {
        tmpa = 0;
        for (uint64_t j = 0; j < nb; j++)
        {
            tmpa += weights[j] * (int32_t)in_frames[j][tmpc + i];
        }
        result[tmpc + i] = (uint16_t)tmpa / total_weights;
    }

    free(tmp_frame);
}

void blend_uint16_int64(uint16_t* result, uint16_t** in_frames, int64_t* weights, uint64_t nb, uint64_t element_count)
{
    int64_t* tmp_frame = calloc(element_count, sizeof(int64_t));
    if (!tmp_frame)
        oom(element_count * sizeof(int64_t));
    
    int64_t total_weights = 0;
    for (uint64_t i = 0; i < nb; i++)
    {
        total_weights += weights[i];
    }
    int64_t tmpa, tmpb, tmpc, tmpd;
    for (uint64_t i = 0; i < element_count; i += 4)
    {
        tmpa = 0;
        tmpb = 0;
        tmpc = 0;
        tmpd = 0;
        for (uint64_t j = 0; j < nb; j++)
        {
            tmpa += weights[j] * (int64_t)in_frames[j][i];
            tmpb += weights[j] * (int64_t)in_frames[j][i + 1];
            tmpc += weights[j] * (int64_t)in_frames[j][i + 2];
            tmpd += weights[j] * (int64_t)in_frames[j][i + 3];
        }
        result[i] = tmpa / total_weights;
        result[i + 1] = tmpb / total_weights;
        result[i + 2] = tmpc / total_weights;
        result[i + 3] = tmpd / total_weights;
    }
    tmpb = element_count % 4;
    tmpc = element_count - tmpb;
    for (uint64_t i = 0; i < tmpb; i++)
    {
        tmpa = 0;
        for (uint64_t j = 0; j < nb; j++)
        {
            tmpa += weights[j] * (int64_t)in_frames[j][tmpc + i];
        }
        result[tmpc + i] = (uint16_t)tmpa / total_weights;
    }

    free(tmp_frame);
}

void blend_float32_float32(fp32* result, fp32** in_frames, fp32* weights, uint64_t nb, uint64_t element_count)
{
    fp32* tmp_frame = calloc(element_count, sizeof(fp32));
    if (!tmp_frame)
        oom(element_count * sizeof(fp32));
    
    fp32 total_weights = 0;
    for (uint64_t i = 0; i < nb; i++)
        total_weights += weights[i];
    fp32 tmpa, tmpb, tmpc, tmpd, tmpe, tmpf, tmpg, tmph, tmpi, tmpj, tmpk;
    for (uint64_t i = 0; i < element_count; i+=11)
    {
        tmpa = 0;
        tmpb = 0;
        tmpc = 0;
        tmpd = 0;
        tmpe = 0;
        tmpf = 0;
        tmpg = 0;
        tmph = 0;
        tmpi = 0;
        tmpj = 0;
        tmpk = 0;
        for (uint64_t j = 0; j < nb; j++)
        {
            tmpa += weights[j] * in_frames[j][i];
            tmpb += weights[j] * in_frames[j][i + 1];
            tmpc += weights[j] * in_frames[j][i + 2];
            tmpd += weights[j] * in_frames[j][i + 3];
            tmpe += weights[j] * in_frames[j][i + 4];
            tmpf += weights[j] * in_frames[j][i + 5];
            tmpg += weights[j] * in_frames[j][i + 6];
            tmph += weights[j] * in_frames[j][i + 7];
            tmpi += weights[j] * in_frames[j][i + 8];
            tmpj += weights[j] * in_frames[j][i + 9];
            tmpk += weights[j] * in_frames[j][i + 10];
        }
        result[i] = tmpa / total_weights;
        result[i + 1] = tmpb / total_weights;
        result[i + 2] = tmpc / total_weights;
        result[i + 3] = tmpd / total_weights;
        result[i + 4] = tmpe / total_weights;
        result[i + 5] = tmpf / total_weights;
        result[i + 6] = tmpg / total_weights;
        result[i + 7] = tmph / total_weights;
        result[i + 8] = tmpi / total_weights;
        result[i + 9] = tmpj / total_weights;
        result[i + 10] = tmpk / total_weights;
    }

    free(tmp_frame);
}

void blend_float64_float64(fp64* result, fp64** in_frames, fp64* weights, uint64_t nb, uint64_t element_count)
{
    fp64* tmp_frame = calloc(element_count, sizeof(fp64));
    if (!tmp_frame)
        oom(element_count * sizeof(fp64));

    fp64 total_weights = 0;
    for (uint64_t i = 0; i < nb; i++)
        total_weights += weights[i];
    fp64 tmpa, tmpb, tmpc, tmpd, tmpe, tmpf, tmpg, tmph, tmpi, tmpj, tmpk;
    for (uint64_t i = 0; i < element_count; i += 11)
    {
        tmpa = 0;
        tmpb = 0;
        tmpc = 0;
        tmpd = 0;
        tmpe = 0;
        tmpf = 0;
        tmpg = 0;
        tmph = 0;
        tmpi = 0;
        tmpj = 0;
        tmpk = 0;
        for (uint64_t j = 0; j < nb; j++)
        {
            tmpa += weights[j] * in_frames[j][i];
            tmpb += weights[j] * in_frames[j][i + 1];
            tmpc += weights[j] * in_frames[j][i + 2];
            tmpd += weights[j] * in_frames[j][i + 3];
            tmpe += weights[j] * in_frames[j][i + 4];
            tmpf += weights[j] * in_frames[j][i + 5];
            tmpg += weights[j] * in_frames[j][i + 6];
            tmph += weights[j] * in_frames[j][i + 7];
            tmpi += weights[j] * in_frames[j][i + 8];
            tmpj += weights[j] * in_frames[j][i + 9];
            tmpk += weights[j] * in_frames[j][i + 10];
        }
        result[i] = tmpa / total_weights;
        result[i + 1] = tmpb / total_weights;
        result[i + 2] = tmpc / total_weights;
        result[i + 3] = tmpd / total_weights;
        result[i + 4] = tmpe / total_weights;
        result[i + 5] = tmpf / total_weights;
        result[i + 6] = tmpg / total_weights;
        result[i + 7] = tmph / total_weights;
        result[i + 8] = tmpi / total_weights;
        result[i + 9] = tmpj / total_weights;
        result[i + 10] = tmpk / total_weights;
    }

    free(tmp_frame);
}

#ifdef x86_64

void blend_uint8_int32_avx2(uint8_t* result, uint8_t** in_frames, int32_t* weights, uint64_t nb, uint64_t element_count)
{
    int32_t* tmp_frame = calloc(element_count, sizeof(int32_t));
    if (!tmp_frame)
        oom(element_count * sizeof(int32_t));

    blend_int8_int32_avx2_asm(result, in_frames, weights, nb, element_count, tmp_frame);

    free(tmp_frame);
}

void blend_float32_float32_avx(fp32* result, fp32** in_frames, fp32* weights, uint64_t nb, uint64_t element_count)
{
    fp32* tmp_frame = calloc(element_count, sizeof(fp32));
    if (!tmp_frame)
        oom(element_count * sizeof(fp32));

    blend_float32_float32_avx_asm(result, in_frames, weights, nb, element_count, tmp_frame);

    free(tmp_frame);
}

void blend_float32_float32_fma3avx(fp32* result, fp32** in_frames, fp32* weights, uint64_t nb, uint64_t element_count)
{
    fp32* tmp_frame = calloc(element_count, sizeof(fp32));
    if (!tmp_frame)
        oom(element_count * sizeof(fp32));

    blend_float32_float32_fma3avx_asm(result, in_frames, weights, nb, element_count, tmp_frame);

    free(tmp_frame);
}

#endif