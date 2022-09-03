
#pragma once

#include <stdint.h>
#include <stddef.h>
#include "../x86.h"

uint64_t sad_u8_square(uint8_t* block1, uint8_t* block2, uint64_t offset, uint64_t width);

uint64_t sad_u8_rectangle(uint8_t* block1, uint8_t* block2, uint64_t offset, uint64_t width, uint64_t height);


#if defined(X86_64) || defined(X86_32)

SSE2_TARGET uint64_t sad_u8_2x2_sse2(uint8_t* block1, uint8_t* block2, uint64_t offset);

SSE2_TARGET uint64_t sad_u8_6x2_sse2(uint8_t* block1, uint8_t* block2, uint64_t offset);

SSE2_TARGET uint64_t sad_u8_4x4_sse2(uint8_t* block1, uint8_t* block2, uint64_t offset);

SSSE3_TARGET uint64_t sad_u8_12x4_ssse3(uint8_t* block1, uint8_t* block2, uint64_t offset);

SSE2_TARGET uint64_t sad_u8_8x8_sse2(uint8_t* block1, uint8_t* block2, uint64_t offset);

SSSE3_TARGET uint64_t sad_u8_24x8_ssse3(uint8_t* block1, uint8_t* block2, uint64_t offset);
AVX2_TARGET uint64_t sad_u8_24x8_avx2(uint8_t* block1, uint8_t* block2, uint64_t offset);

SSSE3_TARGET uint64_t sad_u8_16x16_ssse3(uint8_t* block1, uint8_t* block2, uint64_t offset);

SSSE3_TARGET uint64_t sad_u8_48x16_ssse3(uint8_t* block1, uint8_t* block2, uint64_t offset);
AVX2_TARGET uint64_t sad_u8_48x16_avx2(uint8_t* block1, uint8_t* block2, uint64_t offset);

SSSE3_TARGET uint64_t sad_u8_32x32_ssse3(uint8_t* block1, uint8_t* block2, uint64_t offset);
AVX2_TARGET uint64_t sad_u8_32x32_avx2(uint8_t* block1, uint8_t* block2, uint64_t offset);

SSSE3_TARGET uint64_t sad_u8_96x32_ssse3(uint8_t* block1, uint8_t* block2, uint64_t offset);
AVX2_TARGET uint64_t sad_u8_96x32_avx2(uint8_t* block1, uint8_t* block2, uint64_t offset);

#endif