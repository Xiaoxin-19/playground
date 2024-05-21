#include "TaskQueue.h"

TaskQueue<>::TaskQueue(size_t queSize)
    : _queSize(queSize), _que(), _mutex(), _notFull(), _notEmpty(), _allow_push_flag(true)
{
}

TaskQueue<>::~TaskQueue()
{
}

template <typename TaskType>
// 生产数据
void TaskQueue<TaskType>::push(TaskType ptask)
{
    unique_lock<mutex> autoLock(_mutex);

    while (full())
    {
        _notFull.wait(autoLock);
    }
    _que.push(ptask);
    _notEmpty.notify_one();
}

// 获取数据

template <typename TaskType>
TaskType TaskQueue<TaskType>::pop()
{
    unique_lock<mutex> autoLock(_mutex);
    while (empty() && _allow_push_flag)
    {
        _notEmpty.wait(autoLock);
    }

    if (_allow_push_flag)
    {
        ElemType tmp = _que.front();
        _que.pop();
        _notFull.notify_one();

        return tmp;
    }
    else
    {
        return nullptr;
    }
}

bool TaskQueue<>::full() const
{
    return _que.size() == _queSize;
}

bool TaskQueue<>::empty() const
{
    return _que.size() == 0;
}

// 将所有等在在_notEmpty条件变量上的线程唤醒
void TaskQueue<>::weak_and_stop_push()
{
    _allow_push_flag = false;
    _notEmpty.notify_all();
}
