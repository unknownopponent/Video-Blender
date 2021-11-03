#include "blending.h"

#include "../utils/Error.h"

void blend_int8_int32(uint8_t* result, uint8_t** in_frames, int32_t* weights, uint64_t nb, uint64_t element_count)
{
    int32_t* tmp_frame = calloc(element_count, sizeof(int32_t));
    if (!tmp_frame)
    {
        oom();
    }
    
    int32_t total_weights = 0;
    for (uint64_t i = 0; i < nb; i++)
    {
        if (weights[i] == 0)
            continue;
        total_weights += weights[i];

        for (uint64_t j = 0; j < element_count; j++)
        {
            tmp_frame[j] += (int32_t)in_frames[i][j] * weights[i];
        }
    }

    int32_t tmp = 0;
    for (uint64_t i = 0; i < element_count; i++)
    {
        tmp = tmp_frame[i] % total_weights;
        result[i] = (uint8_t)(tmp_frame[i] / total_weights);
        if (tmp)
        {
            if (tmp * 2 >= total_weights)
            {
                result[i] += 1;
            }
        }
    }

    free(tmp_frame);
}

void blend_int8_int64(uint8_t* result, uint8_t** in_frames, int64_t* weights, uint64_t nb, uint64_t element_count)
{
    int64_t* tmp_frame = calloc(element_count, sizeof(int64_t));
    if (!tmp_frame)
    {
        oom();
    }

    int64_t total_weights = 0;
    for (uint64_t i = 0; i < nb; i++)
    {
        if (weights[i] == 0)
            continue;
        total_weights += weights[i];

        for (uint64_t j = 0; j < element_count; j++)
        {
            tmp_frame[j] += (int64_t)in_frames[i][j] * weights[i];
        }
    }

    int64_t tmp = 0;
    for (uint64_t i = 0; i < element_count; i++)
    {
        tmp = tmp_frame[i] % total_weights;
        result[i] = (uint8_t)(tmp_frame[i] / total_weights);
        if (tmp)
        {
            if (tmp * 2 >= total_weights)
            {
                result[i] += 1;
            }
        }
    }

    free(tmp_frame);
}

void blend_int16_int32(uint16_t* result, uint16_t** in_frames, int32_t* weights, uint64_t nb, uint64_t element_count)
{
    int32_t* tmp_frame = calloc(element_count, sizeof(int32_t));
    if (!tmp_frame)
    {
        oom();
    }

    int32_t total_weights = 0;
    for (uint64_t i = 0; i < nb; i++)
    {
        if (weights[i] == 0)
            continue;
        total_weights += weights[i];

        for (uint64_t j = 0; j < element_count; j++)
        {
            tmp_frame[j] += (int32_t)in_frames[i][j] * weights[i];
        }
    }

    int32_t tmp = 0;
    for (uint64_t i = 0; i < element_count; i++)
    {
        tmp = tmp_frame[i] % total_weights;
        result[i] = (uint16_t)(tmp_frame[i] / total_weights);
        if (tmp)
        {
            if (tmp * 2 >= total_weights)
            {
                result[i] += 1;
            }
        }
    }

    free(tmp_frame);
}

void blend_int16_int64(uint16_t* result, uint16_t** in_frames, int64_t* weights, uint64_t nb, uint64_t element_count)
{
    int64_t* tmp_frame = calloc(element_count, sizeof(int64_t));
    if (!tmp_frame)
    {
        oom();
    }
    
    int64_t total_weights = 0;
    for (uint64_t i = 0; i < nb; i++)
    {
        if (weights[i] == 0)
            continue;
        total_weights += weights[i];

        for (uint64_t j = 0; j < element_count; j++)
        {
            tmp_frame[j] += (int64_t)in_frames[i][j] * weights[i];
        }
    }

    int64_t tmp = 0;
    for (uint64_t i = 0; i < element_count; i++)
    {
        tmp = tmp_frame[i] % total_weights;
        result[i] = (uint16_t)(tmp_frame[i] / total_weights);
        if (tmp)
        {
            if (tmp * 2 >= total_weights)
            {
                result[i] += 1;
            }
        }
    }

    free(tmp_frame);
}

void blend_float32_float32(fp32* result, fp32** in_frames, fp32* weights, uint64_t nb, uint64_t element_count)
{
    fp32* tmp_frame = calloc(element_count, sizeof(fp32));
    if (!tmp_frame)
    {
        oom();
    }

    fp32 total_weights = 0.0f;
    for (uint64_t i = 0; i < nb; i++)
    {
        if (weights[i] == 0.0f)
            continue;
        total_weights += weights[i];

        for (uint64_t j = 0; j < element_count; j++)
        {
            tmp_frame[j] += in_frames[i][j] * weights[i];
        }
    }

    for (uint64_t i = 0; i < element_count; i++)
    {
        result[i] = tmp_frame[i] / total_weights;
    }

    free(tmp_frame);
}

void blend_float64_float64(fp64* result, fp64** in_frames, fp64* weights, uint64_t nb, uint64_t element_count)
{
    fp64* tmp_frame = calloc(element_count, sizeof(fp64));
    if (!tmp_frame)
    {
        oom();
    }

    fp64 total_weights = 0.0f;
    for (uint64_t i = 0; i < nb; i++)
    {
        if (weights[i] == 0.0f)
            continue;
        total_weights += weights[i];

        for (uint64_t j = 0; j < element_count; j++)
        {
            tmp_frame[j] += in_frames[i][j] * weights[i];
        }
    }

    for (uint64_t i = 0; i < element_count; i++)
    {
        result[i] = tmp_frame[i] / total_weights;
    }

    free(tmp_frame);
}