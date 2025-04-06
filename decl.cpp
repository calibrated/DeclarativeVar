
#include <iostream>
#include <map>
#include "op.hpp"
#include "variable.hpp"

int add(int a, int b)
{
    return a + b;
}
double add(double a, double b)
{
    return a + b;
}


int main()
{
    Symbol<int> a;
    Symbol<int> c;
    auto d = lambda<[](int a, int b)
        {
            return add(a, b);
        }>(a, c);
    a = 1;
    c = 2;
    Symbol<int> ggg;
    ggg = d;
}