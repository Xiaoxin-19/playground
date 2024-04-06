#ifndef WORK_H
#define WORK_H
#include "queue.h"
#include "thread_pool.h"


/// @brief 传输协议
typedef struct
{
  /// @brief ox01 表示下载文件 ox02 表示上传文件 ox03 显示服务器可下载文件列表, ox04 表示下载文件列表, ox05 表示下载文件信息
  unsigned char type;
  unsigned len;
  char filename[1024];
} tran_protocol_t;

typedef struct
{
  Queue *queue;
  thread_pool_t *pool;
} work_arg_t;

void *work(void *arg);
void add_job(Queue *q, int client_fd);
#endif // WORK_H