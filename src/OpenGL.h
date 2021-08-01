#pragma once

#include <windows.h>
#include <GL/glew.h>
#include <GL/GL.h>

#pragma comment (lib, "opengl32.lib")

typedef struct Programs
{
    unsigned int nb_x;
    unsigned int nb_y;

    unsigned int init;

    unsigned int add;
    unsigned int w_location;


    unsigned int finish;
    unsigned int tw_location;

    unsigned int texture;
    unsigned int tmp_texture;
} Programs;

//https://gist.github.com/nickrolfe/1127313ed1dbf80254b614a721b3ee9c
#define WGL_CONTEXT_MAJOR_VERSION_ARB             0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB             0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001

#include "Convert.h"

typedef HGLRC WINAPI wglCreateContextAttribsARB_type(HDC hdc, HGLRC hShareContext,
    const int* attribList);
wglCreateContextAttribsARB_type* wglCreateContextAttribsARB;

int set_up_opengl_context(Programs* programs, int width, int height, HDC* dc, HGLRC* gl_ctx);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int read_shader(char* path, char** src);
int compile_shader(char* shader_src, unsigned int* program);
int create_texture(unsigned int* texture);

int test(RGBFrame* frame, int width, int height);