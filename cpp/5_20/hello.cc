#include <iostream>
#include <thread>
using namespace std;

int main()
{
  thread t([]()
           { cout << "hello world" << endl; });
  t.join();
  cout<<std::thread::hardware_concurrency()<<endl;
  return 0;
}