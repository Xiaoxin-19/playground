#include <common_header.h>
#define MAXSLEEP 128
int connect_retry(const char *addr_target, int port);
int main(int argc, char *argv[])
{
  ARGS_CHECK(argc, 3);
  int sockfd;
  char buf[128];
  if ((sockfd = connect_retry(argv[1], atoi(argv[2]))) < 0)
  {
    perror("connect_retry error");
    exit(1);
  }

  fd_set rset, backup_set;
  FD_ZERO(&backup_set);
  FD_SET(sockfd, &backup_set);
  FD_SET(STDIN_FILENO, &backup_set);
  while (true)
  {
    memcpy(&rset, &backup_set, sizeof(fd_set));
    int ret = select(sockfd + 1, &rset, NULL, NULL, NULL);
    if (ret < 0)
    {
      if (errno == EINTR)
      {
        continue;
      }
      perror("select error");
      break;
    }

    if (FD_ISSET(sockfd, &rset))
    {
      int n;

      if ((n = recv(sockfd, buf, 128, 0)) > 0)
      {
        write(STDOUT_FILENO, "serve: ", 8);
        write(STDOUT_FILENO, buf, n);
      }
      if (n == 0)
      {
        printf("server close\n");
        break;
      }
      if (n < 0)
      {
        perror("recv error");
      }
    }

    if (FD_ISSET(STDIN_FILENO, &rset))
    {
      int n;
      if ((n = read(STDIN_FILENO, buf, 128)) > 0)
      {
        if ((n = send(sockfd, buf, n, 0)) < 0)
        {
          perror("send error");
        }
      }
      if (n < 0)
      {
        perror("read error");
      }
    }
  }
  close(sockfd);
  return 0;
}

int connect_retry(const char *addr_target, int port)
{
  int numsec, fd;
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  inet_aton(addr_target, &addr.sin_addr);
  addr.sin_port = htons(port);
  for (numsec = 1; numsec <= MAXSLEEP; numsec <<= 1)
  {
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
      return -1;
    }

    if (connect(fd, (SA *)&addr, sizeof(addr)) == 0)
    {
      printf("connect success\n");
      return fd;
    }

    close(fd);
    printf("connect_retry: waiting %d seconds for retry\n", numsec);
    if (numsec <= MAXSLEEP / 2)
    {
      sleep(numsec);
    }
  }
  return -1;
}