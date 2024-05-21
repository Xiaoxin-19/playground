#include "ThreadPool.h"
#include "Task.h"
#include <iostream>

using std::cout;
using std::endl;

ThreadPool<>::ThreadPool(size_t threadNum, size_t queSize)
    : _threadNum(threadNum), _queSize(queSize), _taskQue(_queSize), _isExit(false)
{
    // 为了防止vector频繁扩容的问题，预留空间
    // 其实没多大卵用
    _threads.reserve(_threadNum);
}

ThreadPool<>::~ThreadPool()
{
    /* if(!_isExit) */
    /* { */
    /*     stop(); */
    /* } */
}

// 线程池的启动与停止
void ThreadPool<void>::start()
{
    // 将所有的工作线程创建出来，同时也要启动起来
    for (size_t idx = 0; idx != _threadNum; ++idx)
    {
        /* thread  th(&ThreadPool::doTask, this); */
        /* _threads.push_back(std::move(th)); */
        _threads.push_back(thread(&ThreadPool<void>::doTask, this));
    }
}

// Q1:线程池并没有保证任务执行完，就已经将线程池退出来了?
// A1:因为任务没有执行完，主线程就已经回收了子线程（工作线程）
//
// Q2：任务可以执行完，但是线程池又无法退出？
// A2: 线程池之所以没有退出，是因为工作线程与主线程的配合问题
// 也就是主线程设置_isExit的时序问题，而导致了子线程处于睡眠
// 状态，解决方法是：在回收之前，将所有可能睡眠的线程唤醒之后
// 再进行回收

void ThreadPool<>::stop()
{
    // 只要工作线程没有将任务执行完，就不能向下执行
    while (!_taskQue.empty())
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    _isExit = true;

    // 将所有等待在_notEmpty条件变量上的线程唤醒（工作线程唤醒）
    /* _notEmpty.notify_all(); */
    _taskQue.weak_and_stop_push();

    // 将所有的工作线程进行回收
    for (auto &th : _threads)
    {
        th.join();
    }
}
template <typename TaskType>
// 任务的添加与获取
void ThreadPool<TaskType>::addTask(TaskType ptask)
{
    if (ptask)
    {
        _taskQue.push(ptask);
    }
}
template <typename TaskType>
TaskType ThreadPool<TaskType>::getTask()
{
    return _taskQue.pop();
}

// 线程池交给工作线程执行的任务
template <typename TaskType>
void ThreadPool<TaskType>::doTask()
{
    while (!_isExit)
    {
        // 现获取到任务
        // 工作线程在执行getTask分配完所有的任务之后，需要继续
        // 执行任务，也就是执行process函数，同时主线程在执行
        // stop的时候，只要任务为空，主线程也会继续向下执行，
        // 将线程池退出的标志位_isExit设置位true，如果主线程
        // 执行的比较快的话，那么对于子线程而言，再进行while
        // 循环条件判断的时候，就不会进入到while循环中，也就
        // 没有机会处于wait状态，也就是工作线程执行process的
        // 速度比较慢；如果子线程拿到任务之后，执行process的
        // 速速比较快，那么有可能在进入循环条件判断的时候，
        // 主线程还没有来得及将标志位_isExit设置位true，那么
        // 子线程就进入到while循环中，发现任务队列为空，那么
        // 工作线程就会处于睡眠状态
        TaskType *ptask = getTask();
        // 然后执行
        if (ptask)
        {
            ptask->process(); // 体现多态
            /* std::this_thread::sleep_for(std::chrono::seconds(3)); */
        }
        else
        {
            cout << "task == nullptr " << endl;
        }
    }
}
