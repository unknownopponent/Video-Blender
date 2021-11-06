#include "blendlib.h"

#include "../utils/Error.h"

char init_blending(BlendContext** blend_ctx, BlendSettings* settings)
{
	if (!settings->blend_funct)
	{
		fprintf(stderr, "no blend funct given\n");
		return 1;
	}
	if (settings->max_blend_threads == 0)
		settings->max_blend_threads = 1;
	if (!settings->width)
	{
		fprintf(stderr, "no width given\n");
		return 1;
	}
	if (!settings->height)
	{
		fprintf(stderr, "no height given\n");
		return 1;
	}
	if (settings->frame_type >= FTYPES_COUNT)
	{
		fprintf(stderr, "no input frame type given\n");
		return 1;
	}
	if (!settings->weights)
	{
		fprintf(stderr, "no weights given\n");
		return 1;
	}
	if (!settings->weights_c)
	{
		fprintf(stderr, "no weigth count given\n");
		return 1;
	}
	if (!settings->iden)
	{
		fprintf(stderr, "no input timebase den given\n");
		return 1;
	}
	if (!settings->inum)
	{
		fprintf(stderr, "no input timebase num given\n");
		return 1;
	}
	if (!settings->onum)
	{
		fprintf(stderr, "no output timebase num given\n");
		return 1;
	}
	if (!settings->oden)
	{
		fprintf(stderr, "no output timebase den given\n");
		return 1;
	}

	//todo check function frame type

	BlendContext* ctx = malloc(sizeof(BlendContext));
	if (!ctx)
	{
		oom();
	}
	
	ctx->settings = malloc(sizeof(BlendSettings));
	if (!ctx->settings)
	{
		oom();
	}
	memcpy(ctx->settings, settings, sizeof(BlendSettings));
	ctx->settings->weights = malloc(settings->internal_size * settings->weights_c);
	if (!ctx->settings->weights)
	{
		oom();
	}
	memcpy(ctx->settings->weights, settings->weights, settings->internal_size * settings->weights_c);

	init_queue(&ctx->blend_threads, sizeof(Thread*));

	sinit(&ctx->input_frames, sizeof(RGBFrame*));
	sinit(&ctx->blend_ctxs, sizeof(BlendThreadContext*));
	ctx->nb_frame_created = 0;
	ctx->nb_blend_launched = 0;
	ctx->total_input_frames = 0;
	ctx->frame_size = settings->width * settings->height * 3 * type_sizes[settings->frame_type];
	init_queue(&ctx->created_frames, sizeof(RGBFrame*));
	if (create_mutex(&ctx->output_frames_mutex))
	{
		fprintf(stderr, "can't create mutex\n");
		return 1;
	}

	ctx->frame_begin = 0;
	ctx->frame_end = 0;
	ctx->weight_offset = 0;

	*blend_ctx = ctx;

	return 0;
}

