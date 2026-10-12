# Day 1 — Linux Environment Investigation

**Date:** 2026-10-12

## 1. System information

- Platform: WSL2
- Architecture: x86_64
- Kernel: 6.18.33.2-microsoft-standard-WSL2
- PID 1: systemd

## 2. ELF investigation

Command: `readelf -l /usr/bin/ls`

Observations:
- The executable is an ELF position-independent executable (`ET_DYN`).
- The program header table contains 15 entries.
- The executable specifies the dynamic program interpreter `/lib64/ld-linux-x86-64.so.2`.
- Loadable segments have different read/write/execute permissions.
- One writable segment requires 1,048 more bytes in memory than are backed by file contents.

## 3. File descriptors

Command: `ls -l /proc/$$/fd`

Observations:
- Standard input, standard output, and standard error reference `/dev/pts/0`.
- An additional descriptor numbered 255 was open.
- These descriptors are references to open resources within the shell process.

## 4. Questions to investigate

- How does ELF loading differ from loading firmware onto a microcontroller?
- What is the difference between an ELF section and an ELF segment?
- Why are virtual addresses used instead of directly accessing physical addresses?
- What information does `/proc/PID/fdinfo` provide?

## 5. References

- https://man7.org/linux/man-pages/man5/elf.5.html
- https://man7.org/linux/man-pages/man5/proc_pid_fd.5.html
- https://man7.org/linux/man-pages/man2/open.2.html
