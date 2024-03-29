#include <common_header.h>

#define MAXSLEEP 128
#define QLEN 10

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 256
#endif

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
  int clfd;
  FILE *fp;
  char buf[128];
  set_cloexec(sockfd);
  for (;;)
  {
    if ((clfd = accept(sockfd, NULL, NULL)) < 0)
    {
      syslog(LOG_ERR, "ruptimed: accept error: %s", strerror(errno));
      exit(1);
    }
    set_cloexec(clfd);
    if ((fp = popen("/usr/bin/uptime", "r")) == NULL)
    {
      sprintf(buf, "error: %s\n", strerror(errno));
      send(clfd, buf, strlen(buf), 0);
    }
    else
    {
      while (fgets(buf, 128, fp) != NULL)
      {
        send(clfd, buf, strlen(buf), 0);
      }
      printf("process return\n");
      pclose(fp);
    }
    close(clfd);
  }
}

int main(int argc, char *argv[])
{
  struct addrinfo hint;
  int sockfd, n;
  char *host;

  if (argc != 1)
  {
    perror("usage: ruptimed");
    return -1;
  }

  if ((n = sysconf(_SC_HOST_NAME_MAX)) < 0)
  {
    n = HOST_NAME_MAX;
  }

  if ((host = malloc(n)) == NULL)
  {
    perror("malloc error");
    return -1;
  }

  if (gethostname(host, n) < 0)
  {
    perror("gethostname error");
    return -1;
  }

  // daemonize("ruptimed");

  memset(&hint, 0, sizeof(hint));
  hint.ai_flags = AI_CANONNAME;
  hint.ai_socktype = SOCK_STREAM;
  hint.ai_canonname = NULL;
  hint.ai_addr = NULL;
  hint.ai_next = NULL;

  // if ((err = getaddrinfo(host, "ruptimed", &hint, &ailist)) != 0)
  // {
  //   syslog(LOG_ERR, "ruptimed: getaddrinfo error in serve: %s", gai_strerror(err));
  //   return -1;
  // }

  if ((sockfd = init_server(SOCK_STREAM, QLEN)) >= 0)
  {
    serve(sockfd);
    exit(0);
  }
  return 1;
}

void daemonize(const char *cmd)
{
  int i, fd0, fd1, fd2;
  pid_t pid;
  struct rlimit rl;
  struct sigaction sa;

  /*
   * Clear file creation mask.
   */
  umask(0);

  /*
   * Get maximum number of file descriptors.
   */
  if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
  {
    char buf[1024];
    sprintf(buf, "%s: can't get file limit", cmd);
    perror(buf);
    exit(1);
  }

  /*
   * Become a session leader to lose controlling TTY.
   */
  if ((pid = fork()) < 0)
  {
    char buf[1024];
    sprintf(buf, "%s: can't fork", cmd);
    perror(buf);
    exit(1);
  }

  else if (pid != 0) /* parent */
    exit(0);

  setsid();

  /*
   * Ensure future opens won't allocate controlling TTYs.
   */
  sa.sa_handler = SIG_IGN;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  if (sigaction(SIGHUP, &sa, NULL) < 0)
  {
    char buf[1024];
    sprintf(buf, "%s: can't ignore SIGHUP", cmd);
    perror(buf);
    exit(1);
  }
  if ((pid = fork()) < 0)
  {
    char buf[1024];
    sprintf(buf, "%s: can't fork", cmd);
    perror(buf);
    exit(1);
  }

  else if (pid != 0) /* parent */
    exit(0);

  /*
   * Change the current working directory to the root so
   * we won't prevent file systems from being unmounted.
   */
  if (chdir("/") < 0)
  {
    char buf[1024];
    sprintf(buf, "%s: can't change directory to /", cmd);
    perror(buf);
    exit(1);
  }

  /*
   * Close all open file descriptors.
   */
  if (rl.rlim_max == RLIM_INFINITY)
    rl.rlim_max = 1024;
  for (i = 0; i < rl.rlim_max; i++)
    close(i);

  /*
   * Attach file descriptors 0, 1, and 2 to /dev/null.
   */
  fd0 = open("/dev/null", O_RDWR);
  fd1 = dup(0);
  fd2 = dup(0);

  /*
   * Initialize the log file.
   */
  openlog(cmd, LOG_CONS, LOG_DAEMON);
  if (fd0 != 0 || fd1 != 1 || fd2 != 2)
  {
    syslog(LOG_ERR, "unexpected file descriptors %d %d %d",
           fd0, fd1, fd2);
    exit(1);
  }
}