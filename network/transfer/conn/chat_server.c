#include <common_header.h>

void set_cloexec(int fd)
{
  int val = fcntl(fd, F_GETFD, 0);
  val |= FD_CLOEXEC;
  fcntl(fd, F_SETFD, val);
}

int init_serve(const int type, const int port, const char *listent_addr)
{
  int fd;
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  inet_aton(listent_addr, &addr.sin_addr);
  addr.sin_port = htons(port);

  if ((fd = socket(addr.sin_family, type, 0)) < 0)
  {
    perror("socket error");
    return -1;
  }
  int reuse = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
  set_cloexec(fd);
  if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    perror("bind error");
    goto error_out;
  }

  if (type == SOCK_STREAM || type == SOCK_SEQPACKET)
  {
    if (listen(fd, 10) < 0)
    {
      perror("listen error");
      goto error_out;
    }
  }

  return fd;

error_out:
  close(fd);
  return -1;
}

int main(int argc, char *argv[])
{
  int fd;
  ERROR_CHECK((fd = init_serve(SOCK_STREAM, 6666, "0.0.0.0")), -1, "init_server error");
  printf("server start\n");

  fd_set rset, backup_set;
  FD_ZERO(&backup_set);
  FD_SET(fd, &backup_set);
  FD_SET(STDIN_FILENO, &backup_set);
  int new_fd = 5;
  while (true)
  {
    memcpy(&rset, &backup_set, sizeof(fd_set));
    int ret = select(new_fd + 1, &rset, NULL, NULL, NULL);
    if (ret < 0)
    {
      if (errno == EINTR)
      {
        continue;
      }
      perror("select error");
      break;
    }

    if (FD_ISSET(fd, &rset))
    {

      if ((new_fd = accept(fd, NULL, NULL)) < 0)
      {
        perror("accept error");
        break;
      }
      printf("cilent connect\n");
      set_cloexec(new_fd);
      FD_ZERO(&backup_set);
      FD_SET(new_fd, &backup_set);
      FD_SET(STDIN_FILENO, &backup_set);
      FD_SET(fd, &backup_set);
    }

    if (FD_ISSET(new_fd, &rset))
    {
      char buf[128];
      int n;
      if ((n = recv(new_fd, buf, 128, 0)) > 0)
      {
        write(STDOUT_FILENO, "client: ", 9);
        write(STDOUT_FILENO, buf, n);
      }
      if (n == 0)
      {
        printf("client close\n");
        FD_CLR(new_fd, &backup_set);
        FD_CLR(STDIN_FILENO, &backup_set);
      }
      if (n < 0)
      {
        perror("recv error");
      }
    }

    if (FD_ISSET(STDIN_FILENO, &rset))
    {
      char buf[128];
      int n;
      if ((n = read(STDIN_FILENO, buf, 128)) > 0)
      {
        if (send(new_fd, buf, n, 0) < 0)
        {
          perror("send error");
          break;
        }
      }
      if (n < 0)
      {
        perror("read error");
      }
    }
  }
  return 0;
}