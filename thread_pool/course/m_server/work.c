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
#include <pthread.h>
#include <global.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include "wrok.h"
#include "socket_m.h"
#include "error.h"
#include "queue.h"

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int send_file(tran_protocol_t *protocol, int client_socket);
void get_file_list(char *buf)
{
  char p[] = "hello.txt\n /home/ubuntu/etc/assp.mkv\n";
  strncpy(buf, p, strlen(p));
}

/// @brief 工作
/// @return
void *work(void *arg)
{
  work_arg_t *work_arg = (work_arg_t *)arg;
  Queue *queue = work_arg->queue;
  thread_pool_t *pool = work_arg->pool;
  // 输出当前线程的ID
  printf("thread[%lu]: is running\n", pthread_self());
  pthread_t tid = pthread_self();
  tran_protocol_t protocol;

  while (true)
  {
    // 检测是否退出
    if (pool->status == QUIT)
    {
      break;
    }
    pthread_mutex_lock(&mutex);
    while (queue->size == 0)
    {
      pthread_cond_wait(&cond, &mutex);
    }
    int client_socket = front(queue);
    printf("thread[%lu]: get client_socket %d\n", tid, client_socket);
    dequeue(queue);
    pthread_mutex_unlock(&mutex);

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
    printf("thread[%lu]: close client_socket %d, complate job\n", tid, client_socket);
    close(client_socket);
  }
  printf("thread [%lu]: is exiting\n", tid);
  return (void *)0;
}

/// @brief 添加任务
/// @param job_queue 任务队列
/// @param client_fd 客户端文件描述符
void add_job(Queue *job_queue, int client_fd)
{
  pthread_mutex_lock(&mutex);
  enqueue(job_queue, client_fd);
  pthread_mutex_unlock(&mutex);
  pthread_cond_signal(&cond);
}

int send_file(tran_protocol_t *protocol, int client_socket)
{
  char file_path[1024];
  realpath(protocol->filename, file_path);
  // 获取文件信息
  struct stat st;
  stat(file_path, &st);
  printf("pthead[%lu]:start send file %s\n", pthread_self(), file_path);
  int file_fd = open(file_path, O_RDONLY);
  if (file_fd == -1)
  {
    perror("open file failed");
    char buf[] = "open file failed: NO such file\n";
    send(client_socket, buf, sizeof(buf), 0);
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
  ssize_t ret = Sendfile(client_socket, file_fd, &offset, st.st_size);

  // 关闭文件描述符
  close(file_fd);
  if (ret <= 0)
  {
    return -1;
  }
  printf("thred[%lu]: send file success, transport %ld bytes\n", pthread_self(), ret);
  return ret;
}