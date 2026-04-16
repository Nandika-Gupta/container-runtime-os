#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sched.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "monitor_ioctl.h"

#define MAX 10

typedef struct {
    char name[50];
    pid_t pid;
    int stop_requested;
} container;

container c[MAX];
int count = 0;

void sigchld_handler(int sig) {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < count; i++) {
            if (c[i].pid == pid) {

                if (c[i].stop_requested)
                    printf("[Stopped] %s (PID %d)\n", c[i].name, pid);
                else
                    printf("[Exited/Killed] %s (PID %d)\n", c[i].name, pid);

                for (int j = i; j < count - 1; j++)
                    c[j] = c[j + 1];

                count--;
                break;
            }
        }
    }
}

void start_container(char *name) {

    for (int i = 0; i < count; i++) {
        if (strcmp(c[i].name, name) == 0) {
            printf("Container already exists\n");
            return;
        }
    }

    int fd[2];
    pipe(fd);

    pid_t pid = fork();

    if (pid == 0) {
        // CHILD

        close(fd[0]);

        dup2(fd[1], STDOUT_FILENO);
        dup2(fd[1], STDERR_FILENO);
        close(fd[1]);

        unshare(CLONE_NEWUTS | CLONE_NEWNS);
        sethostname(name, strlen(name));

        char rootfs[100];
        sprintf(rootfs, "./rootfs-%s", name);

        if (chroot(rootfs) != 0) {
            perror("chroot failed");
            exit(1);
        }

        chdir("/");

        mkdir("/proc", 0555);
        if (mount("proc", "/proc", "proc", 0, NULL) != 0) {
            perror("mount failed");
        }

        setbuf(stdout, NULL);

        while (1) {
            write(1, "container_running\n", 18);
            sleep(2);
        }
    }

    close(fd[1]);

    // LOGGER PROCESS
    pid_t logger_pid = fork();
    if (logger_pid == 0) {

        signal(SIGTERM, exit);   //  important fix

        char logfile[100];
        sprintf(logfile, "%s.log", name);
        FILE *log = fopen(logfile, "a");

        char buffer[256];
        int n;

        while ((n = read(fd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[n] = '\0';
            fprintf(log, "%s", buffer);
            fflush(log);
        }

        fclose(log);
        close(fd[0]);
        exit(0);
    }

    close(fd[0]);

    // KERNEL REGISTRATION
    int fd_dev = open("/dev/container_monitor", O_RDWR);
    if (fd_dev >= 0) {
        struct monitor_request req;
        req.pid = pid;
        req.soft_limit = 50;
        req.hard_limit = 80;

        ioctl(fd_dev, REGISTER_PROCESS, &req);
        close(fd_dev);
    } else {
        perror("open device failed");
    }

    strcpy(c[count].name, name);
    c[count].pid = pid;
    c[count].stop_requested = 0;
    count++;

    printf("Started %s (PID %d)\n", name, pid);
}

void stop_container(char *name) {
    for (int i = 0; i < count; i++) {
        if (strcmp(c[i].name, name) == 0) {

            c[i].stop_requested = 1;

            kill(c[i].pid, SIGTERM);
            waitpid(c[i].pid, NULL, 0);   

            printf("Stopped %s\n", name);
            return;
        }
    }
}

void list_containers() {
    printf("Running containers:\n");
    for (int i = 0; i < count; i++) {
        printf("%s | PID %d\n", c[i].name, c[i].pid);
    }
}

int main() {
    signal(SIGCHLD, sigchld_handler);

    char cmd[100];

    printf("Supervisor started (waiting on FIFO)...\n");

    while (1) {
        FILE *fp = fopen("cmd_pipe", "r");
        if (!fp) continue;

        if (fgets(cmd, sizeof(cmd), fp) == NULL) {
            fclose(fp);
            continue;
        }
        fclose(fp);

        char *token = strtok(cmd, " \n");
        if (!token) continue;

        if (strcmp(token, "start") == 0) {
            char *name = strtok(NULL, " \n");
            if (name) start_container(name);
        }
        else if (strcmp(token, "stop") == 0) {
            char *name = strtok(NULL, " \n");
            if (name) stop_container(name);
        }
        else if (strcmp(token, "ps") == 0) {
            list_containers();
        }
        else if (strcmp(token, "exit") == 0) {
        
            for (int i = 0; i < count; i++) {
                kill(c[i].pid, SIGTERM);
                waitpid(c[i].pid, NULL, 0);
            }

            printf("Shutting down supervisor...\n");
            break;
        }
    }

    return 0;
}
