# Module 5 · Section 02 — Linux Diagnostics & Debugging

> **Goal:** diagnose Linux/C failures from observable evidence, explain the underlying OS mechanism, make the smallest sound correction, and prove the correction works.
>
> **Environment used:** Ubuntu on WSL2, GCC, GDB, `strace`, `ps`, `/proc`, `lsof`, `ss`, sanitizers, and systemd journal tools. Exercises use small BMS/telemetry-flavored C programs.

This is the **single reference page** for the Section 02 debugging labs. It summarizes the tools, how to choose them, what their output means, and what each challenge taught. Source files live alongside this README in the `challenge-*` directories when pushed from the local development environment.

## Contents

1. [A reproducible debugging workflow](#1-a-reproducible-debugging-workflow)
2. [Tool reference and command examples](#2-tool-reference-and-command-examples)
3. [Reading diagnostic evidence correctly](#3-reading-diagnostic-evidence-correctly)
4. [Debugging session index](#4-debugging-session-index)
5. [Detailed case studies](#5-detailed-case-studies)
6. [Choosing the first tool](#6-choosing-the-first-tool)
7. [Build, test, and repository hygiene](#7-build-test-and-repository-hygiene)
8. [Authoritative references](#8-authoritative-references)

---

## 1. A reproducible debugging workflow

**Observe → reproduce → isolate → explain → fix → verify.**

1. **State the symptom.** Include expected behavior, actual behavior, exit status, and elapsed time. A successful system call and a successful application outcome are different things.
2. **Reproduce the failure.** Keep the input, invocation, compiler flags, and environment consistent. Record what is deterministic and what varies with scheduling.
3. **Pick the smallest useful instrument.** Use `ps`/`/proc` for state, `strace` for syscalls, `ss`/`lsof` for endpoint/descriptor ownership, GDB for program control flow, and sanitizers for invalid memory access.
4. **Follow identifiers.** Record the PID/TID, FD, pipe inode, local/remote socket address, memory address, or signal mask that links the observations.
5. **Explain the mechanism, not merely the symptom.** For example: “`read()` blocks because another process still owns a write end” is better than “the program hangs.”
6. **Apply the narrowest correct fix.** Preserve the intended service behavior. Do not hide the symptom by killing the process, adding arbitrary delays, or disabling error checks.
7. **Verify independently.** Rebuild, rerun, inspect the relevant output/syscalls, and test at least one boundary or regression case.

**Suggested report format per challenge:** symptom → reproduction command → strongest evidence → root cause → code correction → verification → remaining limitations.

## 2. Tool reference and command examples

### 2.1 Process state: `ps`, `pgrep`, `pstree` and `/proc`

```bash
ps -o pid,ppid,stat,%cpu,wchan:24,comm -p "$pid"
ps -L -p "$pid" -o pid,tid,stat,%cpu,wchan:26,comm
ps -o pid,ppid,stat,wchan:24,comm -p "$parent" --ppid "$parent"
pgrep -a -P "$parent"
cat "/proc/$pid/status"
ls -l "/proc/$pid/fd"
ls "/proc/$pid/task"
```

| Observation | Meaning / next step |
| --- | --- |
| `R` | Running or runnable; investigate sustained CPU consumption with `top`/`ps`/`strace` or a profiler. |
| `S` | Interruptible sleep; **not automatically a hang**. Read `WCHAN` and the relevant syscalls. |
| `D` | Uninterruptible wait, often I/O-related; investigate the underlying resource. |
| `T` | Stopped/traced. |
| `Z` | Zombie: child exited but its termination status has not been reaped with `wait()`/`waitpid()`. |
| `WCHAN=hrtimer_nanosleep` | Sleeping on a timer, possibly expected. |
| `WCHAN=anon_pipe_read` | Waiting inside a pipe read; inspect all pipe writers. |
| `WCHAN=inet_csk_accept` | TCP server waiting to accept a connection. |
| `WCHAN` involving `futex` | Waiting on a userspace synchronization operation; could be ordinary contention, `pthread_join`, or deadlock. Investigate lock ownership/backtraces. |

`STAT` can contain additional modifiers (such as `l` for multithreaded processes). State and wait-channel observations are snapshots, not proof of the root cause. If the process is gone when inspected, check whether your command ran **after the process exited**.

### 2.2 File descriptors and ownership: `/proc/PID/fd`, `lsof`

```bash
ls -l "/proc/$pid/fd"
readlink "/proc/$pid/fd/3"
lsof -nP -p "$pid"
lsof -nP /path/to/telemetry.log
lsof -nP -iTCP:8765 -sTCP:LISTEN
```

Use these to answer **who owns an open resource**. Descriptor numbers are local to each process: FD 3 in two different processes may reference the same pipe/socket or completely different objects. Compare targets such as `pipe:[inode]` and `socket:[inode]`.

A successful `fork()` inherits open descriptors; a successful `execve()` preserves them unless `FD_CLOEXEC` is set. A pipe reader sees EOF **only when all references to its write end are closed**. This explains delayed EOF even when the “real” telemetry writer has already exited.

### 2.3 System-call evidence: `strace`

```bash
strace -f -tt -T -yy -s 128 -o trace.log ./program
strace -f -tt -T -yy -e trace=read,write,close,poll,pipe2 -o io.trace ./program
strace -e trace=openat,close -o fd.trace ./program
strace -f -e trace=clone,clone3,futex -o threads.trace ./program
strace -f -e trace=socket,bind,listen,accept,connect -o net.trace ./program
strace -c ./program
```

| Option | Use |
| --- | --- |
| `-f` | Follow child processes / traced threads. Essential after `fork()`. |
| `-tt` | Show wall-clock timestamps to correlate ordering and delays. |
| `-T` | Show time spent in each completed syscall. |
| `-yy` | Decode FDs (paths, sockets, pipe identifiers) where possible. |
| `-s 128` | Increase printed string length; output can still be truncated. |
| `-e trace=...` | Reduce noise and focus on a syscall family. |
| `-o FILE` | Keep interleaved output out of the terminal. |
| `-c` | Aggregate syscall counts/errors/time; **`% time` is not process CPU utilization**. |

Read the **return value**, not merely the syscall's presence:

```text
openat(..., "/missing", ...) = -1 ENOENT
poll(..., 1000) = 0 (Timeout)
read(3<pipe:[123]>, ..., 64) = 18
read(3<pipe:[123]>, ..., 64) = 0
clone(... <unfinished ...>
<... clone resumed>) = 8988
```

`ENOENT` is “file/path not found”; `poll=0` means timeout; `read=0` means EOF for this stream/pipe context; `clone=8988` means the parent successfully created child PID 8988. `<unfinished ...>` is **trace formatting for interleaved syscalls**, not an error. Find the matching `<... syscall resumed>` entry.

For GNU `cat` and other utilities, check for `splice()` when a `read/write`-only trace appears incomplete. Startup `read()` calls that load ELF shared libraries are not application telemetry.

**Caution:** Tracing affects timing. Debugger/trace-induced scheduling changes can mask races or partial-read symptoms. When testing termination, do not assume killing `strace` or `timeout` has also cleaned up every traced child; enumerate remaining processes.

### 2.4 Network sockets: `ss` + `lsof` + `strace`

```bash
ss -ltnp
ss -ltnp | grep ':47631'
lsof -nP -iTCP:47631
strace -e trace=socket,bind,listen,connect,accept -o client.trace ./client
```

Distinguish **local address + port** from peer address + port. `127.0.0.1:47631` and `127.0.0.2:47631` are different endpoints. The `0.0.0.0:*` shown under a LISTEN socket's **peer** column does not mean the server listens on every local interface. `ECONNREFUSED` means the attempted connection was refused; first verify the exact destination and listening endpoint before adding retries.

### 2.5 Source-level debugging: GDB

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -g -O0 program.c -o program
gdb -q ./program
```

```text
(gdb) break parse_frame
(gdb) run
(gdb) bt
(gdb) info args
(gdb) info locals
(gdb) next
(gdb) step
(gdb) finish
(gdb) print cell_count
(gdb) x/8xb frame
(gdb) info threads
(gdb) thread apply all bt
```

Use breakpoints and stepping to connect the **source line** with values, call stack, or synchronization state. `next` steps over calls; `step` enters calls; `finish` runs to function return. In a multithreaded deadlock, `thread apply all bt` helps distinguish a joining main thread from workers waiting on mutexes.

**Pitfalls:** Values of uninitialized locals are not valid evidence. Loop variables declared inside a loop may not be in scope at an earlier breakpoint. A function return observed via `finish` is not necessarily the value of a caller variable until the assignment has executed. GDB can alter process scheduling.

### 2.6 Memory and concurrency: ASan, UBSan, TSan

```bash
# AddressSanitizer + UndefinedBehaviorSanitizer
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror \
    -g -O0 -fno-omit-frame-pointer \
    -fsanitize=address,undefined program.c -o program_asan
./program_asan

# ThreadSanitizer (separate build; do not combine with ASan)
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror \
    -g -O1 -pthread -fsanitize=thread threads.c -o threads_tsan
./threads_tsan
```

ASan reports out-of-bounds access, use-after-free, and related memory faults; read the **READ/WRITE size**, faulting stack trace, allocation/free stack trace, and object-boundary description. UBSan covers selected undefined behaviors; TSan targets data races, with environment-dependent support. A negative run doesn't prove bug-free code.

Avoid running LeakSanitizer-based leak checks through `strace`/`ptrace` and expecting all tooling combinations to work reliably. Run sanitizers normally first; if necessary disable leak detection for a separate syscall trace and disclose that limitation.

### 2.7 Logs and timing: `journalctl`, `dmesg`, `time` and `timeout`

```bash
journalctl -t module5-bms --no-pager
journalctl -b --no-pager
dmesg | tail -n 40
time ./program
timeout -k 1s 5s ./program
```

`journalctl` shows systemd journal records (when journald is available); `dmesg` shows kernel-ring-buffer messages. Neither automatically proves that an old log entry caused the current failure. `time` measures wall-clock elapsed time (`real`) and process CPU time (`user`/`sys`). A program can take seven seconds of real time while using almost no CPU because it is blocked.

`timeout` is convenient for bounded experiments, but signals, shell pipelines, tracing, and descendant process behavior need explicit consideration. Always check remaining laboratory PIDs rather than assuming cleanup.

### 2.8 Signals and shutdown: `kill`, `/proc/PID/status`, `strace`

```bash
kill -TERM "$pid"
grep -E '^(State|SigPnd|ShdPnd|SigBlk|SigIgn|SigCgt):' "/proc/$pid/status"
strace -e trace=rt_sigaction,rt_sigprocmask -o signal.trace ./program
pgrep -a -u "$(id -u)" -x shutdown_lab
```

`SigPnd` is thread-pending, `ShdPnd` is process-shared pending, `SigBlk` is blocked delivery, `SigIgn` is ignored, and `SigCgt` is caught by installed handler(s). These are hexadecimal bitmasks: signal `N` corresponds to bit `N - 1`; `SIGTERM` (15) corresponds to `0x4000`.

A successful `kill -TERM` only confirms the signaling request succeeded; it does not prove a handler ran or a process exited. `SIGKILL` is a last-resort lab cleanup, **not** graceful shutdown. Scope `pkill` carefully so it doesn't affect unrelated processes.

### 2.9 `poll()`, blocking I/O and event loops

- `poll(..., 0)` returns immediately: repeating it in an empty loop can consume an entire CPU core.
- `poll(..., 1000)` waits up to about one second for activity.
- `POLLIN` indicates a read can make progress; `POLLHUP` may indicate peer closure.
- A `poll()` timeout does **not** mean a subsequent blocking `read()` is safe.
- A nonblocking `read()` returning `-1/EAGAIN` means “try again when ready,” not necessarily a fatal error.
- `read(fd, buf, 8)` may legally return fewer than eight bytes on a stream socket. Accumulate bytes until the protocol-defined frame length is reached; handle EOF and `EINTR`.

## 3. Reading diagnostic evidence correctly

| Evidence | **What it proves** | **What it does not prove by itself** |
| --- | --- | --- |
| `ps STAT=S` | Task was sleeping when sampled | That it is deadlocked |
| `WCHAN=futex...` | Task is waiting in a futex-related path | Which lock is responsible |
| `strace: <unfinished ...>` | The trace entry was split/interleaved | That the syscall failed |
| `write(2, ..., 34) = 34` | 34 bytes written to stderr (FD 2) | That 34 telemetry bytes were sent |
| `read(..., 8) = 3` | Three bytes returned on that call | That the sender failed to send the remaining five |
| `connect(...) = -1 ECONNREFUSED` | This connection attempt was refused | Whether a listener exists at a *different* address |
| `ASan: 0 bytes after 6-byte region` | Access was at the end boundary of a 6-byte allocation | That the input frame was the overflowing allocation |
| `ShdPnd=SigBlk=0x4000` | SIGTERM pending and blocked | That the signal was ignored |
| `time: real≈7s, user≈0s` | Long elapsed time with little CPU use | CPU saturation |

## 4. Debugging session index

These sessions are indexed by their course challenge number. **Verified** means a corrected run or user-reported successful verification exists; **diagnosed** means the root cause was established but an independent corrected run was not recorded in the course discussion.

| Session / file | Failure investigated | Strongest tools / evidence | Correction | Status |
| --- | --- | --- | --- | --- |
| Foundation: `serial_probe.c` | Missing device path | `strace openat() = -1 ENOENT` | Check/create intended path; distinguish absent device from access errors | Investigated |
| Foundation: FIFO + `cat` | Writer/reader synchronization; unexpected trace shape | `strace -tt -T -yy`, `splice()` | Understand blocking, EOF, and utility implementation | Investigated |
| Foundation: `poll_demo.c` | Timeout, readiness, EAGAIN, hangup and EOF | `poll`, `strace` | Use readiness result and correct read/EOF handling | Investigated |
| Foundation: file/socket ownership | Identify open file and TCP listener owners | `ps`, `/proc/PID/fd`, `lsof`, `ss` | Close/release resources and verify owner disappears | Investigated |
| Foundation: journal/kernel diagnostics | Attribute log entries correctly | `journalctl`, `dmesg` | Correlate exact event/time instead of assuming causality | Investigated |
| Foundation: `frame_lifetime.c` | Heap use-after-free | ASan allocation/free/invalid-access stack | Move data use before `free()`; separately resolve duplicate output | Verified |
| #1: `telemetry_hang.c` | Blocking pipe read after `poll` timeout | `ps` WCHAN, `/proc` FDs, `strace` | Reconsider blocking read after timeout / ensure correct writer closure | Diagnosed |
| #2: `reconnect_lab.c` | FD leak during reconnect/EOF loop | `strace openat/close`, `EMFILE`, `/proc` | Close FD on EOF return path | Verified |
| #3: `cpu_lab.c` | Busy-spin event loop | `ps %CPU`, `strace -c`, zero-timeout `poll` | Wait with a suitable nonzero timeout or event-driven design | Verified |
| #4: `worker_supervisor.c` | Zombie children | `ps Z`, PPIDs, missing `waitpid` | Reap children; design supervisor lifecycle | Diagnosed |
| #5: `worker_lock_lab.c` | Circular mutex deadlock | `ps -L`, futex waits, GDB thread backtraces | Consistent lock order **and** barrier placement before locks/removal | Diagnosed |
| #6: `telemetry_stream_lab.c` | Short read / incomplete 8-byte telemetry frame | `strace -f` showed 3+5 writes, first read 3; GDB changed timing | Loop to accumulate bytes; handle EOF/errors | Verified; 20 repeated runs |
| #7: `tcp_bms_lab.c` | TCP `ECONNREFUSED` despite live server | `ss`/`lsof` show `127.0.0.1:47631`; `strace` shows client `127.0.0.2:47631` | Align client address with server binding | Verified |
| #8: `bms_parser_lab.c` | Two-byte heap-buffer-overflow | ASan points to 6-byte `calloc` region; loop index reaches 3 for count 3 | Change `i <= cell_count` to `i < cell_count` | Diagnosed; fix identified |
| #9: `shutdown_lab.c` | Shutdown signal stays pending | `/proc` signal masks; `rt_sigprocmask(SIG_BLOCK, [TERM])` | Remove inappropriate persistent SIGTERM block / use intentional signal-handling design | Diagnosed; **artificial scenario** |
| #10: `collector_lab.c` | Correct telemetry, delayed EOF (~7 s) | `strace -f -yy`, `pipe:[inode]` ownership, `time` | Close unnecessary pipe write end in diagnostic child before `exec` | Verified |

## 5. Detailed case studies

### #1 — The pipe read that kept waiting

**Symptom:** after a `poll()` timeout, the application still hung. **Evidence:** `poll(...,1000)=0` followed by a blocking `read()`; `ps` showed `anon_pipe_read` and `/proc` showed the pipe's descriptors. **Mechanism:** no readable data was available, but a writer remained open, so blocking `read()` waited instead of returning EOF. **Lesson:** timeouts do not automatically change the semantics of subsequent I/O.

### #2 — The descriptor leak

**Symptom:** repeated reconnect attempts eventually failed with `EMFILE` (“too many open files”). **Evidence:** `openat("/dev/null")` returned successively larger FDs; missing `close()` on the EOF branch; after correction, the same FD was opened and closed repeatedly. **Mechanism:** early-return resource leak. **Fix:** close on every ownership exit path.

### #3 — The busy loop

**Symptom:** near-100% CPU with no useful work. **Evidence:** `ps` showed runnable state and high CPU; `strace -c` showed a huge number of `poll()` calls with a zero timeout. **Mechanism:** a nonblocking event-loop iteration repeated without waiting. **Fix:** wait for events or use a meaningful timeout. Check CPU%, not `strace -c`'s percentage of syscall time.

### #4 — Zombies

**Symptom:** exited workers remained in the process table as `Z`. **Evidence:** their parent was still alive and had not reaped them. **Mechanism:** exit status retained until `wait()/waitpid()`. **Fix:** reap children, including in shutdown/error paths. After reaping, the zombie PIDs disappear rather than become sleeping processes.

### #5 — Deadlock between worker mutexes

**Symptom:** collector and publisher stopped progressing. **Evidence:** three threads, futex-related waits, and two workers holding opposite mutexes while trying to acquire each other's mutex. **Mechanism:** circular wait. **Fix:** establish one lock order. In this exact test, merely changing lock order is insufficient if the barrier remains **after acquiring the first mutex**; move the barrier before locks or remove it as well.

### #6 — Incomplete stream frame

**Symptom:** receiver rejected an eight-byte frame after getting only three bytes. **Evidence:** sender issued `write(3)` then `write(5)`; receiver's first `read(8)` returned 3. Under GDB, the receiver paused while the sender completed both writes, so a later single read returned 8. **Mechanism:** treating a stream read as a complete protocol message. **Fix:** receive loop until eight bytes, EOF, or error. **Verification:** complete decoded frame, exit 0, and 20 repeated successes.

### #7 — Correct port, wrong address

**Symptom:** client got `ECONNREFUSED` while a listening server process was alive. **Evidence:** `ss`/`lsof` showed server `127.0.0.1:47631`; client `strace` showed destination `127.0.0.2:47631`. **Mechanism:** TCP endpoint mismatch. **Fix:** use the actual listener address. Blind retries would not correct a persistent configuration mismatch.

### #8 — One iteration past the heap allocation

**Symptom:** ASan reported a **2-byte WRITE** at the end of a six-byte allocation. **Evidence:** `calloc(3,sizeof(uint16_t))` allocated six bytes, but `for(i=0; i<=cell_count; ++i)` executed for `i=3`. **Mechanism:** invalid write to `cells[3]`; the input bytes at `frame[9]` and `frame[10]` were inside the input array but represented trailer bytes, not another cell. **Fix:** `i < cell_count`. GDB values observed before local initialization were not meaningful.

### #9 — Pending and blocked SIGTERM

**Symptom:** service continued sending heartbeats after `kill -TERM`. **Evidence:** `ShdPnd=SigBlk=SigCgt=0x4000` and `rt_sigprocmask(SIG_BLOCK,[TERM])`. **Mechanism:** SIGTERM could not be delivered to the installed handler while blocked, so the handler never set the shutdown flag. **Fix:** do not permanently block a signal that the program expects an asynchronous handler to process. **Limitation:** this lab deliberately inserted an unrealistic permanent block; a more realistic case would forget to restore a mask after a short critical section. Tracing/timeout cleanup also spawned confusing extra instances.

### #10 — Why EOF took seven seconds

**Symptom:** telemetry arrived correctly (18 bytes), but collector finished only after `sleep 7` exited. **Evidence:** `time` showed ~7 s real elapsed, low CPU; `strace` showed the telemetry worker exited and the collector waiting on `read(pipe)`; diagnostics child successfully executed `/usr/bin/sleep`. **Mechanism:** diagnostic child inherited the **pipe write end**, despite not using it; `execve` preserved that FD, preventing reader EOF. **Fix:** close both unneeded pipe ends in diagnostics child before `execlp` (or use suitable close-on-exec design). **Verification:** corrected lab completed without the seven-second delay.

**Two recurring `strace` traps from #10:** `<unfinished ...>` followed by `<... clone resumed> = PID` indicates a successful fork-like syscall, not failure. `execlp` can probe multiple PATH candidates returning `ENOENT` before a later `execve("/usr/bin/sleep",...) = 0` succeeds.

## 6. Choosing the first tool

| Symptom | Start with | Then ask |
| --- | --- | --- |
| Process appears frozen | `ps` + `WCHAN`, `strace -f -T` | Blocked in which syscall? Who owns the resource? |
| CPU usage unexpectedly high | `ps %CPU`, `strace -c` | Busy poll? Repeated errors? Actual CPU hotspot requires profiling. |
| “Too many open files” | `/proc/PID/fd`, `lsof`, `strace -e openat,close` | Which path leaks an FD? |
| Unexpectedly slow exit | `time`, `strace -f -yy -T` | Outstanding child? Inherited writer? Blocking wait? |
| Network connection refused | `ss -ltnp`, `lsof -iTCP`, `strace connect` | Exact client destination vs bound listener endpoint? |
| Unexpected child processes | `ps --ppid`, `waitpid` tracing | Reaped? Still alive? Inherited descriptors? |
| Threads never progress | `ps -L`, GDB `thread apply all bt` | Lock cycle, barrier problem, or legitimate wait? |
| Crash, corrupted data, UAF | ASan/UBSan, GDB | Invalid address, allocation lifetime, index bound? |
| SIGTERM ignored in practice | `/proc/PID/status`, `strace rt_sigprocmask` | Blocked, ignored, pending, or handler stuck? |
| System-service warning | `journalctl`, `dmesg` | Does PID/time/event actually match the incident? |

## 7. Build, test, and repository hygiene

### Compile individual exercises

```bash
cd month-05/section-02/challenge-10
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -g -O0 \
    collector_lab.c -o collector_lab
time ./collector_lab
```

For problems involving invalid memory, build a separate ASan/UBSan binary. For POSIX threads, include `-pthread`. Prefer source files and short, curated Markdown evidence over checked-in binaries, core dumps, multi-megabyte traces, or environment-specific temporary files.

### Publish source files from the local Ubuntu/WSL checkout

The README was published to GitHub directly. The completed `.c` files and local edits still need to be pushed **from the machine where they were created**:

```bash
cd ~/dev/linux-systems-engineering-lab
git status --short
git pull --ff-only origin main

# Stage the actual C sources, not executables or raw traces:
find month-05/section-02 -type f -name '*.c' -print0 |
  xargs -0 -r git add --

git status --short
git diff --cached --stat
git commit -m "Add Module 5 Section 2 debugging challenge sources"
git push origin main
```

If local modifications prevent pulling, inspect `git status` and preserve those changes first; do not force-reset or overwrite them. Review staged files before committing. Keep raw `*.trace`, `*.log`, test binaries, and debug executables out of commits unless a small, sanitized excerpt is intentionally part of a case study.

**Portfolio standard:** every submitted source file should match its README case study, be buildable, and ideally preserve the fixed behavior with an explanation of the original defect. Mark diagnosis-only cases accurately rather than claiming unrun regressions were tested.

## 8. Authoritative references

- Linux manual pages: [`strace(1)`](https://man7.org/linux/man-pages/man1/strace.1.html), [`proc(5)`](https://man7.org/linux/man-pages/man5/proc.5.html), [`pipe(7)`](https://man7.org/linux/man-pages/man7/pipe.7.html), [`poll(2)`](https://man7.org/linux/man-pages/man2/poll.2.html).
- Linux networking: [`socket(2)`](https://man7.org/linux/man-pages/man2/socket.2.html), [`connect(2)`](https://man7.org/linux/man-pages/man2/connect.2.html), [`read(2)`](https://man7.org/linux/man-pages/man2/read.2.html), [`execve(2)`](https://man7.org/linux/man-pages/man2/execve.2.html).
- Signals and processes: [`signal(7)`](https://man7.org/linux/man-pages/man7/signal.7.html), [`waitpid(2)`](https://man7.org/linux/man-pages/man2/wait.2.html), [`fork(2)`](https://man7.org/linux/man-pages/man2/fork.2.html).
- [GDB manual](https://sourceware.org/gdb/current/onlinedocs/gdb.html).
- [GCC instrumentation options](https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html).
- [systemd journalctl manual](https://www.freedesktop.org/software/systemd/man/latest/journalctl.html).

---

**Section 02 outcome:** practice moved from recognizing symptoms to identifying wait conditions, syscall return values, descriptor ownership, process/thread relationships, network endpoints, signal masks, and memory boundaries. **Next:** Section 03 — CPU profiling and performance measurement.
