#include "thread_pool.h"
#include <signal.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include "queue.h"
#include "socket_m.h"
#include "error.h"
#include "signal_m.h"
#include "wrok.h"
#include "global.h"

void sigint_handler(int signum);
int start_server(int, Queue *);
void *handler_exit(void *);

int farther_son_fd[2];

int main(int argc, char *argv[])
{
  // 参数检查
  ARGS_CHECK(argc, 3);
  int server_fd;

  // 创建父子进程通信管道
  ERROR_CHECK(pipe(farther_son_fd), -1, "pipe error");
  // 创建子进程
  pid_t pid;
  ERROR_CHECK((pid = fork()), -1, "fork error");
  if (pid != 0)
  {
    close(farther_son_fd[0]);

    // 设置SIGINT信号处理函数
    signal_m(SIGINT, sigint_handler);

    // 接收子进程的退出通知
    wait(NULL);
    exit(0);
  }

  // 设置SIGINT信号处理函数
  signal(SIGINT, SIG_IGN);
  pthread_t tid;
  pthread_create(&tid, NULL, handler_exit, NULL);

  close(farther_son_fd[1]);

  job_queue = create_queue();
  ERROR_CHECK(job_queue, NULL, "create_queue error");

  // 初始化服务器
  ERROR_CHECK((server_fd = init_server(SOCK_STREAM, argv[1], atoi(argv[2]))), -1, "init_server error");

  // 创建线程池
  pool = create_thread_pool(job_queue);
  ERROR_CHECK(pool, NULL, "create_thread_pool error");
  ERROR_CHECK((start_server(server_fd, job_queue)), -1, "start_server error");

  // 释放进程池
  destroy_thread_pool(pool);
  free_global_var();

  // 关闭服务器
  close(server_fd);

  return 0;
}

int start_server(int server_fd, Queue *job_queue)
{
  // 屏蔽SIGPIPE信号
  signal_m(SIGPIPE, SIG_IGN);
  // TODO: 使用epoll 监听来自父进程的通知，设置退出标记，并倒数60秒后退出。
  // 监听请求，并加入工作队列
  while (true)
  {
    int newfd = accept(server_fd, NULL, NULL);
    ERROR_CHECK(newfd, -1, "accept error");
    add_job(job_queue, newfd);
  }
}

/// @brief SIGINT信号处理函数
/// @param signum
void sigint_handler(int signum)
{
  char buf[5] = "quit";
  write(farther_son_fd[1], buf, sizeof(buf));
}

/// 接收父进程的退出通知
void *handler_exit(void *arg)
{
  char buf[128];
  ERROR_CHECK(read(farther_son_fd[0], buf, sizeof(buf)), -1, "read failed");
  if (strcmp(buf, "quit") == 0)
  {
    destroy_thread_pool(pool);
  }
  return 0;
}