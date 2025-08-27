#pragma once

// Ensure that the chosen width requires no extra padding for the bitmap
static_assert((WIDTH / 2) % 4 == 0, "Image width must be 4-byte aligned");

// Width is divided by 2 for 4bpp, aka 2 pixels per byte
typedef uint8_t framebuffer_t[HEIGHT][WIDTH / 2];

#include "bmp_header.h"
