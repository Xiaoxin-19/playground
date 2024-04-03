#ifndef SOCKET_M
#define SOCKET_M


int Socketpair(int fd[2]);

int send_fd(int fd, int fd_to_send);

int send_err(int fd, int status, const char *errmsg);

int recv_fd(int fd, ssize_t (*userfunc)(int, const void *, size_t));
#endif