#include "socket_m.h"
#include "error.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/sendfile.h>

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
int send_fd_un(int fd, int fd_to_send)
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
int send_err_un(int fd, int status, const char *errmsg)
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

  if (send_fd_un(fd, status) < 0)
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
int recv_fd_un(int fd, ssize_t (*userfunc)(int, const void *, size_t))
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

#define MAX_LISTEN 10
/// @brief 初始化服务器
/// @param type 套接字类型
/// @param addr 地址 如果 addr 为 NULL 则绑定到所有地址
/// @param port 端口
int init_server(int type, const char *addr, int port)
{
  int fd;
  struct sockaddr_in server_addr;

  // 检查端口是否合法
  if (port <= 1024 || port >= 65535)
  {
    printf("port:[%d] error!\n", port);
    exit(1);
  }

  // 建立套接字
  ERROR_CHECK((fd = socket(AF_INET, type, 0)), -1, "socket error");

  // 设置socket属性
  bzero(&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  if (addr == NULL)
  {
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  }
  else
  {
    ERROR_CHECK(inet_aton(addr, &server_addr.sin_addr), 0, "inet_aton error");
  }
  server_addr.sin_port = htons(port);

  int reuse = 1;
  // 设置地址重用
  ERROR_CHECK(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)), -1, "setsockopt error");

  // 绑定地址
  ERROR_CHECK(bind(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)), -1, "bind error");

  // 如果是流式套接字或者顺序包套接字
  if (type == SOCK_STREAM || type == SOCK_SEQPACKET)
  {
    // 监听
    ERROR_CHECK(listen(fd, MAX_LISTEN), -1, "listen error");
  }
  printf("server: server start at [%s:%d]\n", addr, port);
  return fd;
}

int connect_retry(const char *addr_target, int port)
{
  int numsec, fd;
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  inet_aton(addr_target, &addr.sin_addr);
  addr.sin_port = htons(port);
  for (numsec = 1; numsec <= MAXSLEEP; numsec <<= 1)
  {
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
      return -1;
    }

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == 0)
    {
      printf("connect success\n");
      return fd;
    }

    close(fd);
    printf("connect_retry: waiting %d seconds for retry\n", numsec);
    if (numsec <= MAXSLEEP / 2)
    {
      sleep(numsec);
    }
  }
  return -1;
}

/// @brief 读取n个字节
/// @param fd 文件描述符
/// @param buf 缓冲区指针
/// @param len 读取长度
/// @return
int recv_n(int fd, void *buf, int len)
{
  int total = 0;
  int ret;
  while (total < len)
  {
    ret = recv(fd, (char *)buf + total, len - total, 0);
    if (ret == 0)
    {
      return total;
    }
    total += ret;
  }
  return total;
}

/// @brief sendfile的封装, 可以发送大于2G的文件，并判断客户端是否断开
/// @param out_fd 输出文件描述符
/// @param in_fd 输入文件描述符
/// @param offset 偏移量
/// @param count 发送字节数
/// @return 0 客户端断开连接 -1 发送失败 其他 发送字节数
int Sendfile(int out_fd, int in_fd, off_t *offset, size_t count)
{
  size_t sended = 0;
  off_t offset_tmp = *offset;
  // printf("offset: %ld\n", offset_tmp);
  while (sended < count)
  {
    // 发送大于2G的文件，循环多次发送
    ssize_t ret = sendfile(out_fd, in_fd, &offset_tmp, count - sended);
    printf("ret in send func: %ld\n", ret);
    if (ret == -1)
    {
      // 发送失败
      perror("sendfile error: cilent disconnect");
      return -1;
    }
    sended += ret;
  }
  return sended;
}