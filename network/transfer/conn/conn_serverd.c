#include <common_header.h>

int set_cloexec(int fd)
{
  int val = fcntl(fd, F_GETFD, 0);
  val |= FD_CLOEXEC;
  return fcntl(fd, F_SETFD, val);
}

int init_server(int type, socklen_t alen)
{
  int fd;
  int err = 0;
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  inet_aton("0.0.0.0", &addr.sin_addr);
  addr.sin_port = htons(6666);

  if ((fd = socket(addr.sin_family, type, 0)) < 0)
  {
    return -1;
  }

  if ((bind(fd, (struct sockaddr *)&addr, sizeof(addr))) < 0)
  {
    perror("error:");
    goto errout;
  }

  if (type == SOCK_STREAM || type == SOCK_SEQPACKET)
  {
    if (listen(fd, 10) < 0)
    {
      goto errout;
    }
  }
  return fd;
errout:
  err = errno;
  close(fd);
  errno = err;
  return -1;
}

void serve(int sockfd)
{
  int clfd, status;
  pid_t pid;

  set_cloexec(sockfd);
  while (true)
  {
    if ((clfd = accept(sockfd, NULL, NULL)) < 0)
    {
      perror("accept error");
      exit(1);
    }
    printf("connection from client\n");

    ERROR_CHECK((pid = fork()), -1, "fork error");
    if (pid == 0)
    {
      if (dup2(clfd, STDOUT_FILENO) != STDOUT_FILENO || dup2(clfd, STDERR_FILENO) != STDERR_FILENO)
      {
        perror("dup2 error");
        exit(1);
      }
      close(clfd);
      execl("/usr/bin/uptime", "uptime", (char *)0);
      perror("unexpected return from exec:");
      exit(1);
    }
    else
    {
      close(clfd);
      waitpid(pid, &status, 0);
    }
  }
}
int main(int argc, char *argv[])
{
  if (argc != 1)
  {
    perror("argc error");
    exit(1);
  }

  int sockfd = init_server(SOCK_STREAM, sizeof(struct sockaddr_in));
  if (sockfd < 0)
  {
    perror("init_server error");
    exit(1);
  }
  printf("server start\n");
  serve(sockfd);
  return 0;
}