char add_frame(BlendContext* blend_ctx, void* frame_data)
{
	Thread* tmp_thread;
	BlendThreadContext* tmp_ctx;
	RGBFrame* tmp_frame;

	if (blend_ctx->blend_threads.size)
	{
		if (front(&blend_ctx->blend_threads, &tmp_thread))
		{
			fprintf(stderr, "queue front error\n");
			return 1;
		}
		tmp_ctx = tmp_thread->params;
		uint64_t last_id = tmp_ctx->input_frames[0]->id;

		for (int i = 0; i < blend_ctx->input_frames.availble.size; i++)
		{
			if (get(&blend_ctx->input_frames.elements, i, &tmp_frame))
			{
				fprintf(stderr, "queue get error\n");
				return 1;
			}
			if (tmp_frame->id < last_id)
			{
				if (sset(&blend_ctx->input_frames, i, 0))
				{
					fprintf(stderr, "stock set error\n");
					return 1;
				}
			}
		}
	}
	while (1)
	{
		if (!blend_ctx->blend_threads.size)
			break;
		if (front(&blend_ctx->blend_threads, &tmp_thread))
		{
			fprintf(stderr, "queue front error\n");
			return 1;
		}
		if (blend_ctx->blend_threads.size >= blend_ctx->settings->max_blend_threads)
			join_thread(tmp_thread);
		else if (!((BlendThreadContext*)tmp_thread->params)->exit)
			break;
		qremove(&blend_ctx->blend_threads);
		tmp_ctx = tmp_thread->params;
		free(tmp_thread);
		lock_mutex(&blend_ctx->output_frames_mutex);
		if (add(&blend_ctx->created_frames, tmp_ctx->result_frame))
		{
			fprintf(stderr, "queue add error\n");
			return 1;
		}
		unlock_mutex(&blend_ctx->output_frames_mutex);
		blend_ctx->nb_frame_created += 1;
		BlendThreadContext* tmp_ctx2;
		int index = -1;
		for (int i = 0; i < blend_ctx->blend_ctxs.availble.size; i++)
		{
			if (get(&blend_ctx->blend_ctxs.elements, i, &tmp_ctx2))
			{
				fprintf(stderr, "queue get error\n");
				return 1;
			}
			if (tmp_ctx == tmp_ctx2)
			{
				index = i;
				break;
			}
		}
		if (index == -1)
		{
			fprintf(stderr, "reference not found\n");
			return 1;
		}
		if (sset(&blend_ctx->blend_ctxs, index, 0))
		{
			fprintf(stderr, "stock set error\n");
			return 1;
		}
	}
	
	int index;
	if (sget_available(&blend_ctx->input_frames, &tmp_frame, &index))
	{
		if (alloc_RGBFrame(&tmp_frame, blend_ctx->total_input_frames, blend_ctx->settings->width, blend_ctx->settings->height, type_sizes[blend_ctx->settings->frame_type]))
		{
			oom();
		}
		if (sadd(&blend_ctx->input_frames, tmp_frame))
		{
			fprintf(stderr, "stock add error\n");
			return 1;
		}
	}
	else
	{
		if (sset(&blend_ctx->input_frames, index, 1))
		{
			fprintf(stderr, "stock set error\n");
			return 1;
		}
	}

	tmp_frame->id = blend_ctx->total_input_frames;
	memcpy(tmp_frame->frame_data, frame_data, blend_ctx->frame_size);

	blend_ctx->total_input_frames += 1;

	if (!blend_ctx->frame_end)
	{
		get_interval(
			&blend_ctx->frame_begin,
			&blend_ctx->frame_end,
			blend_ctx->settings->inum,
			blend_ctx->settings->iden,
			blend_ctx->total_input_frames,
			blend_ctx->settings->onum,
			blend_ctx->settings->oden,
			blend_ctx->nb_blend_launched,
			blend_ctx->settings->weights_c
		);

		if (!blend_ctx->frame_end)
			return 0;

		if (blend_ctx->frame_begin < 0)
		{
			blend_ctx->weight_offset = -blend_ctx->frame_begin;
			blend_ctx->frame_begin = 0;
		}
	}

	if (blend_ctx->frame_end < blend_ctx->total_input_frames)
	{
		if (sget_available(&blend_ctx->blend_ctxs, &tmp_ctx, &index))
		{
			tmp_ctx = calloc(1, sizeof(BlendThreadContext));
			if (!tmp_ctx)
			{
				oom();
			}
			tmp_ctx->input_frames = malloc(sizeof(RGBFrame*) * blend_ctx->settings->weights_c);
			tmp_ctx->weights = malloc(blend_ctx->settings->weights_c * blend_ctx->settings->internal_size);
			if (!tmp_ctx->weights || !tmp_ctx->input_frames)
			{
				oom();
			}
			if (sadd(&blend_ctx->blend_ctxs, tmp_ctx))
			{
				fprintf(stderr, "stock add error\n");
				return 1;
			}
		}
		else
		{
			if (sset(&blend_ctx->blend_ctxs, index, 1))
			{
				fprintf(stderr, "stock set error\n");
				return 1;
			}
		}

		tmp_ctx->blend_funct = blend_ctx->settings->blend_funct;
		tmp_ctx->element_count = blend_ctx->settings->width * blend_ctx->settings->height * 3;
		tmp_ctx->frame_type = blend_ctx->settings->frame_type;
		tmp_ctx->count = blend_ctx->frame_end - blend_ctx->frame_begin + 1 - blend_ctx->weight_offset;
		tmp_ctx->exit = 0;

		char flag;
		for (int i = blend_ctx->frame_begin; i <= blend_ctx->frame_end; i++)
		{
			flag = 0;
			for (int j = 0; j < blend_ctx->input_frames.elements.size; j++)
			{
				if (get(&blend_ctx->input_frames.elements, j, &tmp_frame))
				{
					fprintf(stderr, "queue get error\n");
					return 1;
				}
				if (tmp_frame->id == i)
				{
					tmp_ctx->input_frames[i - blend_ctx->frame_begin] = tmp_frame;
					flag = 1;
					break;
				}
			}
			if (!flag)
			{
				fprintf(stderr, "frame not found\n");
				return 1;
			}
		}
		
		memcpy(tmp_ctx->weights, (uint8_t*)blend_ctx->settings->weights + blend_ctx->weight_offset * blend_ctx->settings->internal_size, blend_ctx->settings->internal_size * tmp_ctx->count);
		if (alloc_RGBFrame(&tmp_ctx->result_frame, 0, blend_ctx->settings->width, blend_ctx->settings->height, type_sizes[blend_ctx->settings->frame_type]))
		{
			oom();
		}

		tmp_thread = malloc(sizeof(Thread));
		if (!tmp_thread)
		{
			oom();
		}

		tmp_thread->function = blend;
		tmp_thread->params = tmp_ctx;

		if (launch_thread(tmp_thread))
		{
			fprintf(stderr, "thread launch error\n");
			return 1;
		}

		if (add(&blend_ctx->blend_threads, tmp_thread))
		{
			fprintf(stderr, "queue add error\n");
			return 1;
		}

		blend_ctx->nb_blend_launched += 1;
		blend_ctx->frame_begin = 0;
		blend_ctx->frame_end = 0;
		blend_ctx->weight_offset = 0;
	}

	return 0;
}

