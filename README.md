# Container Runtime OS

A lightweight multi-container runtime implemented in C on Linux, integrating user-space process management with kernel-space monitoring.

## Overview

This project demonstrates core operating system concepts including process isolation, inter-process communication, logging, kernel interaction, scheduling behavior, and resource cleanup.

A supervisor process manages multiple containers concurrently, ensuring isolation using namespaces and chroot, while communicating through FIFO-based IPC. A kernel module is used to monitor container processes via IOCTL.

The runtime is also used as an experimental platform to study Linux scheduling behavior under different workloads and configurations.

---

## Key Features

- Container isolation using UTS and mount namespaces with chroot  
- FIFO-based IPC (`cmd_pipe`) for command handling  
- Pipe-based logging system for container output  
- Kernel module integration via character device and IOCTL  
- Scheduler experiments with CPU-bound and I/O-bound workloads  
- Proper resource cleanup with no zombie processes  

---

## System Components

### User Space (engine.c)

- Long-running supervisor process  
- Container lifecycle management (start, stop, ps)  
- IPC using FIFO  
- Logging via pipes  
- IOCTL communication with kernel module    

---

### Kernel Space (monitor.c)

- Character device: `/dev/container_monitor`  
- Handles IOCTL calls from user space  
- Registers container PIDs  
- Logs monitoring information 

---

### Scheduler Experiments Conducted:

- Execution of **CPU-bound processes** with different priorities (nice values)  
- Execution of **CPU-bound and I/O-bound processes simultaneously**  
- Observation of CPU utilization and responsiveness  

## Execution Steps

### Build

```bash
make
```

### Load Kernel Module

```bash
sudo insmod monitor.ko
sudo mknod /dev/container_monitor c 239 0
sudo chmod 666 /dev/container_monitor
```

### Prepare Runtime

```bash
mkfifo cmd_pipe
gcc engine.c -o engine
```

### Run Supervisor

```bash
sudo ./engine
```

### Start Containers (new terminal)

```bash
echo "start alpha" > cmd_pipe
echo "start beta" > cmd_pipe
echo "ps" > cmd_pipe
```

### View Logs

```bash
cat alpha.log
```

### Stop Container

```bash
echo "stop alpha" > cmd_pipe
```

### Cleanup

```bash
sudo rmmod monitor
```

---

## Resource Cleanup (Task 6)

The system ensures proper cleanup of all resources:

- Child processes are reaped using `waitpid()`  
- No zombie processes remain after execution  
- Logging processes terminate correctly  
- File descriptors are closed properly  
- Kernel module releases resources on unload  

---

## Observations

- Containers run in isolated environments  
- Logging continuously captures container output  
- Kernel module successfully registers and tracks processes  
- System performs clean shutdown without residual resources  

---

## Project Structure

- engine.c
- monitor.c
- monitor_ioctl.h
- Makefile
- rootfs-alpha/
- rootfs-beta/
- README.md

---

## Conclusion

This project demonstrates a functional container runtime combining user-space container management with kernel-space monitoring. It highlights key OS concepts while ensuring reliable execution and complete resource cleanup.
