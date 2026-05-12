#pragma once

#include <stdint.h>

/**
 * Calculate CRC16-CCITT checksum
 * Uses 0x1021 polynomial
 */
uint16_t crc16_ccitt(uint8_t* data, uint16_t len);