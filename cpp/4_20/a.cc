#include <iostream>
using std::cin;
using std::cout;
using std::endl;

namespace MySpace
{
    int add(int a  , int b = 0, int c = 0);
    int add(int a  , int b  );
    double add(int a = 0, double b = 0.0);
}
int MySpace:: add(int a, int b, int c)
{
    cout<<"3 int sum "<<endl;
    return a + b + c;
}
int MySpace:: add(int a, int b)
{
    cout<<"2 int sum"<<endl;
    return a + b ;
}
double MySpace:: add(int a , double b )
{
    cout<<"int + double"<<endl;
    return a + b;
}


