#ifndef __FATAL_HH
#define __FATAL_HH

#include <stdio.h>
#include <stdlib.h>

void fatal() {
    perror("ERROR");
    exit(EXIT_FAILURE);
}

#endif
