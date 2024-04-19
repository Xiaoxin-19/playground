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
#include "epoll_m.h"

void sigint_handler(int signum);
int start_server(int, Queue *);
void child_process(const char *addr, const char *port);
void recv_quit();

int farther_son_fd[2];

int main(int argc, char *argv[])
{
  // 参数检查
  ARGS_CHECK(argc, 3);

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
  child_process(argv[1], argv[2]);
  return 0;
}

void child_process(const char *addr, const char *port)
{
  int server_fd;
  // 屏蔽中断信号
  signal_m(SIGINT, SIG_IGN);

  // 屏蔽SIGPIPE信号
  signal_m(SIGPIPE, SIG_IGN);
  close(farther_son_fd[1]);

  job_queue = create_queue();
  ERROR_CHECK(job_queue, NULL, "create_queue error");

  // 初始化服务器
  ERROR_CHECK((server_fd = init_server(SOCK_STREAM, addr, atoi(port))), -1, "init_server error");

  // 创建线程池
  pool = create_thread_pool(job_queue);
  ERROR_CHECK(pool, NULL, "create_thread_pool error");
  ERROR_CHECK((start_server(server_fd, job_queue)), -1, "start_server error");

  // 释放进程池
  destroy_thread_pool(pool);
  free_global_var();

  // 关闭服务器
  close(server_fd);
}

int start_server(int server_fd, Queue *job_queue)
{

  int epoll_fd = Epoll_create();
  struct epoll_event *ev = NULL, *ev_server = NULL;
  struct epoll_event evlists[3];
  add_read_to_epoll(epoll_fd, server_fd, ev_server);
  add_read_to_epoll(epoll_fd, farther_son_fd[0], ev);
  // 监听请求，并加入工作队列
  while (true)
  {
    int ret = epoll_wait(epoll_fd, evlists, 3, -1);
    ERROR_CHECK(ret, -1, "epoll wait failed");
    for (int i = 0; i < ret; i++)
    {
      int curr_fd = evlists[i].data.fd;
      if (curr_fd == server_fd)
      {
        int newfd = accept(server_fd, NULL, NULL);
        ERROR_CHECK(newfd, -1, "accept error");
        add_job(job_queue, newfd);
      }
      else if (curr_fd == farther_son_fd[0])
      {
        recv_quit();
      }
    }
  }
}

void recv_quit()
{
  char buf[128];
  printf("receive exit from parent\n");
  ERROR_CHECK(read(farther_son_fd[0], buf, sizeof(buf)), -1, "read failed");
  if (strcmp(buf, "quit") == 0)
  {
    destroy_thread_pool(pool);
  }
}
/// @brief SIGINT信号处理函数
/// @param signum
void sigint_handler(int signum)
{
  char buf[5] = "quit";
  write(farther_son_fd[1], buf, sizeof(buf));
}