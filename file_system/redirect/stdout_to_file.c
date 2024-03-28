#include <common_header.h>

int main(void)
{
    close(STDOUT_FILENO);
    int fd = open("file_out", O_RDWR | O_CREAT | O_TRUNC, 0777);
    printf("fd: %d", fd);

    return 0;
}
