#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char **environ;

void fatal() {
    perror("ERROR");
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "usage <exec>\n");
        return EXIT_FAILURE;
    }
    char **new_environ, path[4096];
    void **map;
    struct stat stat_buf;
    int fd, i, naddrs;
    if (fork() == 0) {
        i = 0;
        for (i = 0; environ[i] != nullptr; i++);
        new_environ = (char **) malloc((i + 2) * sizeof(char *));
        strcpy(path, "LD_PRELOAD=");
        getcwd(path + strlen(path), 4096);
        strcat(path, "/librandomness.so");
        new_environ[0] = path;
        for (i = 0; environ[i] != nullptr; i++) {
            new_environ[i + 1] = environ[i];
        }
        new_environ[i + 1] = nullptr;
        for (i = 0; new_environ[i] != nullptr; i++) {
            printf("%s\n", new_environ[i]);
        }
        execve(argv[1], argv + 1, new_environ);
        fatal();
    }
    wait(nullptr);
    fd = open("addrs.bin", O_RDONLY);
    if (fd == -1) {
        fatal();
    }
    if (fstat(fd, &stat_buf) == -1) {
        fatal();
    }
    map = (void **) mmap(nullptr, stat_buf.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        fatal();
    }
    if (madvise(map, stat_buf.st_size, MADV_SEQUENTIAL) == -1) {
        fatal();
    }
    // naddrs = *((int *) map);
    munmap(map, stat_buf.st_size);
    close(fd);
    return EXIT_SUCCESS;
}
