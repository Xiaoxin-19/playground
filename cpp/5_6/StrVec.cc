#include <iostream>
#include <string>
#include <memory>
#include <utility>

using namespace std;

class StrVec
{
public:
  StrVec() : element(nullptr), first_free(nullptr), cap(nullptr) {}
  StrVec(const StrVec &s);
  StrVec &operator=(const StrVec &s);
  ~StrVec();
  size_t size() { return first_free - element; }
  size_t capacity() { return cap - element; }
  void push_back(const string &s);
  string *begin() const { return element; }
  string *end() const { return first_free; }

private:
  static allocator<string> alloc;
  string *element = nullptr;
  string *first_free = nullptr;
  string *cap = nullptr;
  void free();
  pair<string *, string *> alloc_n_copy(const string *, const string *);
  void chk_n_alloc();
};
allocator<string> StrVec::alloc;

pair<string *, string *> StrVec::alloc_n_copy(const string *begin, const string *end)
{
  string *beg = alloc.allocate(end - begin);
  string *my_end = uninitialized_copy(begin, end, beg);
  return {beg, my_end};
}

void StrVec::free()
{
  if (element == nullptr)
    return;
  for (auto p = first_free; p != element;)
  {
    // 销毁string
    alloc.destroy(--p);
  }
  // 释放内存
  alloc.deallocate(element, cap - element);
  element = first_free = cap = nullptr;
}

void StrVec::chk_n_alloc()
{
  if (size() == capacity())
  {
    auto newcapacity = size() ? 2 * size() : 1;
    auto newdata = alloc.allocate(newcapacity);
    auto dest = newdata;
    auto elem = element;
    for (size_t i = 0; i != size(); ++i)
    {
      alloc.construct(dest++, std::move(*elem++));
    }
    free();
    element = newdata;
    first_free = dest;
    cap = element + newcapacity;
  }
}

StrVec::StrVec(const StrVec &s)
{
  auto newdata = alloc_n_copy(s.element, s.first_free);
  element = newdata.first;
  first_free = cap = newdata.second;
}

StrVec::~StrVec()
{
  free();
}

StrVec &StrVec::operator=(const StrVec &s)
{
  auto data = alloc_n_copy(s.element, s.first_free);
  free();
  element = data.first;
  first_free = cap = data.second;
  return *this;
}

void StrVec::push_back(const string &s)
{
  chk_n_alloc();
  alloc.construct(first_free++, s);
}

int main()
{
  StrVec strv;
  strv.push_back("hello");
  strv.push_back("world");
  for (auto it = strv.begin(); it != strv.end(); ++it)
  {
    cout << *it << endl;
  }

  StrVec strv2 = strv;
  *strv2.begin() = "test";
  for (auto it = strv.begin(); it != strv.end(); ++it)
  {
    cout << *it << endl;
  }
  for (auto it = strv2.begin(); it != strv2.end(); ++it)
  {
    cout << *it << endl;
  }
  return 0;
}