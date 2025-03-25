
#include <iostream>
#include <map>
#include "op.hpp"
#include "variable.hpp"


template <typename T, typename U>
auto operator+(T&& v, U &&rhs)
{
    return op_binary<AddOperator,T,U>(std::forward<T>(v) , std::forward<U>(rhs));
}
template <typename T, typename U>
auto operator-(T&& v, U &&rhs)
{
    return op_binary<MinusOperator,T,U>(std::forward<T>(v) , std::forward<U>(rhs));
}
template <typename T, typename U>
auto operator*(T&& v, U &&rhs)
{
    return op_binary<ProductOperator,T,U>(std::forward<T>(v) , std::forward<U>(rhs));
}

int main()
{
    Symbol<Array<int>,true> list;
    Symbol<int> a;
    Symbol<int> c;
    Array<int> bb(5);
    bb[0] = 1;
    /*bb[1] = a + 1;
    bb[2] = a + 1;*/
    /*list = [&a]()->Array<int> {
      
        return bb;
    };*/

    
 
    c = (a + 1) * a;
    a= 2;
    std::cout <<  c.eval();
    a = 3;
    std::cout <<  c.eval();

    std::vector<Symbol<int>> arr = std::vector<Symbol<int>>(5);
    
    for (int i = 0; i < 5; i++)
    {
        arr[i] = (arr[i + 1] + 2) + a;
    }
    arr[4] = 1;
    auto kk = arr[0].eval();
    std::cout << kk;
}