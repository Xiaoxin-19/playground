#ifndef __TASKQUEUE_H__
#define __TASKQUEUE_H__

#include <queue>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include "Task.h"

using std::condition_variable;
using std::mutex;
using std::queue;
using std::unique_lock;

template <typename TaskType = Task<> *>
class TaskQueue
{
public:
    TaskQueue(size_t queSize);
    ~TaskQueue();

    // 生产数据
    void push(TaskType ptask);

    // 获取数据
    TaskType pop();
    bool full() const;
    bool empty() const;

    // 将所有等在在_notEmpty条件变量上的线程唤醒
    void weak_and_stop_push();

private:
    size_t _queSize;              // 任务队列的大小
    queue<TaskType> _que;   // 存放数据的数据结构
    mutex _mutex;                 // 互斥锁
    condition_variable _notFull;  // 非满条件变量
    condition_variable _notEmpty; // 非空条件变量
    bool _allow_push_flag;
};

#endif
