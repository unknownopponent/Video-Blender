#include "vblender.h"

#include <libswscale/swscale.h>

#include "utils/Error.h"
#include "utils/Utils.h"

#include "blendlib/convert.h"
#include "blendlib/blending.h"

char vblend_parse(char** args, int argc, VBlenderSettings* vsettings, BlendSettings* bsettings, char*** input_files, int* input_files_count, char** output_folder)
{
	int index = 0;
	
	int tmpa = 0;
	int tmpb = 0;

	char weight_type = 0;

	for (int i = 0; i < argc; i++)
	{
		if (args[i][0] != '-')
		{
			fprintf(stderr, "%s is not an argument\n", args[i]);
			return 1;
		}
		index = 1;
		if (args[i][index] == 'i' && strlen(args[i]) == 2)
		{
			tmpa = 1;
			while (i + tmpa != argc && args[i + tmpa][0] != '-')
			{
				tmpa += 1;
			}
			if (tmpa == 1)
			{
				fprintf(stderr, "no input file given %s %s\n", args[i], args[i + 1]);
				return 1;
			}
			tmpa -= 1;
			i += 1;
			*input_files = malloc(sizeof(char*) * tmpa);
			if (!*input_files)
				oom(tmpa);
			for (int j = 0; j < tmpa; j++)
			{
				(*input_files)[j] = malloc(strlen(args[i + j]) +1);
				if (!(*input_files)[j])
					oom(strlen(args[i + j]) + 1);
				strcpy((*input_files)[j], args[i + j]);
			}
			*input_files_count = tmpa;
			i += tmpa -1;
		}
		else if (args[i][index] == 'o' && strlen(args[i]) == 2)
		{
			if (args[i + 1][0] == '-')
			{
				fprintf(stderr, "no output file given %s %s\n", args[i], args[i + 1]);
				return 1;
			}
			i += 1;
			*output_folder = malloc(strlen(args[i]) + 1);
			if (!*output_folder)
				oom(strlen(args[i]) + 1);
			strcpy(*output_folder, args[i]);
		}
		else if (!strcmp(args[i], "-dec"))
		{
			if (args[i + 1][0] == '-')
			{
				fprintf(stderr, "no decoder given %s %s\n", args[i], args[i + 1]);
				return 1;
			}
			i += 1;
			vsettings->decoder = malloc(strlen(args[i]) +1);
			if (!vsettings->decoder)
				oom(strlen(args[i]) + 1);
			strcpy(vsettings->decoder, args[i]);
		}
		else if (!strcmp(args[i], "-enc"))
		{
			if (args[i + 1][0] == '-')
			{
				fprintf(stderr, "no encoder given %s %s\n", args[i], args[i + 1]);
				return 1;
			}
			i += 1;
			vsettings->encoder = malloc(strlen(args[i]) + 1);
			if (!vsettings->encoder)
				oom(strlen(args[i]) + 1);
			strcpy(vsettings->encoder, args[i]);
		}
		else if (!strcmp(args[i], "-enc_opt"))
		{
			if (args[i + 1][0] == '-')
			{
				fprintf(stderr, "no encoder given %s %s\n", args[i], args[i + 1]);
				return 1;
			}
			i += 1;
			tmpa = 0;
			while (i + tmpa != argc && args[i + tmpa][0] != '-')
			{
				tmpa += 1;
			}
			vsettings->encoder_options = malloc(sizeof(char*) * tmpa * 2);
			if (!vsettings->encoder_options)
				oom(tmpa * 2);
			for (int j = 0; j < tmpa; j++)
			{
				tmpb = 0;
				while (args[i + j][tmpb] != '=')
				{
					if (!args[i + j][tmpb])
					{
						fprintf(stderr, "invalid encoder option format %s\n", args[i + j]);
						return 1;
					}
					tmpb += 1;
				}
				vsettings->encoder_options[j * 2] = malloc(tmpb + 1);
				if (!vsettings->encoder_options[j * 2])
					oom(tmpb + 1);
				memcpy(vsettings->encoder_options[j * 2], args[i + j], tmpb);
				vsettings->encoder_options[j * 2][tmpb] = 0;
				int tmp_len = strlen(args[i + j]) - tmpb;
				vsettings->encoder_options[j * 2 + 1] = malloc(tmp_len);
				if (!vsettings->encoder_options[j * 2 + 1])
					oom(tmp_len);
				memcpy(vsettings->encoder_options[j * 2 +1], args[i + j] + tmpb +1, tmp_len);
			}
			vsettings->encoder_options_count = tmpa;
			i += tmpa - 1;
		}
		else if (!strcmp(args[i], "-fps"))
		{
			if (args[i + 1][0] == '-')
			{
				fprintf(stderr, "no framerate given %s %s\n", args[i], args[i + 1]);
				return 1;
			}
			i += 1;
			if (sscanf(args[i], "%d", &tmpa) != 1)
			{
				fprintf(stderr, "failled to read framerate %s %s\n", args[i], args[i + 1]);
				return 1;
			}
			vsettings->num = 1;
			vsettings->den = tmpa;
		}
		else if (!strcmp(args[i], "-timebase"))
		{
			if (args[i + 1][0] == '-')
			{
				fprintf(stderr, "no timebase given %s %s\n", args[i], args[i + 1]);
				return 1;
			}
			i += 1;
			if (sscanf(args[i], "%d/%d", &tmpa, &tmpb) != 2)
			{
				fprintf(stderr, "failled to read timebase %s %s\n", args[i], args[i + 1]);
				return 1;
			}
			vsettings->num = tmpa;
			vsettings->den = tmpb;
		}
		else if (!strcmp(args[i], "-interbits"))
		{
			if (args[i + 1][0] == '-')
			{
				fprintf(stderr, "no internal data bits given %s %s\n", args[i], args[i + 1]);
				return 1;
			}
			i += 1;
			if (sscanf(args[i], "%d", &tmpa) != 1)
			{
				fprintf(stderr, "failled to read internal data bits %s %s\n", args[i], args[i + 1]);
				return 1;
			}
			if (tmpa != 32 && tmpa != 64)
			{
				fprintf(stderr, "invalid internal data bits, possible values : 32, 64\n");
				return 1;
			}
			vsettings->internal_data_bits = tmpa;
		}
		else if (!strcmp(args[i], "-blendthreads"))
		{
			if (args[i + 1][0] == '-')
			{
				fprintf(stderr, "no blend thread count given %s %s\n", args[i], args[i + 1]);
				return 1;
			}
			i += 1;
			if (sscanf(args[i], "%d", &tmpa) != 1)
			{
				fprintf(stderr, "failled to read blend thread count %s %s\n", args[i], args[i + 1]);
				return 1;
			}
			bsettings->max_blend_threads = tmpa;
		}
		else if (!strcmp(args[i], "-weights"))
		{
			if (args[i + 1][0] == '-')
			{
				fprintf(stderr, "no weights given %s %s\n", args[i], args[i + 1]);
				return 1;
			}
			i += 1;
			tmpa = 0;
			while (i + tmpa != argc && args[i + tmpa][0] != '-')
				tmpa += 1;

			if (tmpa == 1)
			{
				if (sscanf(args[i], "%d", &tmpa) != 1)
				{
					fprintf(stderr, "failled to read weight count %s %s\n", args[i -1], args[i]);
					return 1;
				}
				bsettings->weights_c = tmpa;
				vsettings->internal_floating = 0;
			}
			else
			{
				if (strchr(args[i], '.'))
				{
					weight_type = FFLOAT64;
					double* tmpd = malloc(sizeof(double) * tmpa);
					if (!tmpd)
						oom(sizeof(double) * tmpa);
					char* tmpc = 0;
					for (int j = 0; j < tmpa; j++)
					{
						tmpd[j] = strtod(args[i + j], &tmpc);
					}
					bsettings->weights = tmpd;
					vsettings->internal_floating = 1;
				}
				else
				{
					weight_type = FUINT64;
					int64_t* tmpl = malloc(sizeof(int64_t) * tmpa);
					if (!tmpl)
						oom(sizeof(int64_t) * tmpa);
					char* tmpc = 0;
					for (int j = 0; j < tmpa; j++)
					{
						tmpl[j] = strtoll(args[i + j], &tmpc, 10);
					}
					bsettings->weights = tmpl;
					vsettings->internal_floating = 0;
				}
				bsettings->weights_c = tmpa;
				i += tmpa -1;
			}
		}
		else
		{
			fprintf(stderr, "unknown arg %s\n", args[i]);
			return 1;
		}
	}

	//default values

	if (!*input_files)
	{
		fprintf(stderr, "no input file\n");
		return 1;
	}
	if (!*output_folder)
	{
		*output_folder = malloc(9);
		if (!*output_folder)
			oom(9);
		memcpy(*output_folder, "./output", 9);
	}
	if (!vsettings->encoder)
	{
		vsettings->encoder = malloc(8);
		if (!vsettings->encoder)
			oom(8);
		memcpy(vsettings->encoder, "libx264", 8);
		vsettings->encoder_options = malloc(sizeof(char*) * 4);
		if (!vsettings->encoder_options)
			oom(sizeof(char*) * 4);
		vsettings->encoder_options[0] = malloc(7);
		if (!vsettings->encoder_options[0])
			oom(7);
		memcpy(vsettings->encoder_options[0], "preset", 7);
		vsettings->encoder_options[1] = malloc(10);
		if (!vsettings->encoder_options[1])
			oom(10);
		memcpy(vsettings->encoder_options[1], "ultrafast", 10);
		vsettings->encoder_options[2] = malloc(4);
		if (!vsettings->encoder_options[2])
			oom(4);
		memcpy(vsettings->encoder_options[2], "crf", 4);
		vsettings->encoder_options[3] = malloc(2);
		if (!vsettings->encoder_options[3])
			oom(2);
		memcpy(vsettings->encoder_options[3], "0", 2);
		vsettings->encoder_options_count = 2;
	}
	if (!vsettings->num)
	{
		vsettings->num = 1;
	}
	if (!vsettings->den)
	{
		vsettings->den = 60;
	}
	if (!vsettings->internal_data_bits)
	{
		vsettings->internal_data_bits = 32;
	}
	if (!bsettings->max_blend_threads)
	{
		bsettings->max_blend_threads = get_available_threads();
	}
	
	//auto settings

	if (bsettings->weights)
	{
		if (vsettings->internal_data_bits == 32)
		{
			if (weight_type == FFLOAT64)
			{
				float* tmpf = malloc(sizeof(float) * bsettings->weights_c);
				if (!tmpf)
					oom(sizeof(float) * bsettings->weights_c);
				for (int i = 0; i < bsettings->weights_c; i++)
				{
					tmpf[i] = (float)(((double*)bsettings->weights)[i]);
				}
				free(bsettings->weights);
				bsettings->weights = tmpf;
			}
			else if (weight_type == FUINT64)
			{
				int* tmpi = malloc(sizeof(int) * bsettings->weights_c);
				if (!tmpi)
					oom(sizeof(int) * bsettings->weights_c);
				for (int i = 0; i < bsettings->weights_c; i++)
				{
					tmpi[i] = (int)(((int64_t*)bsettings->weights)[i]);
				}
				free(bsettings->weights);
				bsettings->weights = tmpi;
			}
			else
			{
				fprintf(stderr, "error, invalid weight type\n");
				return 1;
			}
		}
	}
	else
	{
		if (!bsettings->weights_c)
		{
			bsettings->weights_c = 2;
		}
		int* tmpi = malloc(sizeof(int) * bsettings->weights_c);
		if (!tmpi)
			oom(sizeof(int) * bsettings->weights_c);
		for (int i = 0; i < bsettings->weights_c; i++)
		{
			tmpi[i] = 1;
		}
		bsettings->weights = tmpi;
	}

	bsettings->onum = vsettings->num;
	bsettings->oden = vsettings->den;
	bsettings->internal_size = vsettings->internal_data_bits / 8;

	return 0;
}

