#ifndef MONITOR_IOCTL_H
#define MONITOR_IOCTL_H

#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <sys/types.h>
#endif

#include <linux/ioctl.h>

struct monitor_request {
    pid_t pid;
    int soft_limit;
    int hard_limit;
};

#define REGISTER_PROCESS _IOW('a', 'a', struct monitor_request)

#endif
