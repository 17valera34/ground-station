#include "frame_parser.h"
#include <stdio.h>
#include <string.h>
#include "crc16.h"

/* Reset parser state and data */
void frame_parser_init(frame_parser_t* p)
{
    p->state = FRAME_WAIT_SOF;
    p->len   = 0;
    p->crc   = 0;
    p->pos   = 0;
    memset(p->payload, 0, sizeof(p->payload));
}

/* Process incoming byte; returns 1 if frame is valid */
int frame_parser_push(frame_parser_t* p, uint8_t byte, frame_t* out)
{
    switch (p->state)
    {
        case FRAME_WAIT_SOF:
            if (byte == FRAME_SOF)
            {
                p->state = FRAME_READ_LEN;
                p->pos   = 0;
            }
            break;

        case FRAME_READ_LEN:
            if (p->pos == 0)
            {
                p->len = (uint16_t)(byte << 8);
                p->pos = 1;
            }
            else
            {
                p->len |= (uint16_t)byte;
                /* Validate length */
                if (p->len > FRAME_PAYLOAD_SIZE)
                {
                    p->state = FRAME_WAIT_SOF;
                }
                else
                {
                    p->pos   = 0;
                    p->state = FRAME_READ_PAYLOAD;
                }
            }
            break;

        case FRAME_READ_PAYLOAD:
            p->payload[p->pos++] = byte;
            if (p->pos == p->len)
                p->state = FRAME_READ_CRC_H;
            break;

        case FRAME_READ_CRC_H:
            p->crc   = (uint16_t)(byte << 8);
            p->state = FRAME_READ_CRC_L;
            break;

        case FRAME_READ_CRC_L:
            p->crc |= (uint16_t)byte;

            uint16_t crc_calc = crc16_ccitt(p->payload, p->len);

            /* Verify CRC and copy output */
            if (p->crc == crc_calc)
            {
                out->len = p->len;
                memcpy(out->payload, p->payload, p->len);
                frame_parser_init(p);
                return 1;
            }
            else
            {
                printf("frame parser failed\n");
                frame_parser_init(p);
            }
            break;
    }
    return 0;
}