int vblend(char** args, int argsc)
{
	int return_code = 0;
	VBlenderSettings vsettings = { 0 };
	BlendSettings bsettings = { 0 };
	char** input_files = 0;
	int input_file_count = 0;
	char* output_folder = 0;

	if (vblend_parse(args, argsc, &vsettings, &bsettings, &input_files, &input_file_count, &output_folder))
	{
		fprintf(stderr, "can't parse\n");
		return_code = 1;
		goto end;
	}
	
	if (create_folder(output_folder))
	{
		fprintf(stderr, "can't create output folder\n");
		return_code = 1;
		goto end;
	}

	int path_len = strlen(output_folder);
	char* filename;
	int filename_len;

	for (int i = 0; i < input_file_count; i++)
	{
		filename = strrchr(input_files[i], '/');
		if (!filename)
			filename = strrchr(input_files[i], '\\');
		if (!filename)
		{
			filename = input_files[i];
		}
		else
			filename += 1;

		filename_len = strlen(filename);

		if (vsettings.output_file)
			free(vsettings.output_file);

		vsettings.output_file = malloc(path_len + filename_len + 2);
		if (!vsettings.output_file)
			oom(path_len + filename_len + 2);
		strcpy(vsettings.output_file, output_folder);
		strcat(vsettings.output_file, "/");
		strcat(vsettings.output_file, filename);

		vsettings.input_file = input_files[i];

		if (vblend_funct(&vsettings, &bsettings))
		{
			fprintf(stderr, "\n");
			return_code = 1;
			goto end;
		}
	}

	end:

	if (vsettings.output_file)
		free(vsettings.output_file);
	if (output_folder)
		free(output_folder);
	if (input_files)
	{
		for (int i = 0; i < input_file_count; i++)
			free(input_files[i]);
		free(input_files);
	}
	if (vsettings.decoder)
		free(vsettings.decoder);
	if (vsettings.encoder)
		free(vsettings.encoder);
	if (vsettings.encoder_options)
	{
		for (int i = 0; i < vsettings.encoder_options_count * 2; i++)
			free(vsettings.encoder_options[i]);
		free(vsettings.encoder_options);
	}
	if (bsettings.weights)
		free(bsettings.weights);

	return return_code;
}

