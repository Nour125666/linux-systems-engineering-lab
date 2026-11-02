CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Werror -g -O0 -pthread

.PHONY: all clean qa asan tsan

all: ptrunner

ptrunner: ptrunner.c
	$(CC) $(CFLAGS) $< -o $@

asan: ptrunner.c
	$(CC) -std=c11 -Wall -Wextra -Wpedantic -Werror -g -O1 -pthread -fsanitize=address,undefined -fno-omit-frame-pointer $< -o ptrunner_asan

tsan: ptrunner.c
	$(CC) -std=c11 -Wall -Wextra -Wpedantic -Werror -g -O1 -pthread -fsanitize=thread -fno-omit-frame-pointer $< -o ptrunner_tsan

qa: ptrunner
	bash tests/qa_runner.sh

clean:
	rm -f ptrunner ptrunner_asan ptrunner_tsan
