#include <common_header.h>

int main(void)
{
    fd_set rfds;
    fd_set wfds;

    int fd1, fd2;
    ERROR_CHECK((fd1 = open("1.pipe", O_RDONLY)), -1, "1 open failed");
    ERROR_CHECK((fd2 = open("2.pipe", O_WRONLY)), -1, "2 open failed");
    int max_fd = fd1 > fd2 ? fd1 : fd2;

    char buf[128] = {0};
    while (true)
    {

        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        FD_SET(STDIN_FILENO, &rfds);
        FD_SET(fd1, &rfds);
        struct timeval tv;
        tv.tv_sec = 10;
        tv.tv_usec = 0;
        int ret = select(max_fd + 1, &rfds, NULL, NULL, &tv);
        ERROR_CHECK(ret, -1, "select error");
        if (ret == 0)
        {
            printf("10 sec A disconnect\n");
            break;
        }
        if (FD_ISSET(fd1, &rfds))
        {
            bzero(buf, sizeof(buf));
            if (read(fd1, buf, sizeof(buf)) == 0)
            {
                printf("对方断开连接]\n");
                break;
            }
            printf("UserB: %s\n", buf);
            tv.tv_sec = 10;
            tv.tv_usec = 0;
        }
        if (FD_ISSET(STDIN_FILENO, &rfds))
        {
            bzero(buf, sizeof(buf));
            read(STDIN_FILENO, buf, sizeof(buf));
            write(fd2, buf, sizeof(buf));
        }
    }
    close(fd1);
    close(fd2);
    return 0;
}