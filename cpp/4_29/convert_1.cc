// #include <iostream>
// using namespace std;

// class Data_1
// {
// private:
//   int *data = nullptr;
//   int no = -1;

// public:
//   static int count;
//   Data_1(int d) : data(new int(d))
//   {
//     if (no == -1)
//       no = count++;
//     cout << "Data_1 constructor:" << *data << "[" << no << "]" << endl;
//   }
//   Data_1() = default;
//   Data_1(const Data_1 &other) : data(new int(*other.data)) // Copy constructor
//   {
//     if (no == -1)
//       no = count++;
//     cout << "Data_1 copy constructor:" << *data << "[" << no << "]" << endl;
//   }
//   int getData()
//   {
//     if (data == nullptr)
//       return -1;
//     cout << "Data_1 getData:" << *data << "[" << no << "]" << endl;
//     return *data;
//   }
//   ~Data_1()
//   {
//     if (data != nullptr)
//     {
//       cout << "Data_1 destructor:" << *data << "[" << no << "]" << endl;
//     }
//     else
//     {
//       cout << "Data_1 destructor: nullptr"
//            << "[" << no << "]" << endl;
//     }

//     if (data != nullptr)
//     {
//       delete data;
//       data = nullptr;
//       if (data == nullptr)
//         cout << "Data_1 destructor: data is nullptr" << endl;
//     }
//   }
// };

// int Data_1::count = 0;

// class Data_2
// {
// private:
//   int data = 0;

// public:
//   Data_2(int d) : data(d) {}
//   Data_2() = default;
//   int getData() { return data; }
//   int getData() const { return data; }
//   operator Data_1()
//   {
//     cout << "Data_2 to Data_1 conversion\n";
//     Data_1 val(data);
//     return val;
//   }
// };

// int main(void)
// {
//   Data_2 d2(20);
//   Data_1 d1 = d2;
//   Data_1 d3(30);
//   d3.getData();
//   d1.getData();
//   d3.getData();

//   d3 = d2; // 这里为什么会产生double free
//   d3.getData();
//   return 0;
// }
#include <iostream>
using namespace std;

class Data_2; // Forward declaration of Data_2 class
class Data_1
{
private:
  int *data = nullptr;
  int no = -1;

public:
  static int count;
  Data_1(int d) : data(new int(d))
  {
    if (no == -1)
      no = count++;
    cout << "Data_1 constructor:" << *data << "[" << no << "]" << endl;
  }
  Data_1() = default;
  Data_1(const Data_1 &other) : data(new int(*other.data)) // Copy constructor
  {
    if (no == -1)
      no = count++;
    cout << "Data_1 copy constructor:" << *data << "[" << no << "]" << endl;
  }
  int getData()
  {
    if (data == nullptr)
      return -1;
    cout << "Data_1 getData:" << *data << "[" << no << "]" << endl;
    return *data;
  }
  // Data_1 &operator=(const Data_2 &other);
  Data_1 &operator=(const Data_1 &other)
  {
    cout << "Data_1 assignment operator Data_1:" << *other.data << endl;

    if (data != nullptr)
    {
      delete data;
      data = nullptr;
    }
    data = new int(*other.data);
    return *this;
  }
  ~Data_1()
  {
    if (data != nullptr)
    {
      cout << "Data_1 destructor:" << *data << "[" << no << "]" << endl;
      delete data;
    }
    else
    {
      cout << "Data_1 destructor: nullptr"
           << "[" << no << "]" << endl;
    }
  }
};

int Data_1::count = 0;

class Data_2
{
private:
  int data = 0;

public:
  Data_2(int d) : data(d) {}
  Data_2() = default;
  int getData() { return data; }
  int getData() const { return data; }
  operator Data_1() // 直接返回对象而不是指针
  {
    cout << "Data_2 to Data_1 conversion\n";
    return Data_1(data); // 返回对象
  }
};

// Data_1 &Data_1::operator=(const Data_2 &other)
// {
//   cout << "Data_1 assignment operator:" << other.getData() << endl;

//   if (data != nullptr)
//   {
//     delete data;
//     data = nullptr;
//   }
//   data = new int(other.getData());
//   return *this;
// }

int main(void)
{
  Data_2 d2(20);
  Data_1 d1 = d2; // Explicit conversion
  Data_1 d3(30);
  d3.getData();
  d1.getData();
  d3.getData();

  d3 = d2; // 不会产生double free
  d3.getData();
  return 0;
}