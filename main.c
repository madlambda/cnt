#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sched.h>
#include <errno.h>
#include <string.h>

#define STACK_SIZE 1024

/* 
 * Moves the rootfs of the calling process to `putold` and makes
 * `newroot` the new root filesystem of process. 
 * It does so by first moving:
 * 
 *        MS_REC (since Linux 2.4.11)
              Used in conjunction with MS_BIND to create a recursive bind mount, and in conjunction
              with  the propagation type flags to recursively change the propagation type of all of
              the mounts in a subtree.  See below for further details.
 * */
int pivot_root(char *newroot, char *putold) {
    /* bind mount newroot into itself. 
     * Remember that we are in a new mount namespace, then this bind mount
     * makes the / mount of the container.
     */
    if (mount(newroot, newroot, "bind", MS_BIND|MS_REC, "") < 0) {
        fprintf(stderr, "error mount: %s\n", strerror(errno));
        return 1;
    }

    /* `putold` is where pivot_root is gonna put our parent rootfs */
    if (mkdir(putold, 0755) < 0) {
        fprintf(stderr, "error mkdir: %s\n", strerror(errno));
        /* ignore  */
    }

    printf("set up pivot root\n");
    return syscall(SYS_pivot_root, newroot, putold);
}

int child_exec(void *args) {
    char **cmd = (char **) args;

    if(pivot_root("./rootfs", "./rootfs/.old") < 0) {
        fprintf(stderr, "pivot_root: %s\n", strerror(errno));
        return 1;
    }

    if (mount("tmpfs", "/dev", "tmpfs", MS_NOSUID|MS_STRICTATIME, NULL) < 0) {
        fprintf(stderr, "mount tmpfs: %s\n", strerror(errno));
        return 1;
    }

    if (mount("proc", "/proc", "proc", 0, NULL) < 0) {
        fprintf(stderr, "mount proc: %s\n", strerror(errno));
        return 1;
    }

    chdir("/");

    /* at this point, we still have access to the parent rootfs.
     * Unmount it here if not used anymore.
     */
    if (umount2("./.old", MNT_DETACH) < 0) {
        fprintf(stderr, "umount2 ./.old: %s\n", strerror(errno));
        return 1;
    }

    execve(cmd[0], cmd, NULL);
    return 0;
}

int main(int argc, char **argv) {
    char stack[STACK_SIZE];
    char **args = &argv[1];
    pid_t pid;
    int clone_flags = SIGCHLD | CLONE_NEWNS | CLONE_NEWPID | CLONE_NEWUSER;

    pid = clone(child_exec, stack, clone_flags, args);
    if (pid < 0) {
        fprintf(stderr, "clone failed: %s\n", strerror(errno));
        return 1;
    }

    waitpid(pid, NULL, 0);
    return 0;
}