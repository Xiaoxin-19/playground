#ifndef GOBAL_H
#define GOBAL_H
#include "queue.h"
#include "thread_pool.h"
#include "wrok.h"

#define SA struct sockaddr
Queue *job_queue;
thread_pool_t *pool;
work_arg_t *work_arg;

void free_global_var();
#endif