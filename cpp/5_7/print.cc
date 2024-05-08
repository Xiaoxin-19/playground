#include <iostream>
#include <string>
#include <vector>

using namespace std;

template <typename T>
void print(const T &coll)
{
  for (auto beg = begin(coll); beg != end(coll); ++beg)
  {
    cout << *beg << endl;
  }
}
int main()
{
  int a[3] = {1, 3, 4};
  string str[5] = {"hello", "world", "1", "2", "3"};
  print(a);
  print(str);
  
  return 0;
}