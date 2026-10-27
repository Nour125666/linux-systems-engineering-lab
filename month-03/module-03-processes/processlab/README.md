# ProcessLab — Linux Process Supervisor

A small process supervisor written in C as part of my Linux Systems Engineering Lab.

## Features

- Launches a child process using fork() and execvp()
- Monitors child termination using waitpid() with WNOHANG
- Handles SIGINT and SIGTERM using sigaction()
- Requests graceful shutdown using SIGTERM
- Waits up to five seconds before escalating to SIGKILL
- Reaps the child and reports its termination status
- Uses CLOCK_MONOTONIC for shutdown timing

## Build

gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -g \
    processlab.c -o processlab

## Run

./processlab sleep 3

## Test shutdown escalation

Build the test child:

gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -g \
    ignore_term.c -o ignore_term

Terminal 1:

./processlab ./ignore_term

Terminal 2:

pgrep -a -x processlab
kill -TERM <SUPERVISOR_PID>

Expected behavior:
1. ProcessLab receives SIGTERM.
2. ProcessLab forwards SIGTERM to its child.
3. The child ignores SIGTERM.
4. After five seconds, ProcessLab sends SIGKILL.
5. ProcessLab reaps the child and reports signal 9.

## Limitations

This is an educational single-child supervisor, not a replacement
for systemd or a production service manager.

It does not yet implement:
- Automatic restarts
- Health checks
- Process-group supervision
- Full error-path cleanup
- Race-free startup and signal coordination

## Concepts Practiced

fork(), execvp(), waitpid(), signal handling, process lifecycle,
zombie processes, graceful shutdown, and escalation.
