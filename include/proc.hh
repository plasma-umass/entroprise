#ifndef __MAP_HH
#define __MAP_HH

#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include "fatal.hh"
#define FILE_NAME ".addrs.bin"
#include <cstdio>

/*
    * create_process

    * Creates a new process with the given arguments, environment variables, and allocator

    * argv: arguments for new process, where the first argument is the name of the process and the last argument is nullptr
    * env: environment variables for new process, where the last environment variable is nullptr
    * alloc: the allocator to run the process with in the format "LD_PRELOAD=/path/to/allocator.so"
*/

void create_process(char **argv, char** env, char *alloc) {
    char **new_env;
    int i;

    if (fork() == 0) {
        i = 0;
        for (i = 0; env[i] != nullptr; i++); // Find number of environment variables so we know how much to malloc
        new_env = (char **) malloc((i + 2) * sizeof(char *)); // +1 for LD_PRELOAD and +1 for nullptr
        new_env[0] = alloc; // Copy LD_PRELOAD value to environment variables of new process
        for (i = 0; environ[i] != nullptr; i++) {
            new_env[i + 1] = env[i];
        }
        new_env[i + 1] = nullptr;
        execve(argv[0], argv, new_env);
        fatal(); // execve should never reach this point if it succeeded
    }
    wait(nullptr);
}

/*
    * get_data

    * Obtain the data stored by the forked process

    * Return Value: pointer to the data
*/

void *get_proc_data() {
    void *ptr;
    int fd;
    struct stat sbuf;
    char *err1 = (char *) "open failed\n", *err2 = (char *) "fstat failed\n", *err3 = (char *) "mmap failed\n", *err4 = (char *) "madvise failed\n";

    fd = open(FILE_NAME, O_RDWR); // Open the same file that librandomness.so saved addresses to
    if (fd == -1) {
        fatal(err1);
    }
    if (fstat(fd, &sbuf) == -1) {
        fatal(err2);
    }
    ptr = mmap(nullptr, sbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        fatal(err3);
    }
    if (madvise(ptr, sbuf.st_size, MADV_SEQUENTIAL) == -1) {
        fatal(err4);
    }
    return ptr;
}

#endif
