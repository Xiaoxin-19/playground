#ifndef PROC_POOL_H
#define PROC_POOL_H
#include <sys/epoll.h>

typedef enum
{
  BUSY,
  IDLE
} process_status_t;

typedef struct
{
  int pid;
  int status;
  struct epoll_event ev;
  int fd[2];
} process_t;

typedef struct
{
  process_t list[10];
  int size;
} process_pool_t;

process_pool_t *init_process_pool();
process_pool_t *create_process_pool();
void add_process(process_pool_t *pool, int pid, int fd[2]);
int destroy_process_pool(process_pool_t *pool);
void set_process_status(process_pool_t *pool, int pid, process_status_t status);
long get_num_cores();

#endif