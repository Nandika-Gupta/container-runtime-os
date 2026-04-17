#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include "monitor_ioctl.h"
#define DEVICE_NAME "container_monitor"
#define MAX_PROCESSES 100   

static int major;
static int pid_list[MAX_PROCESSES];
static int pid_count = 0;
static int device_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "Device opened\n");
    return 0;
}

static int device_release(struct inode *inode, struct file *file) {
    printk(KERN_INFO "Device closed\n");
    return 0;
}
static long device_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    printk(KERN_INFO "IOCTL called\n");
    if (cmd == REGISTER_PROCESS) {
        struct monitor_request req;
        if (copy_from_user(&req, (void __user *)arg, sizeof(req))) {
            printk(KERN_INFO "copy failed\n");
            return -1;
        }

        printk(KERN_INFO "Registered PID %d | soft=%dMB | hard=%dMB\n",
               req.pid, req.soft_limit, req.hard_limit);
        if (pid_count < MAX_PROCESSES) {
            pid_list[pid_count++] = req.pid;
        } else {
            printk(KERN_INFO "PID list full\n");
        }
    }
    return 0;
}
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = device_open,
    .release = device_release,
    .unlocked_ioctl = device_ioctl,
};

static int __init monitor_init(void) {
    major = register_chrdev(0, DEVICE_NAME, &fops);

    if (major < 0) {
        printk(KERN_ALERT "Failed to register device\n");
        return major;
    }
    printk(KERN_INFO "Monitor loaded with major %d\n", major);
    return 0;
}
static void __exit monitor_exit(void) {
    printk(KERN_INFO "Cleaning up monitored processes\n");
    for (int i = 0; i < pid_count; i++) {
        printk(KERN_INFO "Removing PID %d from tracking\n", pid_list[i]);
    }

    pid_count = 0;  // reset tracking
    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "Monitor unloaded\n");
}

module_init(monitor_init);
module_exit(monitor_exit);
MODULE_LICENSE("GPL");
