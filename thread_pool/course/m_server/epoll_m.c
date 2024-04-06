#include "epoll_m.h"
#include "error.h"
#include <errno.h>
#include <sys/epoll.h>
#include <stdlib.h>

/// @brief epoll_create 包装函数
/// @return int epoll_fd
int Epoll_create()
{
  int ret = epoll_create(1);
  ERROR_CHECK(ret, -1, "epoll_create error");
  return ret;
}

/// @brief epoll_wait 包装函数
/// @param epoll_fd
/// @param evlist
/// @param maxevents
/// @param timeout
/// @return
int Epoll_wait(int epoll_fd, struct epoll_event *evlist, int maxevents, int timeout)
{
  int ret = epoll_wait(epoll_fd, evlist, maxevents, timeout);
  if (ret == -1 && errno == EINTR)
  {
    return 0;
  }
  ERROR_CHECK(ret, -1, "epoll_wait error");
  return ret;
}

int add_read_to_epoll(int epoll_fd, int fd, struct epoll_event *ev)
{
  if (ev == NULL)
  {
    ev = (struct epoll_event *)calloc(1, sizeof(struct epoll_event));
  }
  ev->events = EPOLLIN;
  ev->data.fd = fd;
  int ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, ev);
  if (ret == -1)
  {
    perror("epoll_ctl add failed");
    exit(1);
  }
  return ret;
}