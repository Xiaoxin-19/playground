#include <common_header.h>

#define BUFLEN 1024
#define MAXADDRLEN 256

void set_cloexce(int fd)
{
  int val = fcntl(fd, F_GETFD, 0);
  val |= FD_CLOEXEC;
  fcntl(fd, F_SETFD, val);
}

int init_serve(int type)
{
  int fd;
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  inet_aton("0.0.0.0", &addr.sin_addr);
  addr.sin_port = htons(6666);

  if ((fd = socket(addr.sin_family, type, 0)) < 0)
  {
    perror("socket error");
    return -1;
  }

  int resuse = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &resuse, sizeof(resuse)) < 0)
  {
    perror("setsockopt error");
    goto error;
  }

  if ((bind(fd, (struct sockaddr *)&addr, sizeof(addr))) < 0)
  {
    perror("bind error");
    goto error;
  }

  if (type == SOCK_STREAM || type == SOCK_SEQPACKET)
  {
    if (listen(fd, 10) < 0)
    {
      perror("listen error");
      goto error;
    }
  }

  return fd;
error:
  close(fd);
  return -1;
}

void serve(int sockfd)
{
  FILE *fp;
  char abuf[MAXADDRLEN];
  socklen_t alen;
  struct sockaddr *addr = (struct sockaddr *)abuf;
  set_cloexce(sockfd);

  while (true)
  {
    int n;
    char buf[1024];
    // 接收upd发来的消息并获取客户端地址
    if ((n = recvfrom(sockfd, buf, BUFLEN, 0, addr, &alen)) < 0)
    {
      perror("recvfrom error");
      exit(1);
    }

    // 执行程序
    char client_addr[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &((struct sockaddr_in *)addr)->sin_addr, client_addr, INET_ADDRSTRLEN) == NULL)
    {
      perror("inet_ntop error");
    }
    printf("server from host: %s\n", client_addr);

    if ((fp = popen("/usr/bin/uptime", "r")) == NULL)
    {
      sprintf(buf, "error: %s\n", strerror(errno));
      sendto(sockfd, buf, strlen(buf), 0, addr, alen);
    }
    else
    {
      if (fgets(buf, BUFLEN, fp) != NULL)
      {
        sendto(sockfd, buf, strlen(buf), 0, addr, alen);
      }
      pclose(fp);
    }
  }
}
int main(int argc, char *argv[])
{
  int sockfd;
  if (argc != 1)
  {
    perror("usage: uptimed");
    exit(1);
  }

  if ((sockfd = init_serve(SOCK_DGRAM)) < 0)
  {
    perror("init_serve error");
    exit(1);
  }

  // 服务
  printf("server started\n");
  serve(sockfd);

  return 0;
}