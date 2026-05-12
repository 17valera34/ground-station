#include "crc16.h"

/* Standard bit-by-bit CRC16-CCITT implementation */
uint16_t crc16_ccitt(uint8_t* data, uint16_t len)
{
    uint16_t crc = 0xFFFF; /* Initial value */

    for (uint16_t i = 0; i < len; i++)
    {
        crc ^= (uint16_t)(data[i] << 8);

        for (int j = 0; j < 8; j++)
        {
            /* Check MSB and shift */
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021; /* CCITT polynomial */
            else
                crc <<= 1;
        }
    }
    return crc;
}