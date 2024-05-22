#ifndef __TASK_H__
#define __TASK_H__
#include <tuple>
#include <functional>
using std::tuple;

class IEventHalder
{
public:
    virtual void process() = 0;
    virtual ~IEventHalder() = default;
};

template <typename... Args>
class EventHalder : public IEventHalder
{
public:
    EventHalder(func, args...)
    {
    }

private:
    std::funciton<void> func;
};

template <typename... Args>
class Task
{
public:
    Task();
    virtual ~Task();
    Task(const tuple<Args...> &tt);
    tuple<Args...> ttl;
    virtual void process() = 0;
};

template <typename... Args>
Task<Args...>::Task(const std::tuple<Args...> &tt) : ttl(tt)
{
    // 构造函数的实现
}

// 析构函数不需要模板参数列表
template <typename... Args>
Task<Args...>::~Task()
{
    // 如果需要在析构函数中执行特殊操作，可以在这里添加代码
}

#endif