char vblend_funct(VBlenderSettings* vsettings, BlendSettings* bsettings)
{
	char return_code = 0;

	CodingContext input = { 0 };
	CodingContext output = { 0 };
	AVDictionary* codec_options = { 0 };
	AVRational timebase = { 0 };
	int width = 0;
	int height = 0;
	int linesize[2] = { 0 };
	int bits_per_channel = 0;
	int bytes_per_channel = 0;
	int rgb_count = 0;
	int raw_rgb_size = 0;
	int converted_rgb_size = 0;
	enum AVPixelFormat rgb_format = 0;
	VBlenderAddSettings asettings = { 0 };
	VBlenderEncodeSettings esettings = { 0 };
	BlendContext* blend_ctx = 0;
	Thread add_thread = { 0 };
	char add_thread_running = 0;
	Thread encode_thread = { 0 };
	char encode_thread_running = 0;
	AVFrame* frame1 = 0;
	AVFrame* frame2 = 0;
	char on_one = 1;
	AVFrame* in_frame = 0;
	AVPacket* packet = 0;

	if (open_input(&input, vsettings->input_file, vsettings->decoder))
	{
		fprintf(stderr, "failled to open input %s\n", vsettings->input_file);
		return_code = 1;
		goto end;
	}

	output.video_stream_index = input.video_stream_index;

	if (open_output(input.format_ctx, &output, vsettings->output_file))
	{
		fprintf(stderr, "failled to open output %s\n", vsettings->output_file);
		return_code = 1;
		goto end;
	}

	for (int i = 0; i < vsettings->encoder_options_count; i++)
	{
		if (av_dict_set(&codec_options, vsettings->encoder_options[i * 2], vsettings->encoder_options[i * 2 + 1], 0) < 0)
		{
			fprintf(stderr, "failled to set codec option in dictionary\n");
			return_code = 1;
			goto end;
		}
	}

	timebase.num = vsettings->num; 
	timebase.den = vsettings->den;

	if (open_encoder(&output, vsettings->encoder, &codec_options, timebase, input.format_ctx->streams[input.video_stream_index]->codecpar))
	{
		fprintf(stderr, "failled to open encoder\n");
		return_code = 1;
		goto end;
	}

	width = input.format_ctx->streams[input.video_stream_index]->codecpar->width;
	height = input.format_ctx->streams[input.video_stream_index]->codecpar->height;
	linesize[0] = width * 3;
	rgb_count = width * height * 3;

	bits_per_channel = input.format_ctx->streams[input.video_stream_index]->codecpar->bits_per_raw_sample;
	bytes_per_channel = bits_per_channel / 8;
	if (bits_per_channel % 8)
		bytes_per_channel += 1;

	raw_rgb_size = rgb_count * bytes_per_channel;

	if (vsettings->internal_floating)
	{
		if (bytes_per_channel == 1)
		{
			rgb_format = AV_PIX_FMT_RGB24;
			esettings.final_rgb_type = FUINT8;
		}
		else if (bytes_per_channel == 2)
		{
			rgb_format = AV_PIX_FMT_RGB48;
			esettings.final_rgb_type = FUINT16;
		}
		else
		{
			fprintf(stderr, "unsupported input data\n");
			return_code = 1;
			goto end;
		}
		if (vsettings->internal_data_bits == 32)
		{
			bsettings->frame_type = FFLOAT32;
			bsettings->blend_funct = blend_float32_float32;
			asettings.convert_funct = uint8_float32;
		}
		else if (vsettings->internal_data_bits == 64)
		{
			bsettings->frame_type = FFLOAT64;
			bsettings->blend_funct = blend_float64_float64;
			asettings.convert_funct = uint8_float64;
		}
		else
		{
			fprintf(stderr, "unsupported input data\n");
			return_code = 1;
			goto end;
		}
	}
	else
	{
		if (bytes_per_channel == 1)
		{
			rgb_format = AV_PIX_FMT_RGB24;
			bsettings->frame_type = FUINT8;
			esettings.final_rgb_type = FUINT8;
			if (vsettings->internal_data_bits == 32)
			{
				bsettings->blend_funct = blend_uint8_int32;
			}
			else if (vsettings->internal_data_bits == 64)
			{
				bsettings->blend_funct = blend_uint8_int64;
			}
			else
			{
				fprintf(stderr, "unsupported input data\n");
				return_code = 1;
				goto end;
			}
		}
		else if (bytes_per_channel == 2)
		{
			rgb_format = AV_PIX_FMT_RGB48;
			bsettings->frame_type = FUINT16;
			esettings.final_rgb_type = FUINT16;
			if (vsettings->internal_data_bits == 32)
			{
				bsettings->blend_funct = blend_uint16_int32;
			}
			else if (vsettings->internal_data_bits == 64)
			{
				bsettings->blend_funct = blend_uint16_int64;
			}
			else
			{
				fprintf(stderr, "unsupported input data\n");
				return_code = 1;
				goto end;
			}
		}
		else
		{
			fprintf(stderr, "unsupported input data\n");
			return_code = 1;
			goto end;
		}
	}

	bsettings->width = width;
	bsettings->height = height;
	bsettings->inum = input.format_ctx->streams[input.video_stream_index]->r_frame_rate.den;
	bsettings->iden = input.format_ctx->streams[input.video_stream_index]->r_frame_rate.num;

	if (init_blending(&blend_ctx, bsettings))
	{
		fprintf(stderr, "failled to init blending\n");
		return_code = 1;
		goto end;
	}

	asettings.yuv_to_rgb =
		sws_getContext(width,
			height,
			input.codec_ctx->pix_fmt,
			width,
			height,
			rgb_format,
			SWS_BILINEAR,
			NULL, NULL, NULL);
	if (!asettings.yuv_to_rgb)
	{
		fprintf(stderr, "failled to get sws context\n");
		return_code = 1;
		goto end;
	}
	asettings.height = height;	
	asettings.tmp_array[0] = malloc(raw_rgb_size);
	if (!asettings.tmp_array[0])
		oom(raw_rgb_size);
	asettings.linesize = linesize;
	if (vsettings->internal_floating)
	{
		converted_rgb_size = rgb_count * type_sizes[bsettings->frame_type];
		asettings.converted_rgb_data = malloc(converted_rgb_size);
		if (!asettings.converted_rgb_data)
			oom(converted_rgb_size);
	}
	asettings.rgb_size = raw_rgb_size;
	asettings.blend_ctx = blend_ctx;

	esettings.output = &output;
	init_queue(&esettings.in_packets, sizeof(AVPacket*));
	create_mutex(&esettings.packet_mutex);
	esettings.blend_ctx = blend_ctx;
	esettings.rgb_count = rgb_count;
	esettings.output_rgb_type = bsettings->frame_type;
	esettings.rgb_to_yuv =
		sws_getContext(width,
			height,
			rgb_format,
			width,
			height,
			input.codec_ctx->pix_fmt,
			SWS_BILINEAR,
			NULL, NULL, NULL);
	if (!esettings.rgb_to_yuv)
	{
		fprintf(stderr, "failled to get sws context\n");
		return_code = 1;
		goto end;
	}
	esettings.linesize = linesize;
	esettings.height = height;
	esettings.out_frame = av_frame_alloc();
	if (!esettings.out_frame)
	{
		fprintf(stderr, "failled to av frame alloc\n");
		return_code = 1;
		goto end;
	}
	esettings.out_frame->format = input.format_ctx->streams[input.video_stream_index]->codecpar->format;
	esettings.out_frame->width = width;
	esettings.out_frame->height = height;
	if (av_frame_get_buffer(esettings.out_frame, 0))
	{
		fprintf(stderr, "failled to av get buffer\n");
		return_code = 1;
		goto end;
	}
	esettings.pts_step = (float)output.format_ctx->streams[output.video_stream_index]->time_base.num
		* (float)output.format_ctx->streams[output.video_stream_index]->time_base.den
		/ (float)bsettings->oden
		* (float)bsettings->onum;

	
	frame1 = av_frame_alloc();
	if (!frame1)
	{
		fprintf(stderr, "failled to av frame alloc\n");
		return_code = 1;
		goto end;
	}
	frame2 = av_frame_alloc();
	if (!frame2)
	{
		fprintf(stderr, "failled to av frame alloc\n");
		return_code = 1;
		goto end;
	}
	in_frame = frame1;
	packet = av_packet_alloc();
	if (!packet)
	{
		fprintf(stderr, "failled to av packet alloc\n");
		return_code = 1;
		goto end;
	}

	char flag;

	add_thread.function = vblender_add;
	add_thread.params = &asettings;
	encode_thread.function = vblender_encode;
	encode_thread.params = &esettings;
	launch_thread(&encode_thread);
	encode_thread_running = 1;

	uint64_t read_frames = 0;

	while (av_read_frame(input.format_ctx, packet) >= 0)
	{
		if (packet->stream_index == input.video_stream_index)
		{
			read_frames += 1;
			flag = decode_frame(&input, in_frame, packet);
			if (flag < 0)
			{
				fprintf(stderr, "failled to decode frame\n");
				return_code = 1;
				goto end;
			}
			if (!flag)
			{
				if (add_thread_running)
				{
					join_thread(&add_thread);
				}
				asettings.in_frame = in_frame;
				launch_thread(&add_thread);
				add_thread_running = 1;
				if (on_one)
					in_frame = frame2;
				else
					in_frame = frame1;
				on_one = !on_one;
			}
		}
		else
		{
			lock_mutex(&esettings.packet_mutex);
			add(&esettings.in_packets, packet);
			unlock_mutex(&esettings.packet_mutex);
			packet = av_packet_alloc();
		}
		if (asettings.exit)
		{
			fprintf(stderr, "add thread error\n");
			return_code = 1;
			goto end;
		}
		if (esettings.exit)
		{
			fprintf(stderr, "encode thread error\n");
			return_code = 1;
			goto end;
		}
		printf("\r%d/%d input frames(%f%%), %lld encoded frames, %d/%d threads",
			read_frames,
			input.format_ctx->streams[input.video_stream_index]->nb_frames,
			(float)read_frames / (float)input.format_ctx->streams[input.video_stream_index]->nb_frames * 100.0f,
			*esettings.encoded_frames,
			blend_ctx->blend_threads.size, 
			bsettings->max_blend_threads);
	}

	printf("\n");	

end:

	if (add_thread_running)
	{
		join_thread(&add_thread);
		add_thread_running = 0;
	}

	if (encode_thread_running)
	{
		esettings.finished_read_input = 1;
		join_thread(&encode_thread);
	}

	if (blend_ctx)
		finish_blending(blend_ctx);

	if (asettings.yuv_to_rgb)
		sws_freeContext(asettings.yuv_to_rgb);
	if (asettings.tmp_array[0])
		free(asettings.tmp_array[0]);
	if (asettings.converted_rgb_data)
		free(asettings.converted_rgb_data);

	if (esettings.in_packets.size)
	{
		AVPacket* tmp_packet;
		while (esettings.in_packets.size)
		{
			front(&esettings.in_packets, (void**)&tmp_packet);
			qremove(&esettings.in_packets);
			av_packet_free(&tmp_packet);
		}
	}
	delete_mutex(&esettings.packet_mutex);
	if (esettings.rgb_to_yuv)
		sws_freeContext(esettings.rgb_to_yuv);
	if (esettings.out_frame)
		av_frame_free(&esettings.out_frame);
	
	if (output.format_ctx)
	{
		close_output_file(output.format_ctx);
		close_coding_context(&output);
	}
	if (input.format_ctx)
		close_coding_context(&input);
	if (codec_options)
		av_dict_free(&codec_options);
	if (blend_ctx)
		free_blending(blend_ctx);
	
	if (frame1)
		av_frame_free(&frame1);
	if (frame2)
		av_frame_free(&frame2);
	if (packet)
		av_packet_free(&packet);
	
	return 0;
}

