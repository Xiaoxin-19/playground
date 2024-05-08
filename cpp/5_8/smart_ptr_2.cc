#include <iostream>
using std::cout;
using std::endl;

template <typename T>
class smart_ptr
{
private:
  T *ptr_;

public:
  explicit smart_ptr(T *p = nullptr) : ptr_(p) {}
  ~smart_ptr() { delete ptr_; }

  T *get() { return ptr_; }

  T &operator*() { return *ptr_; }
  T *operator->() { return ptr_; }
  operator bool() { return ptr_; }

  // 移动构造函数， 如果提供了移动构造，但没有显示提供拷贝构造， 则拷贝构造模式是被删除的
  smart_ptr(smart_ptr &&other)
  {
    ptr_ = other.release();
  }

  template <typename U>
  smart_ptr(smart_ptr<U> &&other)
  {
    ptr_ = other.release();
  }

  // 直接生成临时对象，rhs ，不用在函数体内重新构造
  smart_ptr &operator=(smart_ptr rhs)
  {
    rhs.swap(*this);
    return *this;
  }

  T *release()
  {
    T *ptr = ptr_;
    ptr_ = nullptr;
    return ptr;
  }

  void swap(smart_ptr &rhs)
  {
    using std::swap;
    swap(ptr_, rhs.ptr_);
  }
};

class shape
{
public:
  virtual ~shape()
  {
    cout << "~ shape" << endl;
  }

  shape()
  {
    cout << "shape()" << endl;
  }
};

class circle : public shape
{
public:
  circle() : shape() { cout << "circle" << endl; }
  ~circle() { cout << "~circle" << endl; }
};

int main()
{
  smart_ptr<shape> ptr1{new circle()};
  // smart_ptr<shape> ptr2{ptr1}; // 编译错误， 因为拷贝构造函数被删除了
  smart_ptr<shape> ptr3;
  // ptr3 = ptr1; // 编译处错， 因为拷贝赋值函数被删除了
  ptr3 = std::move(ptr1);
  smart_ptr<shape> ptr4{std::move(ptr3)};
  smart_ptr<circle> ptr5;
  ptr4 = std::move(ptr5);
  return 0;
}