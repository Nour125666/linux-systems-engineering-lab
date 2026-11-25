# 🐧 Linux Systems Engineering Lab

> A progressive, hands-on Linux engineering portfolio exploring **C, POSIX, system calls, processes, multithreading, synchronization, debugging, kernel modules and device driver development**.

This repository documents my practical journey from embedded electronics and hardware development toward **Linux systems programming and Embedded Linux engineering**.

The objective is to understand Linux beyond the command line: how applications interact with the kernel, how processes and threads are managed, how resources are synchronized, how failures are diagnosed, and how kernel code is developed and integrated.

The repository contains coursework exercises, progressively more complex applications, debugging challenges and introductory kernel modules.

**The larger ADS1115 Linux driver and Yocto communication gateway are maintained in separate repositories and linked below.**

---

## 🚀 Project at a Glance

**Primary Language:** `C`  
**Operating System:** `Linux` `Ubuntu`  
**Programming Interfaces:** `POSIX` `Linux System Calls`  
**Concurrency:** `pthreads` `Mutexes` `Condition Variables` `Atomics`  
**IPC:** `Pipes` `Unix Domain Sockets`  
**Networking:** `TCP Sockets`  
**Debugging:** `GDB` `strace` `AddressSanitizer` `perf`  
**Kernel Development:** `Kbuild` `Kernel Modules` `kthreads` `Workqueues` `Wait Queues`  
**Development Environment:** `WSL2` `Ubuntu` `QEMU`  
**Build Tools:** `GCC` `GNU Make` `Git`

### Current Repository Status

| Area | Status |
|---|---|
| Linux foundations and system calls | ✅ Source published |
| UART, parsing and IPC | ✅ Source published |
| Process management and signals | ✅ Source published |
| POSIX threads and concurrency | ✅ Source published |
| Debugging and reliability exercises | ✅ Source published |
| Kernel fundamentals — Module 6 | ✅ Source and Makefiles published |
| Character-device introduction — Module 7 | ✅ Initial exercise published |
| ADS1115 I²C/IIO driver project | ✅ Separate repository |
| Kernel debugging and BSP coursework | 🔄 Continuing |

**Scope:** Educational and experimental code. Publication of an exercise does not imply production readiness or comprehensive automated validation.

---

# 🧩 Linux Systems Architecture

The coursework explores how a Linux system is organized, from userspace applications down to kernel mechanisms and hardware interfaces.

```text
                     USERSPACE
┌────────────────────────────────────────────┐
│                                            │
│       C Applications / Linux Services      │
│                                            │
│  Files   Processes   Threads   Networking  │
│    │        │          │          │        │
│  read()   fork()   pthreads()   socket()   │
│                                            │
└─────────────────────┬──────────────────────┘
                      │
                      ▼
              SYSTEM CALL INTERFACE
                      │
                      ▼
                     KERNEL
┌────────────────────────────────────────────┐
│                                            │
│   Process Scheduler     Memory Management  │
│                                            │
│   Virtual Filesystem    Networking Stack   │
│                                            │
│   Synchronization       Device Drivers     │
│                                            │
│   Kernel Threads        Workqueues         │
│                                            │
└─────────────────────┬──────────────────────┘
                      │
                      ▼
                    HARDWARE
             CPU / Memory / Devices
```

The first modules concentrate on the upper layers of this architecture. Later modules investigate how the kernel itself manages execution, memory, synchronization and device interfaces.

---

# 📚 Curriculum Overview

| Module | Focus | Main concepts |
|---|---|---|
| **01** | Linux Foundations | C, files, file descriptors, system calls, memory |
| **02** | Hardware-Facing Userspace | UART, TTY, binary protocols, CRC, IPC |
| **03** | Process Management | `fork`, `exec`, signals, child processes |
| **04** | Threads & Concurrency | pthreads, races, mutexes, condition variables |
| **05** | Debugging & Reliability | GDB, strace, profiling, hangs, crashes, timeouts |
| **06** | Kernel Fundamentals | Modules, kernel memory, kthreads, workqueues, wait queues |
| **07** | Device Driver Development | Character-device introduction and ADS1115 driver project |

