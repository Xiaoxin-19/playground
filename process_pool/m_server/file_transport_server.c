#include "common.h"

void sigint_handler(int signum);
int start_server(int, process_pool_t *);
void *assign_job(void *arg);
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
Queue *job_queue;

void cleanup();

int main(int argc, char *argv[])
{
  if (atexit(cleanup) != 0)
  {
    perror("atexit");
    return 1;
  }
  ARGS_CHECK(argc, 3);
  int server_fd;
  // 设置SIGINT信号处理函数
  signal_m(SIGINT, sigint_handler);

  // 初始化服务器
  ERROR_CHECK((server_fd = init_server(SOCK_STREAM, argv[1], atoi(argv[2]))), -1, "init_server error");

  // 创建进程池
  process_pool_t *pool = create_process_pool();
  ERROR_CHECK(pool, NULL, "create_process_pool error");
  ERROR_CHECK((start_server(server_fd, pool)), -1, "start_server error");

  // 释放进程池
  destroy_process_pool(pool);
  // 关闭服务器
  close(server_fd);
  return 0;
}

#define MAX_EVENTS 20
int start_server(int server_fd, process_pool_t *pool)
{

  int epoll_fd = Epoll_create();
  struct epoll_event evlist[MAX_EVENTS];
  struct epoll_event *ev = NULL;
  job_queue = create_queue();
  pthread_t tid;
  THREAD_ERROR_CHECK(pthread_create(&tid, NULL, assign_job, pool), "pthread_create");

  // 添加服务器套接字到epoll
  add_read_to_epoll(epoll_fd, server_fd, ev);

  // 添加进程池中的套接字到epoll
  for (int i = 0; i < pool->size; i++)
  {
    add_read_to_epoll(epoll_fd, pool->list[i].fd[1], &(pool->list[i].ev));
  }

  while (true)
  {
    int ret = Epoll_wait(epoll_fd, evlist, MAX_EVENTS, -1);
    for (int i = 0; i < ret; i++)
    {
      if (evlist[i].data.fd == server_fd)
      {
        int newfd = accept(server_fd, NULL, NULL);
        ERROR_CHECK(newfd, -1, "accept error");
        pthread_mutex_lock(&mutex);
        // 添加新连接到任务队列
        enqueue(job_queue, newfd);
        pthread_mutex_unlock(&mutex);
        // 通知任务分配线程有新任务
        pthread_cond_signal(&cond);
      }
      else
      {
        // TODO: 子进程处理完毕，重置状态
        int fd = evlist[i].data.fd;
        pid_t pid;
        recv(fd, &pid, sizeof(pid), 0);
        set_process_status(pool, pid, IDLE);
        printf("process %d is idle\n--------------------------\n", pid);
        // 通知任务分配线程有进程空闲
        pthread_cond_signal(&cond);
      }
    }
  }
  free(ev);
  close(epoll_fd);
}

void cleanup()
{
  kill(0, SIGKILL);
}

/// @brief SIGINT信号处理函数,发送SIGINT信号给进程组内所有进程，并退出
/// @param signum
void sigint_handler(int signum)
{
  printf("\nProcess with Group ID: %d is exiting!\n", getpgrp());
  cleanup();
  exit(1);
}

void *assign_job(void *arg)
{
  printf("assign job Thread %ld is running\n", pthread_self());
  process_pool_t *pool = (process_pool_t *)arg;
  while (true)
  {
    pthread_mutex_lock(&mutex);
    while (job_queue->size == 0)
    {
      pthread_cond_wait(&cond, &mutex);
    }

    // 从任务队列中取出任务
    for (int i = 0; i < pool->size; i++)
    {
      if (pool->list[i].status == IDLE)
      {
        int newfd = front(job_queue);
        dequeue(job_queue);
        // 分配任务给空闲进程
        set_process_status(pool, pool->list[i].pid, BUSY);
        printf("Assign socked_fd:%d to process %d\n", newfd, pool->list[i].pid);
        send_fd_un(pool->list[i].fd[1], newfd);
        close(newfd);
        break;
      }
    }
    pthread_mutex_unlock(&mutex);
  }
}