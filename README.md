# Linux Systems Engineering Lab

A hands-on C/Linux learning repository covering **systems programming, process management, concurrency, debugging, and Linux kernel fundamentals**.

The emphasis is on small reproducible exercises, understanding Linux APIs and execution models, and documenting the limits of each test.

## Curriculum and source index

| Module | Focus | Repository material |
| --- | --- | --- |
| 01 | Files, system calls, descriptors, memory and introductory debugging | [Month 01](month-01/) — file-copy labs and BMS diagnostics |
| 02 | UART/TTY, frame parsing, CRC, IPC and Linux services | [Month 02](month-02/module-02-project/) — BMS service and tests |
| 03 | Processes, `fork`, `exec`, signals and supervision | [Month 03](month-03/module-03-processes/) |
| 04 | Threaded execution, synchronization and test orchestration | [Month 04](month-04/parallel-test-runner-starter/) |
| 05 | Reliability, polling, sockets, process supervision and debugging exercises | [Month 05](month-05/section-02/) |
| 06 | Kernel modules, memory management, kthreads, workqueues, wait queues and kernel diagnostics | [Month 06](month-06/) — **overview added; original lab sources pending upload** |

This table reflects the files currently visible on GitHub and earlier coursework notes; it does not imply that all exercises have a full test report.

## Related practical projects

- **[ADS1115 Linux I²C/IIO driver](https://github.com/Nour125666/linux-ads1115-iio-driver):** out-of-tree kernel module, I²C client matching, raw/scale IIO channels, and mutex-protected synchronous reads. QEMU/`i2c-stub` software integration tested; physical ADC validation is pending.
- **[Yocto gateway application](https://github.com/Nour125666/Gateway-Yocto-App):** C/C++ userspace services, UART/TTY, Unix-domain sockets, TCP/MQTT, `systemd`, and BitBake application integration.

The driver and gateway have **separate repositories**, so their source is linked here rather than duplicated.

## Main skills

```text
Linux systems engineering
├── Userspace C / POSIX / file descriptors
├── Processes, signals and IPC
├── Threads, synchronization and event loops
├── Debugging and failure analysis
└── Kernel development
    ├── Kernel modules / Kbuild
    ├── Kernel memory and resource lifetime
    ├── Kthreads, workqueues and wait queues
    └── Driver subsystem integration (related project)
```

## Development environment

- Ubuntu under WSL2 for source development
- GCC, GNU Make, Git, GDB, `strace` and Linux command-line tools
- Ubuntu in QEMU for kernel-module experiments
- Yocto/QEMU used in the separately linked gateway project

## Working with the labs

Each module folder contains some combination of source files, Makefiles, tests and Markdown notes. Check the local folder README before building.

**For kernel modules:** build against the *guest/target* kernel headers. The WSL2 host's kernel is not the same as the QEMU guest kernel.

## Validation and limitations

- Existing userspace files are published, but their presence alone does not prove every scenario was retested on the current platform.
- Module 6's conceptual overview is published; original lab sources and test records still need to be synchronized from the development machine.
- Physical hardware is not emulated by simple I²C register stubs; see the ADS1115 project for detailed limits.

## Learning approach

This repository documents an AI-assisted self-study program. Code, explanations and debugging were developed with guidance, and practical exercises were carried out in Linux environments. The goal is to make the learning process inspectable, not to present the exercises as production software or independently authored commercial work.

See [Learning log](docs/learning-log.md).