The curriculum continues into kernel debugging, Device Tree, bootloaders, BSP development and Embedded Linux integration.

---

# 📁 Module 1 — Linux Foundations

[Explore Module 1](month-01/)

The first module introduces Linux systems programming through direct interaction with operating-system services.

### Topics

- ELF executables and compilation
- Linux system calls
- File descriptors and open file descriptions
- `open()`, `read()`, `write()` and `close()`
- File offsets and error handling
- Process memory organization
- Basic tracing and debugging

### Practical Work

The module includes file-copy exercises, descriptor-offset experiments, telemetry inspection and a binary diagnostic application.

**Featured application:** [BMS Diagnostic Tool](month-01/module-01-project/)

The project provides a practical context for parsing binary data, validating input and returning meaningful errors.

```text
Binary File
     │
     ▼
open() / read()
     │
     ▼
Header / Data Validation
     │
     ▼
BMS Diagnostic Logic
     │
     ▼
Result / Error Reporting
```

Supporting shell tests are available in the project's `tests/` directory.

---

# 🔌 Module 2 — UART, Binary Protocols & IPC

[Explore Module 2](month-02/module-02-project/)

Module 2 focuses on communicating with external devices through standard Linux userspace interfaces.

### Topics

- UART and the Linux TTY subsystem
- Serial configuration and `termios`
- Binary frame decoding
- CRC validation
- Packet boundaries and incomplete data
- Unix-domain sockets
- Local client/server communication

### Practical Work

The module contains a BMS-oriented communication service with separate components for serial acquisition, parsing, CRC and IPC.

```text
             Serial Device
                   │
                   ▼
             Linux TTY API
                   │
                   ▼
              Serial Reader
                   │
                   ▼
              Frame Parser
                   │
                   ▼
              CRC Validation
                   │
                   ▼
              BMS Service
                   │
                   ▼
              IPC Interface
```

Representative source files:

- [`serial_reader.c`](month-02/module-02-project/serial_reader.c)
- [`frame_parser.c`](month-02/module-02-project/frame_parser.c)
- [`crc8.c`](month-02/module-02-project/crc8.c)
- [`ipc_server.c`](month-02/module-02-project/ipc_server.c)
- [`test_frame_parser.c`](month-02/module-02-project/test_frame_parser.c)

This module connects electronics-oriented serial communication with Linux application architecture.

---

# ⚙️ Module 3 — Processes, Execution & Signals

[Explore Module 3](month-03/module-03-processes/)

This module explores how Linux creates, executes, supervises and terminates processes.

### Topics

- `fork()` and process creation
- `exec()` and executable replacement
- Parent and child processes
- Process memory after `fork()`
- `wait()` and `waitpid()`
- Exit status handling
- UNIX signals
- Process termination and supervision

### Process Lifecycle

```text
                Parent Process
                       │
                       │ fork()
                       ▼
                 Child Process
                       │
                       │ exec()
                       ▼
                 New Program
                       │
                       │ exit()
                       ▼
                 Exit Status
                       │
                       │ waitpid()
                       ▼
                Parent Continues
```

The [`processlab`](month-03/module-03-processes/processlab/) exercise brings several of these concepts together.

---

# 🧵 Module 4 — POSIX Threads & Concurrency

[Explore Module 4](month-04/)

This module examines concurrent execution, shared memory and synchronization.

### Topics

- POSIX thread creation and joining
- Thread lifecycle
- Shared state and race conditions
- Mutexes and critical sections
- Condition variables
- Producer-consumer coordination
- Bounded queues
- Atomic counters
- Process execution from worker threads

### Featured Project: Parallel Test Runner

[View the Parallel Test Runner](month-04/parallel-test-runner-starter/)

The application executes test commands from a manifest using sequential or parallel execution.

```text
                     MAIN THREAD
                          │
                          ▼
                    Read Manifest
                          │
                          ▼
                  Bounded Job Queue
                    /           \
                   /             \
                  ▼               ▼
             Worker 0         Worker 1
                  │               │
                  ▼               ▼
             Spawn Test       Spawn Test
                  │               │
                  ▼               ▼
              waitpid()        waitpid()
                   \             /
                    \           /
                     ▼         ▼
                   PASS / FAIL
                       │
                       ▼
                 Result Summary
```

