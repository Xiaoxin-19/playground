#include <iostream>
using std::cin;
using std::cout;
using std::endl;

namespace MySpace 
{

    extern  int add(int a , int b = 0, int c = 0);
    extern  int add(int a , int  b);
    double add(int a = 0 , double b = 0.0);
}

int main()
{
    using MySpace::add;
    add();
    int a = 1, b = 2;
    double c = 1.25;
    add(a,b,c);
    add(a,c);
    return 0;
}

