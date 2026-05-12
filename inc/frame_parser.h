#pragma once

#include <stdint.h>

/* Protocol constants */
#define FRAME_SOF 0xAA
#define FRAME_PAYLOAD_SIZE 256

/* Data container */
typedef struct
{
    uint16_t len;
    uint8_t payload[FRAME_PAYLOAD_SIZE];
} frame_t;

/* Parser states */
typedef enum
{
    FRAME_WAIT_SOF,
    FRAME_READ_LEN,
    FRAME_READ_PAYLOAD,
    FRAME_READ_CRC_H,
    FRAME_READ_CRC_L
} frame_parse_state_t;

/* Parser context */
typedef struct
{
    frame_parse_state_t state;
    uint16_t len;
    uint16_t crc;
    uint8_t pos;
    uint8_t payload[FRAME_PAYLOAD_SIZE];
} frame_parser_t;

/* API Functions */
void frame_parser_init(frame_parser_t* p);
int frame_parser_push(frame_parser_t* p, uint8_t byte, frame_t* out);