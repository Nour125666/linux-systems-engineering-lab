# Module 6 — Linux Kernel Fundamentals

> **Status:** Module 6 exercises were completed in the course using an Ubuntu QEMU guest. This folder currently documents the work; **the original C source files and build/test evidence have not yet been uploaded**. The summaries below are not a replacement for that source.

## Objectives

Understand what changes when C code executes in the Linux kernel rather than in userspace, and practice the basic infrastructure used by real device drivers.

| Topic | Focus | Coursework file to import |
| --- | --- | --- |
| Kernel module lifecycle | Module initialization, exit, parameters, Kbuild, `insmod`, `rmmod` and `dmesg` | `hello_kernel.c` |
| Kernel memory management | `kmalloc`, `kcalloc`, `kfree` and cleanup on failure | `sensor_memory.c` |
| Kernel threads | `kthread_run`, stopping threads and lifetime management | `kernel_worker.c` |
| Deferred work | Workqueue scheduling and worker execution context | `workqueue_lab.c` |
| Wait queues | Blocking until a condition changes and wakeups | `waitqueue_lab.c` |
| Debugging | Kernel diagnostics, error returns and failure-path investigation | `debug_lab.c` |

Additional subjects studied: **mutexes, spinlocks and atomic-context restrictions, sleepable vs non-sleepable contexts, and PREEMPT_RT concepts**. These topics do not imply that every mechanism has a corresponding code artifact in this repository.

## Execution model

```text
                    USERSPACE
                 insmod / rmmod
                        |
                        v
                Linux module loader
                        |
                module init / exit
                        |
           +------------+-------------+
           |            |             |
           v            v             v
       Memory API    Kernel tasks   Deferred work
     kmalloc/kfree    kthreads       workqueues
           |            |             |
           +------------+-------------+
                        |
                    Debugging
                 dmesg / tracing
```

## What this module prepares you for

Module 6 establishes the kernel mechanisms used in the subsequent [ADS1115 I²C/IIO driver project](https://github.com/Nour125666/linux-ads1115-iio-driver): per-device state, a sleepable mutex-protected conversion path, driver lifecycle, and structured error handling.

## Coursework environment

- Host: Ubuntu under WSL2
- Virtualized Linux guest: Ubuntu in QEMU
- Kernel used during the coursework: `6.8.0-149-generic`
- C and the Linux kernel external-module build system (Kbuild)

## Source upload checklist

The original labs belong here once copied from the development machine:

```text
month-06/
├── README.md
├── lesson-02-first-module/       # hello_kernel.c and its Makefile, if present
├── ...                           # retain original lesson directory names
└── lesson-08-kernel-debugging/  # debug_lab.c and its Makefile, if present
```

These paths are illustrative, **not** a claim that those files are already committed. Keep the original coursework file layout and Makefiles when uploading.

Do not commit `.ko`, `.o`, `.cmd`, `Module.symvers`, or generated kernel build artifacts.

### Build and run an individual lab

From an original lab folder with its own external-module `Makefile`, inside a guest with matching headers installed:

```bash
make -C /lib/modules/$(uname -r)/build M="$PWD" modules
sudo insmod ./<module-name>.ko
sudo dmesg | tail -30
sudo rmmod <module-name>
```

Replace the placeholders with the actual module names and follow each lab's specific instructions. Kernel modules must be built for the target kernel; do not attempt to insert guest modules into the WSL2 host kernel.

## Scope and evidence

The coursework history records exercises and QEMU testing for these subjects. **This repository cannot independently reproduce those labs until the original source code, Makefiles and (where useful) test instructions are committed.** This README intentionally avoids inventing implementations or test logs.
