# Module 5 — three guided GDB investigations

These are intentionally buggy learning programs. No corrections are included.

## Compile (WSL2 / Ubuntu)

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -g -O0 debug_min.c -o debug_min
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -g -O0 debug_crash.c -o debug_crash
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -g -O0 -pthread debug_hang.c -o debug_hang
code .
```

In VS Code, choose the matching launch configuration from Run and Debug, then press F5. Build again after edits.

1. **Wrong result:** Expected minimum = 3250 mV. Investigate initialization, conditions, and loop state.
2. **Crash:** Run until SIGSEGV, inspect the call stack, and trace the pointer back to its source.
3. **Hang:** Run until it stops making progress, press Pause in VS Code, and inspect the queue's count, capacity, call stack, and number of threads. Use Stop to terminate; it will not complete normally without a correction.

For the third case, the demo intentionally omits detailed pthread return-value checks so that the queue-state bug remains isolated. Production C code should check pthread error codes and handle shutdown explicitly.

VS Code debug console: evaluate `minimum_mv`, `frame`, `queue->count`, etc. with ordinary expressions, or use `-exec bt` and `-exec info threads` for GDB commands.
