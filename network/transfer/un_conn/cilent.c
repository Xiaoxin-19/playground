#include <common_header.h>

#define BUFLEN 128
#define TIMEOUT 20

void sig_alarm(int signo)
{
  return;
}

void print_uptime(int sockfd, struct sockaddr_in *addr, socklen_t len)
{
  int n;
  char buf[BUFLEN];

  if (sendto(sockfd, buf, 1, 0, (struct sockaddr *)addr, sizeof(*addr)) < 0)
  {
    perror("sendto error");
    exit(1);
  }
  alarm(TIMEOUT);
  if ((n = recvfrom(sockfd, buf, BUFLEN, 0, (struct sockaddr *)addr, &len)) < 0)
  {
    if (errno != EINTR)
    {
      alarm(0);
    }
    perror("recvfrom error");
  }
  alarm(0);
  write(STDOUT_FILENO, buf, n);
}

int main(int argc, char *argv[])
{
  ARGS_CHECK(argc, 2);
  struct sigaction sa;

  sa.sa_handler = sig_alarm;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);
  ERROR_CHECK(sigaction(SIGALRM, &sa, NULL), -1, "sigaction error");

  int sockfd;
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    perror("socket error");
    exit(1);
  }
  printf("socket created\n");

  struct sockaddr_in addr;
  inet_aton(argv[1], &addr.sin_addr);
  addr.sin_family = AF_INET;
  addr.sin_port = htons(6666);

  print_uptime(sockfd, &addr, sizeof(addr));
  return 0;
}