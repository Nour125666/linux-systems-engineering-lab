# Parallel Test Runner (`ptrunner`)

**Module 4 — Linux Systems Engineering | POSIX Threads & Concurrency**

A small Linux CLI program that runs shell-based test commands from a manifest. It supports sequential execution and a two-worker parallel mode. The project demonstrates POSIX threads, bounded producer-consumer queues, condition variables, atomic counters, process spawning, and orderly shutdown.

## Build

Ubuntu / WSL2 with GCC, GNU Make, and pthread support:

```bash
make
# Equivalent manual build:
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -g -O0 -pthread ptrunner.c -o ptrunner
```

## Run

From the project directory, write a manifest with **one shell command per line**:

```text
true
python3 -c 'assert 2 + 2 == 4'
./tests/my_test_executable
```

Empty lines and lines beginning with `#` are ignored.

```bash
./ptrunner tests/manifest_pass.txt              # sequential
./ptrunner --parallel tests/manifest_pass.txt   # 2 concurrent workers
./ptrunner --queue-selftest                     # bounded-queue self-test
```

Each command runs through `/bin/sh -c` using `posix_spawn()`. A child exit code of zero is counted as PASS; nonzero is counted as FAIL. The runner returns exit code 0 only when the run succeeds and all tests pass; otherwise, it returns nonzero.

**Security:** Manifests contain executable shell commands. Run only trusted manifests, not unvalidated commands from external users.

## Design

```text
Main thread -> getline() -> strdup() -> bounded queue (8 slots)
                                           |               |
                                           v               v
                                      Worker 0        Worker 1
                                           |               |
                                   posix_spawn()    posix_spawn()
                                       waitpid()        waitpid()
                                           \               /
                                             atomic counters
```

- The producer creates a private `strdup()` copy of each command. A successful queue push transfers ownership to the queue; after a pop, the worker owns and frees that allocation.
- A mutex protects the circular buffer's indices, occupancy and closed flag.
- `not_empty` and `not_full` condition variables coordinate workers and the producer. `while` loops recheck their predicates after wakeups.
- Closing the queue broadcasts to waiters; workers drain accepted jobs before exiting.
- Workers launch tests via `posix_spawn()` and wait for their specific child PIDs using `waitpid()`.
- Atomic counters track completed passes and failures; main closes the queue and joins workers before reporting totals.
- The queue self-test runs 12 jobs against two workers and verifies the total consumption count.

## QA

Run the local regression script if present:

```bash
bash tests/qa_runner.sh
```

Build and exercise sanitizers separately:

```bash
make asan
ASAN_OPTIONS=detect_leaks=1 ./ptrunner_asan --queue-selftest
ASAN_OPTIONS=detect_leaks=1 ./ptrunner_asan --parallel tests/manifest_pass.txt

make tsan
./ptrunner_tsan --queue-selftest
./ptrunner_tsan --parallel tests/manifest_pass.txt
```

ThreadSanitizer support can depend on the Linux/WSL runtime. A clean test run does not prove that all races, deadlocks, or failure paths are impossible.

The Module 4 QA exercise covered passing/mixed/empty manifests, 40-job queue wraparound, repeated queue self-tests, ASan/UBSan and reported ThreadSanitizer checks, plus a four-command `sleep 1` concurrency benchmark. The reported WSL benchmark took 4.044 s sequentially and 2.019 s in parallel (~2.00× for that specific waiting workload).

## Current limitations

- Parallel worker count is fixed at 2 (`PARALLEL_WORKERS`); there is no `--jobs` option yet.
- No per-test timeout, child process-group cleanup, or interactive-test management. A hung test can hang the run.
- Output from separate tests can interleave; per-test log capture is not implemented.
- Individual command lines are run as shell commands; background processes and command chaining are governed by shell semantics.
- Tests should be independently runnable. Tests that share mutable files, ports, or hardware can interfere in parallel mode.
- The queue self-test checks total consumption, not a per-job uniqueness bitmap.
- Rare errors from mutex/condition-variable calls and failed `pthread_join()` calls need additional recovery handling for production-grade use.

This is a functional educational CLI tool, not a complete CI framework.
