#ifndef FRAME_PARSER_H
#define FRAME_PARSER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define FRAME_SIZE 8

typedef enum {
    WAIT_MAGIC_0,
    WAIT_MAGIC_1,
    COLLECT_FRAME
} ParserState;

bool parser_push_byte(uint8_t byte,
                      uint8_t frame[FRAME_SIZE],
                      size_t *received,
                      ParserState *state);

#endif
