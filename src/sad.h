
#pragma once

#include <stdint.h>
#include <stddef.h>
#include "x86.h"

uint64_t sad_u8_rectangle(uint8_t* block1, uint8_t* block2, uint64_t offset, uint64_t width, uint64_t height);
SSE2_TARGET uint64_t sad_u8_rectangle_sse2(uint8_t* block1, uint8_t* block2, uint64_t offset, uint64_t width, uint64_t height);
AVX2_TARGET uint64_t sad_u8_rectangle_avx2(uint8_t* block1, uint8_t* block2, uint64_t offset, uint64_t width, uint64_t height);
