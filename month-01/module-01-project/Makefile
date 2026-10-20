
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -g
TARGET = bms_diag

.PHONY: all test clean

all: $(TARGET)

$(TARGET): bms_diag.c
	$(CC) $(CFLAGS) bms_diag.c -o $(TARGET)

test: $(TARGET)
	bash tests/test_bms_diag.sh

clean:
	rm -f $(TARGET)
