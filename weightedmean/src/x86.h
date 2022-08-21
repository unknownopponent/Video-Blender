#pragma once

#if defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64) || defined(__amd64) || defined(__amd64__) || defined (__x86_64) || defined(__x86_64__)
#define  X86_64
#elif defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)
#define X86_32
#endif

#ifdef __GNUC__
#define SSE2_TARGET __attribute__ ((__target__ ("sse2")))
#define SSE41_TARGET __attribute__ ((__target__ ("sse4.1")))
#define AVX2_TARGET __attribute__ ((__target__ ("avx2")))
#else
#define SSE2_TARGET 
#define SSE3_TARGET
#define SSE41_TARGET
#define AVX2_TARGET
#endif