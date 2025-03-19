
#include <iostream>
#include <functional>
#include <map>
#include <list>
#include <vector>
#include "op.hpp"
#include "variable.hpp"
// #include <Eigen/Dense>
std::map<size_t, std::vector<size_t>> affectionMap;


template <typename T, typename U>
auto operator+(T&& v, U &&rhs)
{
    return op_binary<AddOperator,T,U>(std::forward<T>(v) , std::forward<U>(rhs));
}

template <typename T, typename U>
auto operator*(T&& v, U &&rhs)
{
    return op_binary<ProductOperator,T,U>(std::forward<T>(v) , std::forward<U>(rhs));
}
int main()
{
    // auto g = ff(std::make_unique<A<G>>(A<G>(G(2))));
    Symbol<int> a;
    Symbol<int> b;
    Symbol<int> c;
 
   
    b =  (a+1) *(a+2);
    c = b +1 + a;
    // //c = a + b;
    a = 5;
    auto gg = c.eval();
    std::vector<Symbol<int>> arr = std::vector<Symbol<int>>(5);
    for (int i = 0; i < 5; i++)
    {
        arr[i] = (arr[i + 1] + 2) + a;
    }
    arr[4] = 1;
    auto kk = arr[0].eval();
    std::cout << kk;
}