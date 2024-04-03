#include <common_header.h>
#include "socket_m.h"

int main(void)
{
  pid_t pid;
  int fd[2];
  Socketpair(fd);
  ERROR_CHECK((pid = fork()), -1, "fork error");
  if (pid == 0)
  {
    close(fd[1]);
    int fd_to_recv = recv_fd(fd[0], write);
    if (fd_to_recv < 0)
    {
      printf("\nerror status is  = %d\n", fd_to_recv);
      return -1;
    }
    ERROR_CHECK(fd_to_recv, -1, "recv_fd error");
    char buf[] = "hello=world";
    ERROR_CHECK((write(fd_to_recv, buf, strlen(buf))), -1, "write error");
    close(fd_to_recv);
  }
  else
  {
    close(fd[0]);
    int fd_to_send = open("./file", O_RDWR);
    ERROR_CHECK((send_err(fd[1], -127, "这是一条错误消息")), -1, "send_fd error");
    // ERROR_CHECK((send_fd(fd[1], fd_to_send)), -1, "send_fd error");
    close(fd_to_send);
    waitpid(pid, NULL, 0);
  }
  return 0;
}