//flush
// loop launch last threads
// wait free threads

char finish_blending(BlendContext* blend_ctx)
{
	uint64_t simulated_input = blend_ctx->total_input_frames;
	
	Thread* tmp_thread;
	BlendThreadContext* tmp_ctx;
	int index;
	RGBFrame* tmp_frame;

	while (1)
	{
		simulated_input += 1;

		get_interval(
			&blend_ctx->frame_begin,
			&blend_ctx->frame_end,
			blend_ctx->settings->inum,
			blend_ctx->settings->iden,
			simulated_input,
			blend_ctx->settings->onum,
			blend_ctx->settings->oden,
			blend_ctx->nb_blend_launched,
			blend_ctx->settings->weights_c
		);

		if (!blend_ctx->frame_end)
			continue;

		blend_ctx->frame_end = blend_ctx->total_input_frames -1;

		if (blend_ctx->frame_begin >= blend_ctx->frame_end)
			break;

		if (sget_available(&blend_ctx->blend_ctxs, &tmp_ctx, &index))
		{
			tmp_ctx = calloc(1, sizeof(BlendThreadContext));
			if (!tmp_ctx)
			{
				oom();
			}
			tmp_ctx->input_frames = malloc(sizeof(RGBFrame*) * blend_ctx->settings->weights_c);
			tmp_ctx->weights = malloc(blend_ctx->settings->weights_c * blend_ctx->settings->internal_size);
			if (!tmp_ctx->weights || !tmp_ctx->input_frames)
			{
				oom();
			}
			if (sadd(&blend_ctx->blend_ctxs, tmp_ctx))
			{
				fprintf(stderr, "stock add error\n");
				return 1;
			}
		}
		else
		{
			if (sset(&blend_ctx->blend_ctxs, index, 1))
			{
				fprintf(stderr, "stock set error\n");
				return 1;
			}
		}

		tmp_ctx->blend_funct = blend_ctx->settings->blend_funct;
		tmp_ctx->element_count = blend_ctx->settings->width * blend_ctx->settings->height * 3;
		tmp_ctx->frame_type = blend_ctx->settings->frame_type;
		tmp_ctx->count = blend_ctx->frame_end - blend_ctx->frame_begin + 1;

		char flag;
		for (int i = blend_ctx->frame_begin; i <= blend_ctx->frame_end; i++)
		{
			flag = 0;
			for (int j = 0; j < blend_ctx->input_frames.elements.size; j++)
			{
				if (get(&blend_ctx->input_frames.elements, j, &tmp_frame))
				{
					fprintf(stderr, "queue get error\n");
					return 1;
				}
				if (tmp_frame->id == i)
				{
					tmp_ctx->input_frames[i - blend_ctx->frame_begin] = tmp_frame;
					flag = 1;
					break;
				}
			}
			if (!flag)
			{
				fprintf(stderr, "frame not found\n");
				return 1;
			}
		}

		memcpy(tmp_ctx->weights, (uint8_t*)blend_ctx->settings->weights + (blend_ctx->settings->weights_c - tmp_ctx->count) * blend_ctx->settings->internal_size, blend_ctx->settings->internal_size * tmp_ctx->count);
		if (alloc_RGBFrame(&tmp_ctx->result_frame, 0, blend_ctx->settings->width, blend_ctx->settings->height, type_sizes[blend_ctx->settings->frame_type]))
		{
			oom();
		}
		tmp_ctx->count -= blend_ctx->weight_offset;

		tmp_thread = malloc(sizeof(Thread));
		if (!tmp_thread)
		{
			oom();
		}

		tmp_thread->function = blend;
		tmp_thread->params = tmp_ctx;

		if (launch_thread(tmp_thread))
		{
			fprintf(stderr, "thread launch error\n");
			return 1;
		}

		if (add(&blend_ctx->blend_threads, tmp_thread))
		{
			fprintf(stderr, "queue add error\n");
			return 1;
		}

		blend_ctx->nb_blend_launched += 1;
		blend_ctx->frame_begin = 0;
		blend_ctx->frame_end = 0;
	}

	while (blend_ctx->blend_threads.size)
	{
		front(&blend_ctx->blend_threads, &tmp_thread);
		qremove(&blend_ctx->blend_threads);
		join_thread(tmp_thread);
		tmp_ctx = tmp_thread->params;
		free(tmp_thread);
		lock_mutex(&blend_ctx->output_frames_mutex);
		if (add(&blend_ctx->created_frames, tmp_ctx->result_frame))
		{
			printf("");
			return 1;
		}
		unlock_mutex(&blend_ctx->output_frames_mutex);
		blend_ctx->nb_frame_created += 1;
	}
	
	return 0;
}

