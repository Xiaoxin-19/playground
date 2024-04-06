#include <stdlib.h>
#include "global.h"

/// @brief 释放全局变量 work_arg, pool, job_queue 的内存
void free_global_var()
{
  free(work_arg);
  free(pool);
  free(job_queue);
  return;
}