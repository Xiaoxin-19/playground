#ifndef SOCKET_M
#define SOCKET_M
#include <sys/types.h>

int Socketpair(int fd[2]);

int send_fd_un(int fd, int fd_to_send);

int send_err_un(int fd, int status, const char *errmsg);

int recv_fd_un(int fd, ssize_t (*userfunc)(int, const void *, size_t));

int init_server(int type, const char *addr, int port);

int recv_n(int fd, void *buf, int len);

#define MAXSLEEP 128
int connect_retry(const char *addr_target, int port);

int Sendfile(int out_fd, int in_fd, off_t *offset, size_t count);
#endif