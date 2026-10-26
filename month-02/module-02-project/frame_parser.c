#include "frame_parser.h"
#include <string.h>
#include "crc8.h"
bool parser_push_byte(uint8_t byte,
                      uint8_t frame[FRAME_SIZE],
                      size_t *received,
                      ParserState *state)
{
    if (*state == WAIT_MAGIC_0) {
        if (byte == 0x4E) {
            frame[0] = byte;
            *received = 1;
            *state = WAIT_MAGIC_1;
        }

        return false;
    }

    if (*state == WAIT_MAGIC_1) {
        if (byte == 0x52) {
            frame[1] = byte;
            *received = 2;
            *state = COLLECT_FRAME;
        } else if (byte == 0x4E) {
            frame[0] = byte;
            *received = 1;
            *state = WAIT_MAGIC_1;
        } else {
            *received = 0;
            *state = WAIT_MAGIC_0;
        }

        return false;
    }

    if (*state == COLLECT_FRAME) {
    frame[*received] = byte;
    (*received)++;

    if (*received < FRAME_SIZE) {
        return false;
    }

    /* We now have an eight-byte candidate. */

    bool valid =
        frame[0] == 0x4E &&
        frame[1] == 0x52 &&
        frame[2] == 0x01 &&
    crc8_calculate(frame, FRAME_SIZE - 1) == frame[FRAME_SIZE - 1];

    if (valid) {
        *received = 0;
        *state = WAIT_MAGIC_0;
        return true;
    }

for (size_t i = 1; i + 1 < FRAME_SIZE; i++) {

    if (frame[i] == 0x4E &&
        frame[i + 1] == 0x52) {

        size_t remaining = FRAME_SIZE - i;

memmove(frame, &frame[i], remaining);
*received=remaining;
*state=COLLECT_FRAME;
return false;    }
}

if (frame[FRAME_SIZE - 1]==0x4E){
frame[0]=0x4E;
*received=1;
*state=WAIT_MAGIC_1;}
else {
*received=0;
*state=WAIT_MAGIC_0;}
    return false;
}
return false;
}
