#include <iostream>
#include <vector>
#include <list>
using namespace std;
template <typename T, typename T2>
T find(T beg, T end, const T2 &value)
{
  for (; beg != end; ++beg)
  {
    if (*beg == value)
    {
      return beg;
    }
  }
  return end;
}

int main()
{
  vector<int> vv{1, 2, 3, 4, 5};
  list<int> ll{1, 2, 3, 4, 5};
  auto it = find(vv.begin(), vv.end(), 3);
  cout << "the vector find 3 is" << *it << endl;
  auto it_ll = find(ll.begin(), ll.end(), 5);
  cout << "the list find 5 is " << *it_ll << endl;
  return 0;
}