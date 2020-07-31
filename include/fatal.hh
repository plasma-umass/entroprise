#ifndef __FATAL_HH
#define __FATAL_HH

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <errno.h>

extern int errno;

void fatal(char *msg) {
    char *prefix = (char *) "ENTROPRISE ERROR: ";
    write(STDERR_FILENO, prefix, strlen(prefix));
    write(STDERR_FILENO, msg, strlen(msg));
    exit(EXIT_FAILURE);
}

void fatal() {
    perror("ENTROPRISE ERROR");
    // fprintf(stderr, "%d\n", errno);
    exit(EXIT_FAILURE);
}

#endif
