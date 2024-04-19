#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>
typedef struct
{
  pthread_t tid;
} thread_t;

typedef enum
{
  RUNNING,
  QUIT,
} thread_pool_status;

typedef struct
{
  thread_t list[10];
  int size;
  thread_pool_status status;
} thread_pool_t;

thread_pool_t *create_thread_pool();
void destroy_thread_pool(thread_pool_t *pool);
#endif