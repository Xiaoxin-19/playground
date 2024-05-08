#include <iostream>
using namespace std;
int main()
{
    const int a = 100;
    int & ptr1 = const_cast<int &> (a);
    ptr1 = 101;
    cout<<"ptr: "<<ptr1<<endl;
    return 0;
}