The implementation uses a mutex and condition variables to coordinate access to the bounded queue.

This project demonstrates why synchronization is necessary when multiple threads share resources and how concurrent applications can shut down cleanly.

### Related Experiments

[`thread_intro.c`](month-04/module-04-pthreads/day01/thread_intro.c) · [`race_lab.c`](month-04/module-04-pthreads/day01/race_lab.c) · [`race_mutex.c`](month-04/module-04-pthreads/day01/race_mutex.c)

---

# 🔍 Module 5 — Debugging, Profiling & Reliability

[Explore Module 5](month-05/section-02/)

Module 5 focuses on identifying and understanding software failures rather than simply writing code that works under normal conditions.

### Topics

- System-call tracing with `strace`
- Source-level debugging with GDB
- Memory-error investigation
- Crash analysis
- Hangs and blocked execution
- CPU profiling
- Lock-related failures
- Network communication failures
- Graceful shutdown
- Resource lifetime and cleanup

### Debugging Methodology

```text
               Observe Failure
                      │
                      ▼
             Reproduce Behavior
                      │
                      ▼
              Collect Evidence
                      │
            ┌─────────┼─────────┐
            ▼         ▼         ▼
          strace     GDB       perf
            │         │         │
            └─────────┼─────────┘
                      ▼
               Identify Cause
                      │
                      ▼
                Implement Fix
                      │
                      ▼
                Verify Result
```

The repository contains dedicated exercises for crashes, hangs, reconnect behavior, thread synchronization, TCP communication, memory lifetime and process shutdown.

### Representative Labs

- [`debug_crash.c`](month-05/module5_debugging_cases/module5_debugging_cases/debug_crash.c)
- [`worker_lock_lab.c`](month-05/section-02/challenge-05/worker_lock_lab.c)
- [`tcp_bms_lab.c`](month-05/section-02/challenge-07/tcp_bms_lab.c)
- [`shutdown_lab.c`](month-05/section-02/challenge-09/shutdown_lab.c)
- [`bms_hang.c`](month-05/section-02/final-investigation/bms_hang.c)
- [`bms_profile_lab.c`](month-05/section-03/lab-01/bms_profile_lab.c)

These labs explore failures that are particularly relevant to long-running embedded Linux applications.

---

# 🐧 Module 6 — Linux Kernel Fundamentals

[Explore Module 6](month-06/)

Module 6 transitions from userspace programming to code executing inside the Linux kernel.

### Topics

- Loadable kernel modules
- Module initialization and exit
- Kernel parameters
- Kbuild and external modules
- Kernel memory allocation
- Kernel threads
- Mutex-protected shared data
- Workqueues and deferred execution
- Wait queues
- Kernel logging and initialization errors

### Kernel Execution Model

```text
              Linux Kernel
                    │
                    ▼
             Module Loading
                    │
                    ▼
             module_init()
                    │
          ┌─────────┼──────────┐
          │         │          │
          ▼         ▼          ▼
       Memory     Kernel     Deferred
      Allocation  Threads      Work
          │         │          │
          └─────────┼──────────┘
                    │
                    ▼
             Module Cleanup
                    │
                    ▼
             module_exit()
```

### Kernel Labs

| Lab | Implementation | Main concept |
|---|---|---|
| First Kernel Module | [`hello_kernel.c`](month-06/lesson-02-first-module/hello_kernel.c) | Initialization, exit, module parameters |
| Kernel Memory | [`sensor_memory.c`](month-06/lesson-03-kernel-memory/sensor_memory.c) | Allocation and cleanup |
| Kernel Concurrency | [`kernel_worker.c`](month-06/lesson-04-kernel-concurrency/kernel_worker.c) | Kthreads and mutexes |
| Workqueues | [`workqueue_lab.c`](month-06/lesson-06-workqueues/workqueue_lab.c) | Deferred kernel execution |
| Wait Queues | [`waitqueue_lab.c`](month-06/lesson-07-waitqueues/waitqueue_lab.c) | Producer-consumer synchronization |
| Kernel Debugging | [`debug_lab.c`](month-06/lesson-08-kernel-debugging/debug_lab.c) | Initialization failures and cleanup |

