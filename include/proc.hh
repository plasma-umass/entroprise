#ifndef __MAP_HH
#define __MAP_HH

#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <dirent.h>
#include "tprintf.h"
#include "fatal.hh"
// #define FILE_NAME ".addrs.bin"
#define FILE_NAME "/nfs/cm/scratch1/emery/msteranka/entroprise-parsec-native/.entroprise-data.bin"
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
    * get_proc_data

    * Obtain the data stored by the forked process

    * Return Value: pointer to the data
*/

void **get_child_data() {
    void **data;
    DIR *dir = opendir("./");
    if (dir == nullptr) {
        return nullptr;
    }
    struct *dirent ent;
    while ((ent = readdir(dir)) != nullptr) {
        printf("%s\n", ent->d_name);
    }
    return nullptr;
}

void *extend_data(int fd, void *map, const int OLD_SIZE, const int NEW_SIZE) {
    void *ptr;
    char c = '\0', *err1 = (char *) "extend_data(): munmap failed\n", *err2 = (char *) "extend_data(): lseek failed\n", *err3 = (char *) "extend_data(): write failed\n", *err4 = (char *) "extend_data(): mmap failed\n", *err5 = (char *) "extend_data(): madvise failed\n";
    if (map != nullptr && OLD_SIZE != -1) {
        if (munmap(map, OLD_SIZE) == -1) {
            fatal(err1);
        }
    }
    if (lseek(fd, NEW_SIZE, SEEK_SET) == -1) {
        fatal(err2);
    }
    if (write(fd, &c, 1) == -1)  { // lseek + write to increase the size of the file so that it can be used to sufficiently store all data
        fatal(err3);
    }
    ptr = mmap(nullptr, NEW_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        fatal(err4);
    }
    if (madvise(ptr, NEW_SIZE, MADV_SEQUENTIAL) == -1) {
        fatal(err5);
    }
    return ptr;
}

void *extend_data(int fd, const int NEW_SIZE) {
    return extend_data(fd, nullptr, -1, NEW_SIZE);
}

void create_thread_data(int *fp, void **ptr, char *fname, const int INIT_SIZE) {
    char *err = (char *) "open failed\n";
    *fp = open(fname, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH); // Open file that will store data
    if (*fp == -1) {
        fatal(err);
    }
    *ptr = extend_data(*fp, INIT_SIZE);
}

#endif
