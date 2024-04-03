#include <common_header.h>
#include "socket_m.h"

/// @brief UNIX域套接字创建
/// @param fd 两个文件描述符存储在fd[0]、fd[1]
/// @return -1 失败 0 成功
int Socketpair(int fd[2])
{
  int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, fd);
  ERROR_CHECK(ret, -1, "socketpair error");
  return ret;
}

#define CONTROLLEN CMSG_LEN(sizeof(int))
static struct cmsghdr *cmptr = NULL;

/// @brief 发送文件描述符,如果fd_to_send<0,表示发生错误，发送fd_to_send绝对值表示错误状态status, 使用前两个字节表示控制码0X00表示正常，0X01表示错误
/// @param fd UNIX域套接字
/// @param fd_to_send  需要发送的文件描述符
/// @return 0 成功 -1 失败
int send_fd(int fd, int fd_to_send)
{
  struct iovec iov[1];
  struct msghdr msg;
  char buf[2];

  iov[0].iov_base = buf;
  iov[0].iov_len = 2;
  msg.msg_iov = iov;
  msg.msg_iovlen = 1;
  msg.msg_name = NULL;
  msg.msg_namelen = 0;

  if (fd_to_send < 0)
  {
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    buf[1] = -fd_to_send;
    if (buf[1] == 0)
      buf[1] = 1;
  }
  else
  {
    if (cmptr == NULL && (cmptr = malloc(CONTROLLEN)) == NULL)
    {
      return -1;
    }

    cmptr->cmsg_level = SOL_SOCKET;
    cmptr->cmsg_type = SCM_RIGHTS;
    cmptr->cmsg_len = CONTROLLEN;
    msg.msg_control = cmptr;
    msg.msg_controllen = CONTROLLEN;
    *(int *)CMSG_DATA(cmptr) = fd_to_send;
    buf[1] = 0;
  }

  buf[0] = 0;

  if (sendmsg(fd, &msg, 0) != 2)
  {
    return -1;
  }
  return 0;
}

/// @brief 发送错误信息
/// @param fd UNIX域套接字
/// @param status 状态码
/// @param errmsg 错误信息
/// @return 0 成功 -1 失败
int send_err(int fd, int status, const char *errmsg)
{
  int n;
  if ((n = strlen(errmsg)) > 0)
  {
    if (n != write(fd, errmsg, n))
    {
      return -1;
    }
  }

  if (status >= 0)
  {
    status = -1;
  }

  if (send_fd(fd, status) < 0)
  {
    return -1;
  }

  return 0;
}

#define MAXLINE 1024
/// @brief 接收文件描述符, 当发生错误时，使用usrfunc函数将错误信息写入标准错误
/// @param fd UNIX域套接字
/// @param userfunc 用户自定义函数
/// @return 成功返回文件描述符，失败返回-1
int recv_fd(int fd, ssize_t (*userfunc)(int, const void *, size_t))
{
  int newfd, nr, status;
  char *ptr;
  char buf[MAXLINE];
  struct iovec iov[1];
  struct msghdr msg;

  status = -1;
  while (true)
  {
    bzero(&msg, sizeof(struct msghdr));
    iov[0].iov_base = buf;
    iov[0].iov_len = MAXLINE;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    if (cmptr == NULL && (cmptr = malloc(CONTROLLEN)) == NULL)
    {
      return -1;
    }

    msg.msg_control = cmptr;
    msg.msg_controllen = CONTROLLEN;

    if ((nr = recvmsg(fd, &msg, 0)) < 0)
    {
      return -1;
    }
    else if (nr == 0)
    {
      perror("connection closed by server");
      return -1;
    }

    for (ptr = buf; ptr < &buf[nr];)
    {
      if (*ptr++ == 0)
      {
        if (ptr != &buf[nr - 1])
        {
          perror("message format error");
          return -1;
        }

        status = *ptr & 0xFF;
        if (status == 0)
        {
          if (msg.msg_controllen < CONTROLLEN)
          {
            perror("status = 0 but no fd");
            return -1;
          }
          newfd = *(int *)CMSG_DATA(cmptr);
        }
        else
        {
          newfd = -status;
        }
        nr -= 2;
      }
    }

    if (nr > 0 && (*userfunc)(STDERR_FILENO, buf, nr) != nr)
    {
      return -1;
    }

    if (status >= 0)
    {
      return newfd;
    }
  }
}