Every listed lab has its corresponding Makefile in the repository.

These exercises establish the foundations for more advanced driver development, including understanding execution contexts, resource ownership and synchronization inside the kernel.

---

# ⚡ Module 7 — Device Driver Development

The introductory coursework currently includes a [character-device registration lab](month-07/lesson-01/m7_chardev.c).

The lab explores:

- Allocating device numbers
- Initializing a `struct cdev`
- Registering a character device
- Defining file operations
- Implementing an `open()` callback
- Releasing kernel resources during module removal

### Related Project: ADS1115 Linux I²C/IIO Driver

The larger driver project is maintained separately:

**[ADS1115 Linux I²C/IIO Driver](https://github.com/Nour125666/linux-ads1115-iio-driver)**

It explores Linux device-driver integration beyond basic character-device registration.

```text
                USERSPACE
                    │
                    ▼
             IIO Sysfs API
                    │
                    ▼
              IIO Subsystem
                    │
                    ▼
             ADS1115 Driver
                    │
             Mutex Protection
                    │
                    ▼
              I²C Subsystem
                    │
                    ▼
               ADS1115 ADC
```

Its implementation includes I²C driver registration, IIO channel callbacks, register access, synchronous conversion logic and mutex-based serialization.

The QEMU/`i2c-stub` tests demonstrate software integration; real ADC hardware validation remains separate.

---

# 🔐 Concurrency: Userspace vs Kernelspace

One of the main themes connecting these modules is synchronization.

The concepts are related, but the APIs and execution constraints are different.

| Userspace | Kernelspace |
|---|---|
| `pthread_create()` | `kthread_run()` |
| `pthread_mutex_lock()` | `mutex_lock()` |
| POSIX condition variables | Kernel wait queues |
| Userspace worker threads | Kernel workqueues / kthreads |
| `malloc()` / `free()` | `kmalloc()` / `kfree()` |
| `printf()` / application logging | `pr_info()` / `pr_err()` |
| GDB and process tracing | Kernel logs and kernel tracing |

A central lesson of kernel programming is that **execution context matters**: sleepable operations cannot be used indiscriminately inside hard interrupt or other atomic contexts.

---

# 🧪 Development & Test Environment

Most userspace coursework was developed under Ubuntu running in WSL2.

Kernel-module experiments used an isolated Linux environment under QEMU.

```text
                  Windows Host
                       │
                       ▼
                  Ubuntu WSL2
                       │
             ┌─────────┴─────────┐
             │                   │
             ▼                   ▼
        Userspace Labs      QEMU Linux Guest
             │                   │
             ▼                   ▼
        GCC / Make         Kernel Headers
             │                   │
             ▼                   ▼
       POSIX Programs      Kbuild / Modules
             │                   │
             └─────────┬─────────┘
                       │
                       ▼
                Logs / Debugging
```

QEMU provides an isolated environment for loading and testing experimental kernel modules without using the Windows or WSL2 host kernel as the module target.

## Building an Example Userspace Project

The Module 4 parallel test runner includes its own Makefile.

```bash
cd month-04/parallel-test-runner-starter

make

./ptrunner tests/manifest_pass.txt

./ptrunner --parallel tests/manifest_pass.txt
```

## Building a Kernel Module

Run this example **inside a Linux environment with matching kernel headers**, such as the QEMU guest used for the coursework.

```bash
cd month-06/lesson-02-first-module

make

sudo insmod hello_kernel.ko
sudo dmesg | tail -20
sudo rmmod hello_kernel
```

Exact commands and expected behavior depend on the individual lab.

---

# 📂 Repository Structure

The following tree reflects the **tracked source files currently published on GitHub**. Generated executables, object files, trace outputs and other untracked local build artifacts are not included.

<details>
<summary><strong>Expand full repository tree</strong></summary>

```text
linux-systems-engineering-lab/
├── docs/
│   ├── learning-log.md
│   └── references.md
├── month-01/
│   ├── module-01-project/
│   │   ├── tests/
│   │   │   └── test_bms_diag.sh
│   │   ├── .gitignore
│   │   ├── bms_diag.c
│   │   ├── Makefile
│   │   └── README.md
│   └── week-01/
│       ├── day02/
│       │   ├── fd_offset_lab.c
│       │   ├── README.md
│       │   └── telemetry_inspector.c
│       ├── day03/
│       │   ├── fork_exec_lab.c
│       │   ├── main.c
│       │   ├── memory_lab.c
│       │   ├── memory_sections.c
│       │   ├── process_lab.c
│       │   ├── sensor.c
│       │   ├── sensor.h
│       │   └── signal_lab.c
│       ├── debugging/
│       │   ├── day01_copy.md
│       │   └── day01_strace.md
│       ├── notes/
│       │   └── day01_environment.md
│       ├── src/
│       │   ├── .gitkeep
│       │   ├── broken_copy.c
│       │   └── copy.c
│       ├── tests/
│       │   ├── .gitkeep
│       │   └── test_copy.sh
│       ├── .gitignore
│       ├── copy.c
│       ├── environment.md
│       └── README.md
├── month-02/
│   └── module-02-project/
│       ├── .gitignore
│       ├── bms_service.c
│       ├── crc8.c
│       ├── crc8.h
│       ├── frame_parser.c
│       ├── frame_parser.h
│       ├── ipc_client.c
│       ├── ipc_server.c
│       ├── README.md
│       ├── serial_config.c
│       ├── serial_reader.c
│       ├── test_crc8.c
│       └── test_frame_parser.c
├── month-03/
│   └── module-03-processes/
│       ├── processlab/
│       │   ├── ignore_term.c
│       │   ├── processlab.c
│       │   └── README.md
│       ├── exec_lab.c
│       ├── fork_lab.c
│       ├── fork_memory.c
│       ├── fork_wait.c
│       └── signal_lab.c
├── month-04/
│   ├── module-04-pthreads/
│   │   └── day01/
│   │       ├── four_workers.c
│   │       ├── race_lab.c
│   │       ├── race_mutex.c
│   │       └── thread_intro.c
│   └── parallel-test-runner-starter/
│       ├── tests/
│       │   ├── manifest_empty.txt
│       │   ├── manifest_mixed.txt
│       │   ├── manifest_parallel.txt
│       │   ├── manifest_pass.txt
│       │   ├── qa_runner.sh
│       │   ├── test_arithmetic.c
│       │   ├── test_fail.sh
│       │   ├── test_ok.sh
│       │   └── test_python.py
│       ├── .gitignore
│       ├── Makefile
│       ├── ptrunner.c
│       └── README.md
├── month-05/
│   ├── module5_debugging_cases/
│   │   └── module5_debugging_cases/
│   │       ├── debug_crash.c
│   │       ├── debug_hang.c
│   │       ├── debug_min.c
│   │       └── README.md
│   ├── section-02/
│   │   ├── challenge-02/
│   │   │   └── reconnect_lab.c
│   │   ├── challenge-03/
│   │   │   └── cpu_lab.c
│   │   ├── challenge-04/
│   │   │   └── worker_supervisor.c
│   │   ├── challenge-05/
│   │   │   └── worker_lock_lab.c
│   │   ├── challenge-06/
│   │   │   └── telemetry_stream_lab.c
│   │   ├── challenge-07/
│   │   │   └── tcp_bms_lab.c
│   │   ├── challenge-08/
│   │   │   └── bms_parser_lab.c
│   │   ├── challenge-09/
│   │   │   └── shutdown_lab.c
│   │   ├── challenge-10/
│   │   │   └── collector_lab.c
│   │   ├── final-investigation/
│   │   │   └── bms_hang.c
│   │   ├── final-lab/
│   │   │   └── telemetry_hang.c
│   │   ├── lesson-01/
│   │   │   └── serial_probe.c
│   │   ├── lesson-03/
│   │   │   └── poll_demo.c
│   │   ├── lesson-04/
│   │   │   └── webroot/
│   │   │       └── index.html
│   │   ├── lesson-06/
│   │   │   └── frame_lifetime.c
│   │   └── README.md
│   ├── section-03/
│   │   └── lab-01/
│   │       └── bms_profile_lab.c
│   ├── pointer_debug.c
│   ├── README.md
│   ├── scale_value.c
│   └── sum_debug.c
├── month-06/
│   ├── lesson-02-first-module/
│   │   ├── hello_kernel.c
│   │   └── Makefile
│   ├── lesson-03-kernel-memory/
│   │   ├── Makefile
│   │   └── sensor_memory.c
│   ├── lesson-04-kernel-concurrency/
│   │   ├── kernel_worker.c
│   │   └── Makefile
│   ├── lesson-06-workqueues/
│   │   ├── Makefile
│   │   └── workqueue_lab.c
│   ├── lesson-07-waitqueues/
│   │   ├── Makefile
│   │   └── waitqueue_lab.c
│   ├── lesson-08-kernel-debugging/
│   │   ├── debug_lab.c
│   │   └── Makefile
│   └── README.md
├── month-07/
│   └── lesson-01/
│       ├── m7_chardev.c
│       └── Makefile
├── .gitignore
├── bounded_queue.c
├── producer_consumer.c
└── README.md
```

</details>

---

# 🛠️ Skills Covered

### Systems Programming

`C` · `POSIX` · `System Calls` · `File Descriptors` · `Process Management`

### Concurrency

`pthreads` · `Mutexes` · `Condition Variables` · `Atomic Operations` · `Producer-Consumer`

### Communication

`UART` · `TTY` · `termios` · `Unix Sockets` · `TCP`

### Debugging & Performance

`GDB` · `strace` · `Sanitizers` · `perf` · `Crash Analysis` · `Failure Investigation`

### Linux Kernel

`Kernel Modules` · `Kbuild` · `Memory Allocation` · `Kernel Threads` · `Workqueues` · `Wait Queues`

### Development Tools

`GCC` · `GNU Make` · `Git` · `WSL2` · `QEMU`

---

# 🌐 Related Embedded Linux Projects

### 1. ADS1115 Linux I²C/IIO Driver

**[View Repository](https://github.com/Nour125666/linux-ads1115-iio-driver)**

An educational out-of-tree Linux kernel driver integrating the I²C and IIO subsystems to expose ADC measurements through standard Linux interfaces.

### 2. Yocto Sensor Communication Gateway

**[View Repository](https://github.com/Nour125666/Gateway-Yocto-App)**

A modular C/C++ communication stack integrated into Yocto, using systemd services, Unix-domain sockets, UART/TTY, TCP and MQTT.

Its userspace architecture was validated in a QEMU-based Embedded Linux environment. Physical board deployment remains future work.

Together, these projects extend the coursework into kernel-facing hardware integration and userspace system integration.

---

# 📌 Validation & Project Scope

The repository includes source code, individual Makefiles, some automated test scripts and debugging exercises.

The work has been developed primarily in Ubuntu/WSL2, with QEMU used for isolated kernel experiments.

**Important limitations:**

- Not every lab includes a standalone automated test report.
- The repository does not claim full continuous-integration coverage.
- Experimental modules are not production device drivers.
- Real hardware validation is documented separately where applicable.
- Rebuilding kernel modules requires a compatible target kernel and matching headers.

The purpose is to demonstrate the concepts explored, the code produced and the practical engineering progression.

---

# 🎯 Continuing the Curriculum

The next areas of study are:

- Advanced kernel debugging and execution contexts
- Kernel locking and driver resource management
- Device Tree and hardware description
- Bootloaders and ARM Linux boot flow
- Board Support Packages (BSP)
- Yocto image and platform integration
- Embedded Linux reliability and deployment

These subjects build on the systems programming and kernel foundations established in this repository.

---

# 🧠 Learning Approach & AI Assistance

This repository is part of a structured, AI-assisted self-study effort.

My professional background is primarily in electronics and embedded hardware, rather than Linux kernel software development. I began this curriculum without prior professional experience writing Linux device drivers.

AI tools were used to support explanations, implementation guidance, code generation, troubleshooting and review. The learning process also involved practical work with Linux tools, debugging exercises and integration testing.

The repository is intended to document that progression transparently, rather than present the coursework as production-grade or entirely independently authored software.

---

# 📄 License

No repository-wide software license has been declared at this stage. Unless a specific file states otherwise, redistribution and reuse permissions are not granted by the public availability of the source code.
