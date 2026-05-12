#pragma once

#include <stdint.h>

/* Protocol constants */
#define FRAME_SOF 0xAA

/**
 * Build a protocol frame
 * Returns total frame size
 */
int frame_build(uint8_t* out, void* p, uint16_t len);