#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define oom() fprintf(stderr, "out of memory\n"); exit(1);