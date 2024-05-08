#include <iostream>
using namespace std;

class Data
{
private:
  int data = 0;

public:
  Data(int d) : data(d) {}
  Data() = default;
  int getData() { return data; }
};

class Middle
{
private:
  Data *pdata;

public:
  Middle(int d) : pdata(new Data(d)) {}
  Data *operator->()
  {
    return pdata;
  }
  Data &operator*()
  {
    return *pdata;
  }

  ~Middle()
  {
    delete pdata;
  }
};

int main(void)
{
  Middle m(10);
  cout << m->getData() << endl;

  Middle m2(20);
  cout << (*m2).getData() << endl;
  return 0;
}