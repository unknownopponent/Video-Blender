#include "OpenGL.h"

#include <stdio.h>

int set_up_opengl_context(Programs* programs, int width, int height, HDC* dc, HGLRC* gl_ctx)
{
	WNDCLASSEXW wclass = { 0 };
	wclass.cbSize = sizeof(WNDCLASSEXW);
	wclass.style = CS_OWNDC;
	wclass.lpfnWndProc = WndProc;
	wclass.hInstance = GetModuleHandle(NULL);
	wclass.lpszClassName = L"videoblenderclassname";

	if (!RegisterClassExW(&wclass))
	{
		printf("can't RegisterClassExW\n");
		return 1;
	}

	HWND window = CreateWindowExW(
		0,
		wclass.lpszClassName,
		L"video blender window",
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		wclass.hInstance,
		0
	);

	if (!window)
	{
		printf("can't CreateWindowExW\n");
		return 1;
	}

	MSG msg = { 0 };

	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		;

	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
		PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
		32,                   // Colordepth of the framebuffer.
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		24,                   // Number of bits for the depthbuffer
		8,                    // Number of bits for the stencilbuffer
		0,                    // Number of Aux buffers in the framebuffer.
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	*dc = GetDC(window);
	if (!*dc)
	{
		printf("can't GetDC\n");
		return 1;
	}

	int  pix_fmt;
	pix_fmt = ChoosePixelFormat(*dc, &pfd);
	if (!pix_fmt)
	{
		printf("can't ChoosePixelFormat\n");
		return 1;
	}
	if (!SetPixelFormat(*dc, pix_fmt, &pfd))
	{
		printf("can't SetPixelFormat\n");
		return 1;
	}

	*gl_ctx = wglCreateContext(*dc);
	if (!wglMakeCurrent(*dc, *gl_ctx))
	{
		printf("can't wglMakeCurrent\n");
		return 1;
	}

	if (glewInit() != GLEW_OK)
	{
		printf("can't glewInit\n");
		return 1;
	}

	wglCreateContextAttribsARB = (wglCreateContextAttribsARB_type*)wglGetProcAddress(
		"wglCreateContextAttribsARB");

	if (!wglDeleteContext(*gl_ctx))
	{
		printf("can't wglDeleteContext\n");
		return 1;
	}

	int attribs[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
		WGL_CONTEXT_MINOR_VERSION_ARB, 6,
		WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0,
	};
	
	*gl_ctx = wglCreateContextAttribsARB(*dc, 0, attribs);
	if (!*gl_ctx)
	{
		printf("can't wglCreateContextAttribsARB\n");
		return 1;
	}

	if (!wglMakeCurrent(*dc, *gl_ctx))
	{
		printf("can't wglMakeCurrent\n");
		return 1;
	}

	char* paths[] = {
		"repo/assets/Initialize.compute",
		"repo/assets/AddFrame.compute",
		"repo/assets/Finish.compute"
	};

	char src[1024];
	//todo
	char* head = "#version 460\n"
		"layout (local_size_x = 16, local_size_y = 9) in;\n"
	;
	programs->nb_x = 120;
	programs->nb_y = 120;

	char* src_body;

	if (read_shader(paths[0], &src_body))
	{
		printf("can't read shader\n");
		return 1;
	}

	if (strcpy_s(src, 1024, head))
	{
		printf("can't strcat\n");
		return 1;
	}
	if (strcat_s(src, 1024, src_body))
	{
		printf("can't strcat\n");
		return 1;
	}
	free(src_body);

	if (compile_shader(src, &programs->init))
	{
		printf("can't compile shader\n");
		return 1;
	}

	if (read_shader(paths[1], &src_body))
	{
		printf("can't read shader\n");
		return 1;
	}

	if (strcpy_s(src, 1024, head))
	{
		printf("can't strcat\n");
		return 1;
	}
	if (strcat_s(src, 1024, src_body))
	{
		printf("can't strcat\n");
		return 1;
	}
	free(src_body);

	if (compile_shader(src, &programs->add))
	{
		printf("can't compile add shader\n");
		return 1;
	}

	glUseProgram(programs->add);
	programs->w_location = glGetUniformLocation(programs->add, "weight");

	if (read_shader(paths[2], &src_body))
	{
		printf("can't read shader\n");
		return 1;
	}

	if (strcpy_s(src, 1024, head))
	{
		printf("can't strcat\n");
		return 1;
	}
	if (strcat_s(src, 1024, src_body))
	{
		printf("can't strcat\n");
		return 1;
	}
	free(src_body);

	if (compile_shader(src, &programs->finish))
	{
		printf("can't compile finish shader\n");
		return 1;
	}

	GLenum err;

	glUseProgram(programs->finish);
	programs->tw_location = glGetUniformLocation(programs->finish, "total_weight");

	while ((err = glGetError()) != GL_NO_ERROR)
	{
		printf("%d\n", err);
	}

	create_texture(&programs->texture);
	glBindTexture(GL_TEXTURE_2D, programs->texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGB, GL_FLOAT, 0);
	glBindImageTexture(0, programs->texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	while ((err = glGetError()) != GL_NO_ERROR)
	{
		printf("%d\n", err);
	}

	create_texture(&programs->tmp_texture);
	glBindTexture(GL_TEXTURE_2D, programs->tmp_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGB, GL_FLOAT, 0);
	glBindImageTexture(1, programs->tmp_texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	while ((err = glGetError()) != GL_NO_ERROR)
	{
		printf("%d\n", err);
	}

	printf("");

	/*
	printf("%s\n", glGetString(GL_VERSION));

	int workgroup_count[3];
	int workgroup_size[3];
	int workgroup_invocations;

	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &workgroup_count[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &workgroup_count[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &workgroup_count[2]);

	printf("Taille maximale des workgroups:\n\tx:%u\n\ty:%u\n\tz:%u\n",
		workgroup_size[0], workgroup_size[1], workgroup_size[2]);

	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &workgroup_size[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &workgroup_size[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &workgroup_size[2]);

	printf("Nombre maximal d'invocation locale:\n\tx:%u\n\ty:%u\n\tz:%u\n",
		workgroup_size[0], workgroup_size[1], workgroup_size[2]);

	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &workgroup_invocations);
	printf("Nombre maximum d'invocation de workgroups:\n\t%u\n", workgroup_invocations);
	*/

	if (!wglMakeCurrent(*dc, 0))
	{
		printf("can't wglMakeCurrent\n");
		return 1;
	}

	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd, message, wParam, lParam);
}

int read_shader(char* path, char** src)
{
	FILE* file;
	if (fopen_s(&file, path, "r"))
	{
		printf("can't open file\n");
		return 1;
	}
	
	*src = malloc(1024);
	if (!*src)
	{
		printf("can't malloc");
		return 1;
	}
	*src[0] = '\0';

	char string[128];

	while (fgets(string, 128, file))
	{
		if (strcat_s(*src, 1024, string))
		{
			printf("can't strcat\n");
			return 1;
		}
	}

	fclose(file);

	return 0;
}

int compile_shader(char* shader_src, unsigned int* program)
{
	GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(shader, 1, &shader_src, NULL); // vertex_shader_source is a GLchar* containing glsl shader source code
	glCompileShader(shader);

	GLint code;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &code);
	if (code != GL_TRUE)
	{
		GLsizei log_length = 0;
		GLchar message[1024];
		glGetShaderInfoLog(shader, 1024, &log_length, message);
		printf("%s\n", message);
		return 1;
	}

	*program = glCreateProgram();
	glAttachShader(*program, shader);
	glLinkProgram(*program);

	glGetProgramiv(*program, GL_LINK_STATUS, &code);
	if (code != GL_TRUE)
	{
		GLsizei log_length = 0;
		GLchar message[1024];
		glGetProgramInfoLog(*program, 1024, &log_length, message);
		printf("%s\n", message);
		return 1;
	}

	return 0;
}

