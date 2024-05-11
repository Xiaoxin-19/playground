#include <iostream>
#include <vector>
#include <random>
#include <initializer_list>

using std::begin;
using std::cout;
using std::end;
using std::endl;
using std::less;
using std::vector;

// hoare partition implement quick sort
template <typename T, typename Compare = std::less<T>>
class MyQsort
{

private:
  vector<T> vec_;

public:
  MyQsort(T *arr, size_t size, Compare cmp) : vec_(arr, arr + size)
  {
    cout << "constructor" << endl;
    this->print();
    cout << "-------------------" << endl;
    quick(0, size - 1, cmp);
  }

  void print()
  {
    for (auto &i : vec_)
    {
      cout << i << " ";
    }
    cout << endl;
  }

private:
  void quick(int left, int right, Compare &cmp)
  {
    if (left >= right)
      return;
    int pivot = partition(left, right, cmp);
    quick(left, pivot, cmp);
    quick(pivot + 1, right, cmp);
  }

  int partition(int left, int right, Compare &cmp)
  {
    cout << "left:" << left << " right:" << right << endl;
    std::random_device rd;  // 随机设备，用于产生种子
    std::mt19937 gen(rd()); // 梅森旋转法生成器，使用随机设备生成种子

    // 生成指定范围内的随机数
    std::uniform_int_distribution<> dis(left, right);
    T pivot = vec_.at(dis(gen));
    cout << "pivot:" << pivot << endl;

    while (left <= right)
    {
      int r = right, l = left;
      // left < right && cmp(vec_.at(left), pivot) 条件不等于 cmp(vec_.at(l), pivot)
      // 当 i < j && a[i] < pivot ， i == j 不会进入if循环
      // 当 i == j ,a[i] < pivot 则 < 返回true 进入 if 循环 这导致 返回
      // while (left < right && cmp(vec_.at(left), pivot))
      //   left++;
      // while (left < right && cmp(pivot, vec_.at(right)))
      //   right--;
      while (cmp(vec_.at(left), pivot))
        left++;
      while (cmp(pivot, vec_.at(right)))
        right--;
      while (cmp(vec_.at(l), pivot))
        l++;
      while (cmp(pivot, vec_.at(r)))
        r--;
      cout << "----after while op: " << l << "-" << r << endl;
      cout << "++++after while op: " << left << "-" << right << endl;
      if (left == right)
      {
        cout << "left == right break \n";
        break;
      }

      if (left < right)
      {
        T temp = vec_.at(left);
        vec_.at(left) = vec_.at(right);
        vec_.at(right) = temp;
        ++left;
        --right;
        cout << "after swap: left : right " << left << " : " << right << endl;
      }
      this->print();
    }

    this->print();
    cout << "left: " << left << endl;
    cout << "return :" << right << endl;
    cout << "-------------------" << endl;
    return right;
  }
};

class MyValue
{
private:
  int value_;

public:
  int get_value() const { return value_; }
  void set_value(int value) { value_ = value; }
  MyValue() : value_(0) {}
  MyValue(int value) : value_(value) {}
  friend std::ostream &operator<<(std::ostream &os, const MyValue &v);
};

std::ostream &operator<<(std::ostream &os, const MyValue &v)
{
  os << v.get_value();
  return os;
}

int main()
{
  MyValue arr[10] = {MyValue(9), MyValue(8), MyValue(7), MyValue(6), MyValue(5), MyValue(4), MyValue(3), MyValue(2), MyValue(1), MyValue(0)};
  auto cmp = [](const MyValue &a, const MyValue &b) -> bool
  { return a.get_value() < b.get_value(); };
  MyQsort<MyValue, decltype(cmp)> msort(arr, sizeof(arr) / sizeof(arr[0]), cmp);
  msort.print();
  return EXIT_SUCCESS;
}