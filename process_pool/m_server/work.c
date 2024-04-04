#include "wrok.h"
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include "socket_m.h"
#include "error.h"
#include <stdio.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <netinet/in.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal_m.h>
#include <signal.h>

void get_file_list(char *buf)
{
  char p[] = "hello.txt\n /home/ubuntu/etc/assp.mkv\n";
  strncpy(buf, p, strlen(p));
}

/// @brief 工作
/// @return
void work(int main_fd)
{
  // 忽略SIGPIPE信号
  signal_m(SIGPIPE, SIG_IGN);

  // 输出当前进程的PID和进程组ID
  printf("Process ID: %d, Process Group ID: %d is working!\n", getpid(), getpgrp());

  int pid;
  tran_protocol_t protocol;
  while (true)
  {

    int client_socket = recv_fd_un(main_fd, write);
    ERROR_CHECK(client_socket, -1, "recv_fd_un error");

    while (true)
    {
      // printf("the client_socket is : %d\n", client_socket);
      bzero(&protocol, sizeof(tran_protocol_t));
      int n = recv_n(client_socket, &protocol, sizeof(tran_protocol_t));
      if (n == 0)
      {
        break;
      }
      if (protocol.type == 0x01)
      {
        // 发送文件
        send_file(&protocol, client_socket);
        break;
      }
      else if (protocol.type == 0x03)
      {
        // 发送文件列表
        bzero(&protocol, sizeof(tran_protocol_t));
        get_file_list(protocol.filename);
        protocol.type = 0x04;
        send(client_socket, &protocol, sizeof(tran_protocol_t), 0);
      }
    }
    pid = getpid();
    send(main_fd, &pid, sizeof(pid_t), 0);
  }
}

int send_file(tran_protocol_t *protocol, int client_socket)
{
  char file_path[1024];
  realpath(protocol->filename, file_path);
  // 获取文件信息
  struct stat st;
  stat(file_path, &st);
  printf("start send file %s\n", file_path);
  int file_fd = open(file_path, O_RDONLY);
  if (file_fd == -1)
  {
    perror("open file failed");
    char buf[] = "open file failed: NO such file\n";
    send(client_socket, buf, sizeof(buf), 0);
    close(client_socket);
    return -1;
  }
  // 设置TCP_CORK
  int flag = 1;
  setsockopt(client_socket, IPPROTO_TCP, TCP_CORK, &flag, sizeof(flag));

  // 设置协议块
  bzero(protocol, sizeof(tran_protocol_t));
  protocol->type = 0x05;
  protocol->len = st.st_size;
  send(client_socket, protocol, sizeof(tran_protocol_t), 0);

  // 发送文件
  off_t offset = 0;
  // Sendfile包装函数
  int ret = Sendfile(client_socket, file_fd, &offset, st.st_size);
  printf("send bytes %d\n", ret);

  // 关闭文件描述符
  close(file_fd);
  close(client_socket);
  if (ret <= 0)
  {
    return -1;
  }
  printf("PID:%d send file success\n", getpid());
  return ret;
}