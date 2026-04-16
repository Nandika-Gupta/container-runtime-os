#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include "monitor_ioctl.h"

#define DEVICE_NAME "container_monitor"

static int major;

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
    printk(KERN_INFO "Monitor loaded with major %d\n", major);
    return 0;
}

static void __exit monitor_exit(void) {

    printk(KERN_INFO "Cleaning up monitored processes\n");  

    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "Monitor unloaded\n");
}

module_init(monitor_init);
module_exit(monitor_exit);

MODULE_LICENSE("GPL");
