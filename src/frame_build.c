#include "frame_build.h"
#include <stdio.h>
#include <string.h>
#include "crc16.h"

/* Construct a frame: [SOF][LEN][PAYLOAD][CRC] */
int frame_build(uint8_t* out, void* p, uint16_t len)
{
    int pos = 0;

    /* Add Start of Frame and Length (Big-endian) */
    out[pos++] = FRAME_SOF;
    out[pos++] = (uint8_t)(len >> 8);
    out[pos++] = (uint8_t)len;

    /* Copy payload data */
    memcpy(&out[pos], p, len);
    pos += len;

    /* Calculate and append CRC16 */
    uint16_t crc = crc16_ccitt((uint8_t*)p, len);

    out[pos++] = (uint8_t)(crc >> 8);
    out[pos++] = (uint8_t)crc;

    /* Return total bytes written */
    return pos;
}