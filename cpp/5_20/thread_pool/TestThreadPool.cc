#include "Task.h"
#include "ThreadPool.h"
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <memory>
#include <tuple>

using std::cout;
using std::endl;
using std::get;
using std::make_tuple;
using std::unique_ptr;

class MyTask
    : public Task<void>
{
public:
    void process() override
    {
        // 到底应该做什么呢?
        // shift + k
        ::srand(::clock());
        int number = ::rand() % 100;
        cout << "MyTask process number = " << number << endl;
    }
};

class TaksWithArgs : public Task<int, double>
{
public:
    TaksWithArgs(const std::tuple<int, double> &tt) : Task(tt) {}
    void process() override
    {
        auto args1 = get<0>(this->ttl);
        auto args2 = get<1>(this->ttl);
        cout << "the args 1 : " << args1 << endl;
        cout << "the args 2 : " << args2 << endl;
    }
};

void test()
{
    unique_ptr<Task<>> ptask(new MyTask());
    std::tuple<int, double> params(42, 3.14);
    unique_ptr<Task<int, double>> mtask(new TaksWithArgs(params));
    ThreadPool<Task<void>> pool(4, 10);
    pool.start();

    int cnt = 20;
    while (cnt--)
    {
        // 添加任务
        if (cnt >= 10)
        {
            pool.addTask(mtask.get());
        }
        else
        {
            pool.addTask(ptask.get());
        }
        cout << "cnt = " << cnt << endl;
    }
    pool.stop();
}

int main(int argc, char *argv[])
{
    test();
    return 0;
}