void vblender_add(VBlenderAddSettings* asettings)
{
	sws_scale(asettings->yuv_to_rgb,
		(const uint8_t*const*) asettings->in_frame->data,
		asettings->in_frame->linesize,
		0,
		asettings->height,
		(uint8_t**)asettings->tmp_array,
		asettings->linesize);

	void* rgb_data;

	if (asettings->convert_funct)
	{
		asettings->convert_funct(asettings->tmp_array[0], asettings->converted_rgb_data, asettings->rgb_size);
		rgb_data = asettings->converted_rgb_data;
	}
	else
	{
		rgb_data = asettings->tmp_array[0];
	}

	if (add_frame(asettings->blend_ctx, rgb_data))
	{
		fprintf(stderr, "failled to add frame to blending\n");
		asettings->exit = 1;
		return;
	}
}

void vblender_encode(VBlenderEncodeSettings* esettings)
{
	AVPacket** tmp_packets = 0;
	void** tmp_frames = 0;
	uint64_t tmp_count = 0;
	uint64_t tmp_size = 0;

	int ret = 0;

	void* tmp_array[] = { 0, 0 };

	void* converted_rgb_data = 0;
	int converted_rgb_size = esettings->rgb_count * type_sizes[esettings->output_rgb_type];
	char convertion = esettings->output_rgb_type != esettings->final_rgb_type;
	if (convertion)
	{
		converted_rgb_data = malloc(converted_rgb_size);
		if (!converted_rgb_data)
			oom(converted_rgb_size);
		tmp_array[0] = converted_rgb_data;
	}
	void (*convert_funct)(void*, void*, uint64_t) = get_convert_function(esettings->output_rgb_type, esettings->final_rgb_type);

	uint64_t encoded_frames = 0;
	esettings->encoded_frames = &encoded_frames;

	AVPacket* packet = av_packet_alloc();
	if (!packet)
	{
		fprintf(stderr, "failled to av packet alloc\n");
		print_av_error(ret);
		esettings->exit = 1;
		goto end;
	}

	while (!esettings->finished_read_input || esettings->in_packets.size || esettings->blend_ctx->created_frames.size)
	{
		if (esettings->in_packets.size)
		{
			lock_mutex(&esettings->packet_mutex);
			tmp_count = esettings->in_packets.size;
			tmp_size = tmp_count * sizeof(AVPacket*);
			tmp_packets = malloc(tmp_size);
			if (!tmp_packets)
				oom(tmp_size);
			memcpy(tmp_packets, esettings->in_packets.elements, tmp_size);
			empty(&esettings->in_packets);
			unlock_mutex(&esettings->packet_mutex);

			for (uint64_t i = 0; i < tmp_count; i++)
			{
				ret = av_interleaved_write_frame(esettings->output->format_ctx, tmp_packets[i]);
				if (ret < 0)
				{
					fprintf(stderr, "failled to write packet\n");
					print_av_error(ret);
					esettings->exit = 1;
					goto end;
				}

				av_packet_free(&tmp_packets[i]);
				tmp_packets[i] = 0;
			}

			free(tmp_packets);
			tmp_packets = 0;
		}
		else if (esettings->blend_ctx->created_frames.size)
		{
			acquire_output_frames(esettings->blend_ctx);
			tmp_count = get_output_frame_count(esettings->blend_ctx);
			tmp_frames = malloc(tmp_count * sizeof(void*));
			if (!tmp_frames)
				oom(tmp_count * sizeof(void*));
			for (uint64_t i = 0; i < tmp_count; i++)
			{
				tmp_frames[i] = get_output_frame(esettings->blend_ctx);
			}
			release_out_frames(esettings->blend_ctx);

			for (uint64_t i = 0; i < tmp_count; i++)
			{
				if (convertion)
				{
					convert_funct(tmp_frames[i], converted_rgb_data, esettings->rgb_count);
				}
				else
				{
					tmp_array[0] = tmp_frames[i];
				}
				sws_scale(esettings->rgb_to_yuv,
					(const uint8_t *const*) tmp_array,
					esettings->linesize,
					0,
					esettings->height,
					esettings->out_frame->data,
					esettings->out_frame->linesize);

				esettings->out_frame->pts = (uint64_t)(esettings->pts_step * (float)encoded_frames);

				if ((ret = avcodec_send_frame(esettings->output->codec_ctx, esettings->out_frame)) < 0)
				{
					fprintf(stderr, "failled to send frame to codec\n");
					print_av_error(ret);
					esettings->exit = 1;
					goto end;
				}

				while ((ret = avcodec_receive_packet(esettings->output->codec_ctx, packet)) >= 0)
				{
					if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
						break;

					packet->stream_index = esettings->output->video_stream_index;

					ret = av_interleaved_write_frame(esettings->output->format_ctx, packet);
					if (ret < 0)
					{
						fprintf(stderr, "failled to write packet\n");
						print_av_error(ret);
						esettings->exit = 1;
						goto end;
					}

					av_packet_unref(packet);
				}

				encoded_frames += 1;
				free(tmp_frames[i]);
				tmp_frames[i] = 0;
			}

			free(tmp_frames);
			tmp_frames = 0;
		}
		else
		{
			sleep_milli(1);
		}
	}

	//flush encoder and encode last frames
	if ((ret = avcodec_send_frame(esettings->output->codec_ctx, 0)) < 0)
	{
		fprintf(stderr, "failled to send frame to codec\n");
		print_av_error(ret);
		esettings->exit = 1;
		goto end;
	}

	while ((ret = avcodec_receive_packet(esettings->output->codec_ctx, packet)) >= 0)
	{
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			break;

		packet->stream_index = esettings->output->video_stream_index;

		ret = av_interleaved_write_frame(esettings->output->format_ctx, packet);
		if (ret < 0)
		{
			fprintf(stderr, "failled to write packet\n");
			print_av_error(ret);
			esettings->exit = 1;
			goto end;
		}

		av_packet_unref(packet);
	}
	
	end:

	av_packet_free(&packet);

	if (convertion)
		free(converted_rgb_data);

	if (tmp_packets)
	{
		for (int i=0;i< tmp_count;i++)
			if (tmp_packets[i])
				av_packet_free(&tmp_packets[i]);
		free(tmp_packets);
	}

	if (tmp_frames)
	{
		for (int i = 0; i < tmp_count; i++)
			if (tmp_frames[i])
				free(tmp_frames[i]);
		free(tmp_frames);
	}

}
