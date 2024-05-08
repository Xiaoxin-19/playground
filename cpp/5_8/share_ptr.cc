#include <iostream>
using std::cout;
using std::endl;

class shared_count
{
public:
  shared_count() : count_(1) {}
  void add_count() { ++count_; }
  long reduce_count() { return --count_; }
  long get_count() const { return count_; };

private:
  long count_;
};

template <typename T>
class smart_ptr
{
private:
  T *ptr_;
  shared_count *shared_count_;

public:
  template <typename U>
  friend class smart_ptr;

  explicit smart_ptr(T *p = nullptr) : ptr_(p)
  {
    if (p)
    {
      shared_count_ = new shared_count();
    }
  }
  ~smart_ptr()
  {
    if (ptr_ && !shared_count_->reduce_count())
    {
      delete ptr_;
      delete shared_count_;
    }
  }

  T *get() { return ptr_; }

  T &operator*() { return *ptr_; }
  T *operator->() { return ptr_; }
  operator bool() { return ptr_; }

  smart_ptr(const smart_ptr &other)
  {
    ptr_ = other.ptr_;
    if (ptr_)
    {
      other.shared_count_->add_count();
      shared_count_ = other.shared_count_;
    }
  }

  template <typename U>
  smart_ptr(const smart_ptr<U> &other)
  {
    ptr_ = other.ptr_;
    if (ptr_)
    {
      other.shared_count_->add_count();
      shared_count_ = other.shared_count_;
    }
  }

  template <typename U>
  smart_ptr(smart_ptr<U> &&other)
  {
    ptr_ = other.ptr_;
    if (ptr_)
    {
      shared_count_ = other.shared_count_;
      other.ptr_ = nullptr;
    }
  }

  // 直接生成临时对象，rhs ，不用在函数体内重新构造
  smart_ptr &operator=(smart_ptr rhs)
  {
    rhs.swap(*this);
    return *this;
  }

  long use_count() const
  {
    if (ptr_)
    {
      return shared_count_->get_count();
    }
    else
    {
      return 0;
    }
  }

  void swap(smart_ptr &rhs)
  {
    using std::swap;
    swap(ptr_, rhs.ptr_);
    swap(shared_count_, rhs.shared_count_);
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