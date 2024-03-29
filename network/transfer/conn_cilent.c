#include <common_header.h>

#define MAXSLEEP 128
extern int connect_retry(int, int, int);

void print_uptime(int sockfd)
{
  int n;
  char buf[128];

  while ((n = recv(sockfd, buf, 128, 0)) > 0)
  {
    write(STDOUT_FILENO, buf, n);
  }
  if (n < 0)
  {
    perror("recv error");
  }
}

int main(int argc, char *argv[])
{
  ARGS_CHECK(argc, 2);
  struct addrinfo *ailist, *aip;
  struct addrinfo hint;
  int sockfd, err;

  memset(&hint, 0, sizeof(hint));
  hint.ai_socktype = SOCK_STREAM;
  hint.ai_canonname = NULL;
  hint.ai_addr = NULL;
  hint.ai_next = NULL;

  if ((sockfd = connect_retry(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    err = errno;
  }
  else
  {
    print_uptime(sockfd);
    return 0;
  }

  return 0;
}

int connect_retry(int domain, int type, int protocol)
{
  int numsec, fd;
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  inet_aton("0.0.0.0", &addr.sin_addr);
  addr.sin_port = 6666;
  for (numsec = 1; numsec <= MAXSLEEP; numsec <<= 1)
  {
    if ((fd = socket(domain, type, protocol)) < 0)
    {
      return -1;
    }

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == 0)
    {
      return fd;
    }

    close(fd);

    if (numsec <= MAXSLEEP / 2)
    {
      sleep(numsec);
    }
  }
  return -1;
}