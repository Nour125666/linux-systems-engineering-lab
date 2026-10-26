#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "crc8.h"
#include "frame_parser.h"
#include "string.h"
static void set_frame_crc(uint8_t frame[FRAME_SIZE])
{
    frame[FRAME_SIZE - 1] =
        crc8_calculate(frame, FRAME_SIZE - 1);
}
static void test_lost_byte(void)
{
    uint8_t first[FRAME_SIZE] = {
        0x4E, 0x52, 0x01, 0x00,
        0x34, 0x12, 0xAB, 0x00
    };

    uint8_t second[FRAME_SIZE] = {
        0x4E, 0x52, 0x01, 0x00,
        0x78, 0x13, 0xAB, 0x00
    };

    set_frame_crc(first);
    set_frame_crc(second);

    uint8_t stream[2 * FRAME_SIZE - 1];
    size_t pos = 0;

    /* Simulate losing byte 3 of the first frame. */
    for (size_t i = 0; i < FRAME_SIZE; i++) {
        if (i != 3) {
            stream[pos++] = first[i];
        }
    }

    /* Append the intact second frame. */
    for (size_t i = 0; i < FRAME_SIZE; i++) {
        stream[pos++] = second[i];
    }

    assert(pos == sizeof(stream));

    uint8_t frame[FRAME_SIZE] = {0};
    size_t received = 0;
    ParserState state = WAIT_MAGIC_0;
    size_t completed = 0;

    for (size_t i = 0; i < sizeof(stream); i++) {
        bool ready = parser_push_byte(
            stream[i], frame, &received, &state
        );

        if (ready) {
            completed++;

            /* Only the second frame should be accepted. */
            assert(memcmp(frame, second, FRAME_SIZE) == 0);
        }
    }

    assert(completed == 1);
    assert(received == 0);
    assert(state == WAIT_MAGIC_0);

    puts("PASS: lost-byte recovery");
}       
static void test_valid_frame(void)
{
    uint8_t input[FRAME_SIZE] = {
        0x4E, 0x52, 0x01, 0x00,
        0x34, 0x12, 0xAB, 0xCD
    };

    set_frame_crc(input);

    uint8_t frame[FRAME_SIZE] = {0};
    size_t received = 0;
    ParserState state = WAIT_MAGIC_0;

    for (size_t i = 0; i < FRAME_SIZE; i++) {
        bool complete = parser_push_byte(
            input[i], frame, &received, &state
        );

        // TODO 1:
if (i < FRAME_SIZE - 1) {
        assert(complete == false);
    } else {
        assert(complete == true);
    }
        // For the first seven bytes, assert complete == false.
        // For the eighth byte, assert complete == true.
    }

    // TODO 2:
    // Verify frame[0] == 0x4E.
   assert(frame[0]==0x4E);
assert(frame[1]==0x52);
assert(frame[4]==0x34);
assert(frame[5]==0x12); // Verify frame[1] == 0x52.
    // Verify frame[4] == 0x34.
    // Verify frame[5] == 0x12.

    printf("PASS: valid frame\n");
}
static void test_garbage_before_frame(void)
{
    uint8_t input[] = {
        0xFF, 0x00, 0x99,
        0x4E, 0x52, 0x01, 0x00,
        0x34, 0x12, 0xAB, 0xCD
    };

    set_frame_crc(&input[3]);

    uint8_t frame[FRAME_SIZE] = {0};
    size_t received = 0;
    ParserState state = WAIT_MAGIC_0;
    size_t complete_count = 0;

    for (size_t i = 0; i < sizeof(input); i++) {
        if (parser_push_byte(input[i], frame, &received, &state)) {
            complete_count++;
        }
    }

    // TODO 3:
    // Assert that complete_count == 1.
assert(complete_count ==1);
    printf("PASS: garbage before frame\n");
}

static void test_incomplete_frame(void)
{
    uint8_t input[] = {
        0x4E, 0x52, 0x01, 0x00
    };

    uint8_t frame[FRAME_SIZE] = {0};
    size_t received = 0;
    ParserState state = WAIT_MAGIC_0;

    for (size_t i = 0; i < sizeof(input); i++) {
        bool complete = parser_push_byte(
            input[i], frame, &received, &state
        );

        // TODO 4:
      assert(complete == false);  // Assert that complete is always false.
    }

    printf("PASS: incomplete frame\n");
}
static void test_repeated_magic(void)
{
    uint8_t input[] = {
        0x4E, 0x4E, 0x52, 0x01,
        0x00, 0x34, 0x12, 0xAB, 0xCD
    };

    set_frame_crc(&input[1]);

    uint8_t frame[FRAME_SIZE] = {0};
    size_t received = 0;
    size_t complete_count = 0;
    ParserState state = WAIT_MAGIC_0;

    for (size_t i = 0; i < sizeof(input); i++) {
        bool complete = parser_push_byte(
            input[i], frame, &received, &state
        );

 if (complete) {
    complete_count++;
}    }

   assert(complete_count==1); // TODO 2: Assert exactly one frame was completed.
   assert(frame[0]==0x4E);
assert(frame[1]==0x52); // TODO 3: Assert frame[0] and frame[1] are correct.
  assert(state==WAIT_MAGIC_0);  // TODO 4: Assert the parser returned to WAIT_MAGIC_0.

    printf("PASS: repeated magic\n");
}

int main(void)
{
    test_valid_frame();
    test_garbage_before_frame();
    test_incomplete_frame();
    test_repeated_magic();
   test_lost_byte();
   printf("All parser tests passed!\n");
    return 0;
}
