
template <typename T>
class smart_ptr
{
private:
  T *ptr_;

public:
  explicit smart_ptr(T *p = nullptr) : ptr_(p) {}
  ~smart_ptr()
  {
    delete ptr_;
  }

  T *get() { return ptr_; }

  T &operator*() { return *ptr_; }
  T *operator->() { return ptr_; }
  operator bool() { return ptr_; }

  smart_ptr(smart_ptr &other)
  {
    ptr_ = other.release();
  }

  smart_ptr &operaotr = (smart_ptr & rhs)
  {
    smart_ptr(rhs).swap(*this);
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