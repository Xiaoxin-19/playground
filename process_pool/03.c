#include <common_header.h>

int main(void)
{
  pid_t pid;
  int socket_fd[2];

  socketpair(AF_UNIX, SOCK_STREAM, 0, socket_fd);
  ERROR_CHECK((pid = fork()), -1, "fork error");
  if (pid == 0)
  {
    close(socket_fd[1]);

    
    struct msghdr msg;
    bzero(&msg, sizeof(msg));

    // 设置数据信息
    struct iovec iov[1];
    char *str = "hello world!";
    iov[0].iov_base = str;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    // 设置控制信息
    struct cmsghdr *cms = (struct cmsghdr *)malloc(CMSG_LEN(sizeof(int)));
    cms->cmsg_len = CMSG_LEN(sizeof(int));
    cms->cmsg_level = SOL_SOCKET;
    cms->cmsg_type = SCM_RIGHTS;
    void *p = CMSG_DATA(cms);
    int *pFd = (int *)p;
    *pFd = file_fd;
    msg.msg_control = &cms;

    printf("child process\n");
  }
  else
  {
    close(socket_fd[0]);

    int file_fd = open("file.txt", O_RDWR);

    struct msghdr msg;
    bzero(&msg, sizeof(msg));

    // 设置数据信息
    struct iovec iov[1];
    char *str = "hello world!";
    iov[0].iov_base = str;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    // 设置控制信息
    struct cmsghdr *cms = (struct cmsghdr *)malloc(CMSG_LEN(sizeof(int)));
    cms->cmsg_len = CMSG_LEN(sizeof(int));
    cms->cmsg_level = SOL_SOCKET;
    cms->cmsg_type = SCM_RIGHTS;
    void *p = CMSG_DATA(cms);
    int *pFd = (int *)p;
    *pFd = file_fd;
    msg.msg_control = &cms;

    sendmsg(socket_fd[1], &msg, 0);
    printf("parent process\n");
  }
  return 0;
}
