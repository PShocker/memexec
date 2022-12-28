#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>

#define SHM_PATH "/jit-cache"
#define EXEC_PATH "/data/local/tmp/NativeSurface"
extern char **environ;

struct image
{
    long size;
    void *data;
}image;

static char *args[] = {
    "/data/local/tmp/payload",
    NULL,
    NULL,
    NULL
};


void initImage()
{
    int fd, rc;
    void *p;
    struct stat st;
    rc = stat(EXEC_PATH, &st);
    if (rc == -1)
    {
        perror("stat");
        exit(1);
    }
    fd = open(EXEC_PATH, O_RDONLY, 0);
    if (fd == -1)
    {
        perror("openls");
        exit(1);
    }

    p = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE,
             fd, 0);

    image.size = st.st_size;
    image.data = p;
    close(fd);
}

void cleanImage()
{
    munmap(image.data, image.size);
}

int main(int argc, char *argv[])
{
    initImage();
    int fd = memfd_create(SHM_PATH, MFD_CLOEXEC);
    ftruncate(fd, (long)image.size); // 设置文件长度
    void *mem = mmap(NULL, image.size, PROT_WRITE, MAP_SHARED, fd, 0);
    memcpy(mem, image.data, image.size);
    munmap(mem, image.size);
    fexecve(fd, args, environ);
    cleanImage();
}