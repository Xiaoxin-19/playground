#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include "thread_pool.h"
#include "wrok.h"
#include "error.h"
#include "socket_m.h"
#include "queue.h"
#include "global.h"

long get_num_cores();

/// @brief 初始化线程池 返回线程池指针，动态分配内存，需要外部手动释放
/// @return thread_pool_t* 线程池指针
thread_pool_t *create_thread_pool(Queue *q)
{
  thread_pool_t *pool = (thread_pool_t *)calloc(1, sizeof(thread_pool_t));
  ERROR_CHECK(pool, NULL, "init thread pool error");
  pool->status = RUNNING;
  int core_num = get_num_cores();
  int thread_num = core_num * 2;
  work_arg = (work_arg_t *)calloc(1, sizeof(work_arg_t));
  work_arg->queue = q;
  work_arg->pool = pool;
  for (int i = 0; i < thread_num; i++)
  {
    thread_t tid;
    THREAD_ERROR_CHECK(pthread_create(&tid.tid, NULL, work, work_arg), "pthread_create");
    pool->list[pool->size++] = tid;
  }
  return pool;
}

void destroy_thread_pool(thread_pool_t *pool)
{
  pool->status = QUIT;
  for (int i = 0; i < pool->size; i++)
  {
    pthread_join(pool->list[i].tid, NULL);
  }
  free_global_var();
  printf("destroy thread pool success, child process exiting\n");
  exit(0);
}

/// @brief 获取CPU核心数
/// @return long CPU核心数
long get_num_cores()
{
  long num_cores = sysconf(_SC_NPROCESSORS_CONF);
  if (num_cores < 1)
  {
    perror("Error: Unable to determine the number of CPU cores.\n");
    return 1;
  }

  printf("Number of CPU cores: %ld\n", num_cores);
  return num_cores;
}