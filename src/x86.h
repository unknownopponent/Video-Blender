#pragma once

#if defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64) || defined(__amd64) || defined(__amd64__) || defined (__x86_64) || defined(__x86_64__)
#define  X86_64
#elif defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)
#define X86_32
#endif

#if defined(X86_64) || defined(X86_32)

#if defined(__GNUC__) || defined(__llvm__)
#define SSE2_TARGET __attribute__ ((__target__ ("sse2")))
#define SSSE3_TARGET __attribute__ ((__target__ ("ssse3")))
#define SSE41_TARGET __attribute__ ((__target__ ("sse4.1")))
#define SSE42_TARGET __attribute__ ((__target__ ("sse4.2")))
#define AVX2_TARGET __attribute__ ((__target__ ("avx2")))
#define FMA_TARGET __attribute__ ((__target__ ("fma")))
#define FMA_AVX2_TARGET __attribute__ ((__target__ ("fma,avx2")))
#else
#define SSE2_TARGET 
#define SSSE3_TARGET
#define SSE41_TARGET
#define SSE42_TARGET
#define AVX2_TARGET
#define FMA_TARGET
#define FMA_AVX2_TARGET
#endif

#if defined(__GNUC__) || defined(__llvm__)
#include <x86intrin.h>
#elif defined(_MSC_VER)
#include <intrin.h>
#else
#include <immintrin.h>
#endif

#endif // x86