#include "proc_pool.h"
#include "wrok.h"
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include "error.h"
#include "socket_m.h"

/// @brief 初始化进程池
/// @return process_pool_t* 进程池指针 动态分配内存，需要外部手动释放
process_pool_t *init_process_pool()
{
  process_pool_t *pool = (process_pool_t *)calloc(1, sizeof(process_pool_t));
  ERROR_CHECK(pool, NULL, "init process pool error");
  pool->size = 0;
  return pool;
}

/// @brief 创建进程池
/// @return 进程池指针， 动态分配，需要调用destroy_process_pool释放
process_pool_t *create_process_pool()
{
  // 创建进程池
  process_pool_t *pool = init_process_pool();
  pid_t pid = getpid(); // 获取当前进程的PID
  if (setpgid(pid, pid) == -1)
  {
    perror("setpgid");
    exit(1);
  }
  // 输出当前进程的PID和进程组ID
  printf("Process ID: %d, Process Group ID: %d\n", getpid(), getpgrp());

  // 获取CPU核心数
  long num_cores = get_num_cores();
  int main_fd;
  // 创建子进程
  for (int i = 0; i < num_cores * 2; i++)
  {
    int fd[2];
    Socketpair(fd);
    main_fd = fd[0];
    pid = fork();
    if (pid == 0)
    {
      // 子进程
      break;
    }
    else
    {
      // 父进程
      add_process(pool, pid, fd);
    }
  }

  // 子进程执行工作函数
  if (pid == 0)
  {
    work(main_fd);
  }

  return pool;
}

/// @brief 为进程池添加进程
/// @param pool  进程池指针
/// @param pid 进程ID
/// @param fd Unix域套接字描述符
void add_process(process_pool_t *pool, int pid, int fd[2])
{
  int idx = pool->size;
  pool->list[idx].pid = pid;
  pool->list[idx].status = IDLE;
  printf("Process %d added to process pool\n", pid);
  printf("Process %d fd[0]: %d, fd[1]: %d\n", pid, fd[0], fd[1]);
  pool->list[idx].fd[0] = fd[0];
  pool->list[idx].fd[1] = fd[1];
  pool->size++;
}

/// @brief 销毁进程池
/// @param pool 进程池指针
/// @return
int destroy_process_pool(process_pool_t *pool)
{
  for (int i = 0; i < pool->size; i++)
  {
    printf("Killing process %d\n", pool->list[i].pid);
    pid_t pid = pool->list[i].pid;
    kill(pid, SIGKILL);
    close(pool->list[i].fd[0]);
    close(pool->list[i].fd[1]);
  }
  free(pool);
  return 0;
}

/// @brief 获取CPU核心数
/// @return
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

void set_process_status(process_pool_t *pool, int pid, process_status_t status)
{
  for (int i = 0; i < pool->size; i++)
  {
    if (pool->list[i].pid == pid)
    {
      pool->list[i].status = status;
      printf("Process %d status set to %d\n", pid, status);
      break;
    }
  }
}