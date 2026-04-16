# Container Runtime OS

A lightweight multi-container runtime implemented in C on Linux, integrating user-space process management with kernel-space monitoring.

## Overview

This project demonstrates core operating system concepts including process isolation, inter-process communication, logging, kernel interaction, and resource cleanup.

A supervisor process manages multiple containers concurrently, ensuring isolation using namespaces and `chroot`, while communicating through FIFO-based IPC. A kernel module is used to monitor container processes via IOCTL.

---

## Key Features

- Container isolation using UTS and mount namespaces with `chroot`
- FIFO-based IPC (`cmd_pipe`) for command handling
- Pipe-based logging system for container output
- Kernel module integration via character device and IOCTL
- Proper resource cleanup with no zombie processes

---

## System Components

### User Space (engine.c)

- Long-running supervisor process  
- Container lifecycle management (`start`, `stop`, `ps`)  
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

## Execution Steps

### Build
make


### Load Kernel Module

sudo insmod monitor.ko
sudo mknod /dev/container_monitor c 239 0
sudo chmod 666 /dev/container_monitor


### Prepare Runtime

mkfifo cmd_pipe
gcc engine.c -o engine


### Run Supervisor

sudo ./engine


### Start Containers (new terminal)

echo "start alpha" > cmd_pipe
echo "start beta" > cmd_pipe
echo "ps" > cmd_pipe


### View Logs

cat alpha.log


### Stop Container

echo "stop alpha" > cmd_pipe


### Cleanup

sudo rmmod monitor


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

## Limitations

- Memory limits are logged but not enforced  
- No custom scheduling algorithm implemented  
- Simplified model of container runtime  

---

## Project Structure


engine.c
monitor.c
monitor_ioctl.h
Makefile
rootfs-alpha/
rootfs-beta/
README.md

---

## Conclusion

This project demonstrates a functional container runtime combining user-space container management with kernel-space monitoring. It highlights key OS concepts while ensuring reliable execution and complete resource cleanup.
