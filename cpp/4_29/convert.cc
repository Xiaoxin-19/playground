#include <iostream>
using namespace std;
class Data_2
{
private:
  int data = 0;

public:
  Data_2(int d) : data(d) {}
  Data_2() = default;
  int getData() { return data; }
  int getData() const { return data; }
  // operator Data_1()
  // {
  //   cout << "Data_2 to Data_1 conversion\n";
  //   Data_1 *tem = new Data_1(data);
  //   return (*tem);
  // }
};

class Data_1
{
private:
  int *data = nullptr;

public:
  Data_1(int d) : data(new int(d))
  {
    cout << "Data_1 constructor:" << *data << endl;
  }
  Data_1() = default;
  Data_1(const Data_1 &other) : data(new int(*other.data)) // Copy constructor
  {
    cout << "Data_1 copy constructor:" << *data << endl;
  }
  Data_1(const Data_2 &other) // Copy constructor
  {
    int temp = other.getData();
    data = new int(temp);
    cout << "Data_1 copy constructor:" << *data << endl;
  }
  int getData()
  {
    if (data == nullptr)
      return -1;
    return *data;
  }
  ~Data_1()
  {
    if (data != nullptr)
    {
      cout << "Data_1 destructor:" << *data << endl;
    }
    else
    {
      cout << "Data_1 destructor: nullptr" << endl;
    }

    if (data != nullptr)
    {
      delete data;
      data = nullptr;
    }
  }

  Data_1 &operator=(const Data_2 &other)
  {
    cout << "Data_1 assignment operator:" << other.getData() << endl;

    if (data != nullptr)
    {
      delete data;
      data = nullptr;
    }
    data = new int(other.getData());
    return *this;
  }
};

int main(void)
{
  Data_2 d2(20);
  Data_1 d1 = d2;
  Data_1 d3(30);
  cout << "1:  " << d3.getData() << endl;
  cout << "2:  " << d1.getData() << endl;
  cout << "3:  " << d3.getData() << endl;
  d3 = d2;
  cout << "4:  " << d3.getData() << endl;
  return 0;
}