#ifndef __FATAL_HH
#define __FATAL_HH

#include <cstdlib>
#include <cstring>
#include <unistd.h>

void fatal(char *msg) {
    write(STDERR_FILENO, msg, strlen(msg));
    exit(EXIT_FAILURE);
}

#endif
