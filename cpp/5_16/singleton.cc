#include <iostream>
using namespace std;

class Singleton
{
private:
  static SingletonHolder holder;
};

class SingletonHolder
{
  static const Singleton *instance = new Singleton();
};
int main()
{
  return 0;
}