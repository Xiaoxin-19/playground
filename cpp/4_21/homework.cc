#include <iostream>
using std::cin;
using std::cout;
using std::endl;
using std::string;
class A
{
private:
    string msg;
public:
    A(string msg):msg(msg){

    }

    ~A(){
        cout<<msg<<endl;
    }
};


A g_s_a("gobal static");
A g_a("global");
A *g_h_a = new A("gobal heap");

int main()
{
    
    A p_s_a("part static");
    A p_a("part");
    A *ip_h_a = new A("part heap");

    delete ip_h_a;
    delete g_h_a;
    return 0;
}

