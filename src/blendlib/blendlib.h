#pragma once

#include <stdint.h>

#include "BlendSettings.h"
#include "RGBFrame.h"

#include "utils/Thread.h"
#include "utils/Queue.h"
#include "utils/Stock.h"
#include "utils/Mutex.h"

typedef struct BlendThreadContext
{
	//input
	void* blend_funct;
	uint64_t element_count;
	RGBFrame** input_frames;
	void* weights;
	uint64_t count;

	//output
	RGBFrame* result_frame;
	char exit;
} BlendThreadContext;

typedef struct BlendContext
{
	//settings
	BlendSettings* settings;

	//work variables
	Queue blend_threads;
	Stock blend_ctxs;
	Stock input_frames;
	uint64_t nb_frame_created;
	uint64_t nb_blend_launched; // frames created + frames in creation (thread)
	uint64_t total_input_frames; 
	uint64_t frame_size;
	Queue created_frames;
	Mutex output_frames_mutex;

	int64_t frame_begin;
	int64_t frame_end;
	int64_t weight_offset;

} BlendContext;

char init_blending(BlendContext** blend_ctx, BlendSettings* settings);

// after exiting add_frame, there is no guaranty that a frame is available in output
// block if maximum thread count is hitten
// if return true, it is a fatal error
char add_frame(BlendContext* blend_ctx, void* frame_data);

//flush the blending
char finish_blending(BlendContext* blend_ctx);

//must be called after finish_blending
void free_blending(BlendContext* blend_ctx);

//not synchronized
int get_output_frame_count(BlendContext* blend_ctx);

char acquire_output_frames(BlendContext* blend_ctx);

//must be called after acquire_output_frames
void* get_output_frame(BlendContext* blend_ctx);

char release_out_frames(BlendContext* blend_ctx);

void blend(BlendThreadContext* ctx);

void get_interval(int64_t* begin, int64_t* end, uint16_t input_num, uint16_t input_den, uint64_t input_index, uint16_t output_num, uint16_t output_den, uint64_t output_index, uint64_t range);