#include <common_header.h>

#define MAXSLEEP 128

int connect_retry(int sockfd, const struct sockaddr *addr, socklen_t alen)
{
  int numsec;
  for (numsec = 1; numsec <= MAXSLEEP; numsec <<= 1)
  {
    if (connect(sockfd, addr, alen) == 0)
    {
      return 0;
    }

    if (numsec <= MAXSLEEP / 2)
    {
      sleep(numsec);
    }
  }
  return -1;
}

int connect_retry_2(int domain, int type, int protocol, const struct sockaddr *addr, socklen_t alen)
{
  int numsec, fd;
  for (numsec = 1; numsec <= MAXSLEEP; numsec <<= 1)
  {
    if ((fd = socket(domain, type, protocol)) < 0)
    {
      return -1;
    }

    if (connect(fd, addr, alen) == 0)
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

int init_server(int type, const struct sockaddr *addr, socklen_t alen)
{
  int fd;
  int err = 0;
  if ((fd = socket(addr->sa_family, type, 0)) < 0)
  {
    return -1;
  }

  if ((bind(fd, addr, alen)) < 0)
  {
    goto errout;
  }

  if (type == SOCK_STREAM || type == SOCK_SEQPACKET)
  {
    if (listen(fd, 10) < 0)
    {
      goto errout;
    }
  }

errout:
  err = errno;
  close(fd);
  errno = err;
  return -1;
}
int main(void)
{

  return 0;
}