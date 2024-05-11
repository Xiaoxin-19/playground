#include <iostream>
#include <vector>
#include <array>
#include <iterator>
using std::array;
using std::begin;
using std::cout;
using std::end;
using std::endl;
using std::vector;

void display(vector<int> &v)
{
   for (auto item : v)
   {
      cout << item << " ";
   }
   cout << endl;
}

int main()
{
   using myv = vector<int>;
   myv v1;
   display(v1);
   myv v2(10, 3);
   display(v2);
   myv v3{1, 2, 3, 4, 5};
   display(v3);
   int arr[10]{12};
   array<int, 10> a1{1, 2, 3, 4, 5};
   myv v4(v3.begin(), v3.end());
   display(v4);
   myv v5(begin(arr), end(arr));
   display(v5);
   myv v6(a1.begin(), a1.end());
   display(v6);
   myv v7(v6);
   display(v7);
   myv v8(myv{99, 100, 100});
   display(v8);
}