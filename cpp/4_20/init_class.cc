#include <iostream>
using namespace std;

class Point
{
private:
    int _x;
    int _y;
public:
    void print();
};

class Point_d
{
private:
    double _y;
    int _x;
public:
    void print(){

    cout<< "the x is : "<<_x<< "the y is : "<<_y<<endl;
    }
};
void Point::print()
{
    cout<< "the x is : "<<_x<< "the y is : "<<_y<<endl;
}

int main()
{
    Point p;
    p.print();
    cout<<"the int p size is : "<<sizeof(p)<<endl;
    Point_d p_d;
    p_d.print();
    cout<<"the doubt int p size is: "<<sizeof(p_d)<<endl;
    return 0;
}

