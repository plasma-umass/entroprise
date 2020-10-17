#include <iostream>
#include <cstdlib>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "proc.hh"

extern char **environ;

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "usage <ld_preload> <exec>\n");
        return EXIT_FAILURE;
    }

    char c;
    void *ptr;
    int fd, num_allocs, max_num_live;
    double reuse;

    c = '\0';
    fd = open(FILE_NAME, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH); // Open file that will store data
    if (fd == -1) {
        fatal();
    }
    if (lseek(fd, 4096 - 1, SEEK_SET) == -1) {
        fatal();
    }
    if (write(fd, &c, 1) == -1)  { // lseek + write to increase the size of the file so that it can be used to sufficiently store all data
        fatal();
    }
    close(fd);

    create_proc(argv + 2, environ, argv[1]);
    ptr = get_proc_data();
    num_allocs = *((int *) ptr);
    max_num_live = *((int *) ptr + 1);
    reuse = (double) num_allocs / max_num_live;
    std::cout << "Number of Allocations: " << num_allocs << ", Maximum Number of Live Objects: " << max_num_live << ", Reuse: " << reuse << std::endl;
    return EXIT_SUCCESS;
}