void free_blending(BlendContext* blend_ctx)
{
	if (blend_ctx->settings)
	{
		if (blend_ctx->settings->weights)
			free(blend_ctx->settings->weights);
		free(blend_ctx->settings);
	}

	Thread* tmp_thread;
	while (blend_ctx->blend_threads.size)
	{
		if (front(&blend_ctx->blend_threads, &tmp_thread))
		{
			fprintf(stderr, "queue front error\n");
			return 1;
		}
		if (qremove(&blend_ctx->blend_threads))
		{
			fprintf(stderr, "queue remove error\n");
			return 1;
		}
		join_thread(tmp_thread);
		free(tmp_thread);
	}
	
	BlendThreadContext* tmp_ctx;
	while (blend_ctx->blend_ctxs.elements.size)
	{
		if (sget(&blend_ctx->blend_ctxs, 0, &tmp_ctx))
		{
			fprintf(stderr, "stock get error\n");
			return 1;
		}
		free(tmp_ctx->input_frames);
		free(tmp_ctx->weights);
		free(tmp_ctx);
		sremove(&blend_ctx->blend_ctxs);
	}

	RGBFrame* tmp_frame;
	while (blend_ctx->input_frames.availble.size)
	{
		sget(&blend_ctx->input_frames, 0, &tmp_frame);
		free_RGBFrame(tmp_frame);
		sremove(&blend_ctx->input_frames);
	}
	while (blend_ctx->created_frames.size)
	{
		get(&blend_ctx->created_frames, 0, &tmp_frame);
		free_RGBFrame(tmp_frame);
		qremove(&blend_ctx->created_frames);
	}
	delete_mutex(&blend_ctx->output_frames_mutex);
	free(blend_ctx);
}

int get_output_frame_count(BlendContext* blend_ctx)
{
	return blend_ctx->created_frames.size;
}

char acquire_output_frames(BlendContext* blend_ctx)
{
	return lock_mutex(&blend_ctx->output_frames_mutex);
}

void* get_output_frame(BlendContext* blend_ctx)
{
	if (blend_ctx->created_frames.size < 1)
		return 0;
	RGBFrame* tmp;
	if (front(&blend_ctx->created_frames, &tmp))
		return 0;
	qremove(&blend_ctx->created_frames);
	void* tmp2 = tmp->frame_data;
	free(tmp);
	return tmp2;
}

char release_out_frames(BlendContext* blend_ctx)
{
	return unlock_mutex(&blend_ctx->output_frames_mutex);
}

void blend(BlendThreadContext* ctx)
{
	void (*funct)(void*, void*, void*, uint64_t, uint64_t) = ctx->blend_funct;

	void** in_frames = malloc(ctx->count * sizeof(void*));
	if (!in_frames)
	{
		oom();
	}
	for (int i = 0; i < ctx->count; i++)
	{
		in_frames[i] = ctx->input_frames[i]->frame_data;
	}

	funct(ctx->result_frame->frame_data, in_frames, ctx->weights, ctx->count, ctx->element_count);

	free(in_frames);
	ctx->exit = 1;
}

void get_interval(int64_t* begin, int64_t* end, uint16_t input_num, uint16_t input_den, uint64_t input_index, uint16_t output_num, uint16_t output_den, uint64_t output_index, uint64_t range)
{
	float tmp = (float)input_num / (float)input_den * (float)input_index;
	float tmp2 = (float)output_num / (float)output_den * (float)output_index;
	float tmp3 = (float)input_num / (float)input_den * (float)(input_index - 1);

	if (tmp > tmp2)
	{
		if (range % 2)
		{
			if (tmp2 - tmp < tmp - tmp3)
			{
				*begin = input_index - range / 2;
				*end = input_index + range / 2;
			}
			else
			{
				*begin = input_index - 1 - range / 2;
				*end = input_index - 1 + range / 2;
			}
		}
		else
		{
			if (tmp2 - tmp < tmp - tmp3)
			{
				*begin = input_index - range / 2;
				*end = input_index - 1 + range / 2;
			}
			else
			{
				*begin = input_index - 1 - range / 2;
				*end = input_index + range / 2;
			}
		}
	}
}