int create_texture(unsigned int* texture)
{
	glGenTextures(1, texture);
	glBindTexture(GL_TEXTURE_2D, *texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glBindTexture(GL_TEXTURE_2D, 0);

	return 0;
}

int test(RGBFrame* frame, int width, int height)
{
	GLenum err;

	int size = width * height * 3;

	float* data = malloc(sizeof(float) * size);

	for (int i = 0; i < size; i++)
	{
		data[i] = (float)frame->data[0][i];
	}

	char* src =
		"#version 460\n"
		"\n"
		"layout (local_size_x = 16, local_size_y = 9) in;\n"
		"layout (rgba32f, binding = 0) uniform image2D img_output;\n"
		"uniform float weight;\n"
		"\n"
		"void main() {\n"
		"	ivec2 coords = ivec2(gl_GlobalInvocationID);\n"

		// Pour mettre en evidence. Les groupes de travail locaux on dessine un damier.
		"	vec4 pixel = imageLoad(img_output, coords);\n"
		"	pixel = vec4(pixel.x + weight, pixel.y + 3000.0, pixel.z + 3000.0, pixel.w);\n"
		"	imageStore(img_output, coords, pixel);\n"
		"}"
	;

	GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(shader, 1, &src, NULL); // vertex_shader_source is a GLchar* containing glsl shader source code
	glCompileShader(shader);

	GLint code;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &code);
	if (code != GL_TRUE)
	{
		GLsizei log_length = 0;
		GLchar message[1024];
		glGetShaderInfoLog(shader, 1024, &log_length, message);
		// Write the error to a log
		printf("%s\n", message);
	}

	GLuint program = glCreateProgram();
	glAttachShader(program, shader);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &code);
	if (code != GL_TRUE)
	{
		GLsizei log_length = 0;
		GLchar message[1024];
		glGetProgramInfoLog(program, 1024, &log_length, message);
		// Write the error to a log
		printf("%s\n", message);
	}

	glUseProgram(program);

	int location = glGetUniformLocation(program, "weight");
	glUniform1f(location, 100.0f);

	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	while ((err = glGetError()) != GL_NO_ERROR)
	{
		printf("%d\n", err);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGB, GL_FLOAT, data);
	
	while ((err = glGetError()) != GL_NO_ERROR)
	{
		printf("%d\n", err);
	}

	glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	
	while ((err = glGetError()) != GL_NO_ERROR)
	{
		printf("%d\n", err);
	}

	glDispatchCompute(120, 120, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	
	while ((err = glGetError()) != GL_NO_ERROR)
	{
		printf("%d\n", err);
	}

	//memset(data, 1.0f, size * sizeof(float));

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, data);
	//glGetnTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, size * sizeof(float), data);
	//glGetTextureImage(texture, 0, GL_RGB, GL_FLOAT, size * sizeof(float),data);

	while ((err = glGetError()) != GL_NO_ERROR)
	{
		printf("%d\n", err);
	}

	return 0;
}