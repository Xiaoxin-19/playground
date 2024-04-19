#ifndef EPOOL_M_H
#define EPOOL_M_H
#include <sys/epoll.h>

int Epoll_create();
int Epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
int add_read_to_epoll(int epoll_fd, int fd, struct epoll_event *ev);
#endif