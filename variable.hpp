#ifndef VARIABLE_H
#define VARIABLE_H
#include <set>
#include <vector>
#include <map>
#include "trait.hpp"
#include <functional>
class SymbolBase;


template <typename T>
class Symbol;

std::map<SymbolBase*,std::set<SymbolBase*>> affectionMap;

template <typename T>
class VariableBase
{
    public:
    virtual T eval() = 0;
};
template <typename T, typename Derived>
class Variable  : public VariableBase<T>
{
public:
    using variable_type = T; // T 타입을 알기 위해 추가
    Variable()
    {
    }
    Variable(Derived & derived) = delete;
    Variable(Derived && derived) = delete;
    T eval() override { return self().eval(); }
    Derived &self() { return static_cast<Derived &>(*this); }
};
template <typename T>
class Number  : public Variable<T, Number<T>>
{
    T val;
    public:
    template <typename U>
        Number(U v):val(std::forward<U>(v))
        {}
        void usedSymbols(std::vector<SymbolBase*>& list)
        {

        }
        T eval()
        {
            return val;
        }
};

class SymbolBase
{
    public:
    virtual void SetDirty() =0;
    virtual ~SymbolBase() = default;
};

template <typename T>
class Symbol : public Variable<T, Symbol<T>>, public SymbolBase
{
public:
    std::function<T()> compute;
    std::vector<SymbolBase*> affectingSymbols;
    bool computed = false;
    T val;
    Symbol()
    {
        affectionMap[this] = std::set<SymbolBase*>();
    }
    Symbol(Symbol&& other) = delete;
    Symbol(Symbol& other) = delete;
    
    void SetDirty(){
        computed = false;
        for(auto& affected: affectionMap[this])
        {
            affected->SetDirty();
        }
    }
    T eval()
    {
        if (computed)
            return val;
        else
        {
            computed = true;
            val = compute();
            return val;
        }
    }

   
    
    void usedSymbols(std::vector<SymbolBase*>& list)
    {
        list.push_back(this);
    }
    template <typename D>
    auto operator=(std::shared_ptr<D> g)
    {
        computed = false;
        affectingSymbols.clear();
        (*g).usedSymbols(affectingSymbols);
        for( auto& v : affectingSymbols)
        {
            affectionMap[v].insert(this);
        }
        auto ss = std::make_shared<D>(std::move(*g));
        compute = [ss]() -> T
        { return ss->eval(); };
    }
    
    auto operator=(T val_)
    {
        val = val_;
        affectingSymbols.clear();
        for(auto& affected: affectionMap[this])
        {
            affected->SetDirty();
        }
        computed = true;
    }
    Symbol<T>& operator = (std::function<T()> ff)
    {
        return *this;
    }
};

template <typename T>
void callUsedSymbols(T&& obj, std::vector<SymbolBase*>& list) {
    if constexpr (is_shared_ptr<T>::value) {
        obj->usedSymbols(list);
    } else {
        if constexpr (!std::is_arithmetic_v<std::decay_t<T>>)
        {
           obj.usedSymbols(list);
        }            
    }
}

template <typename T, typename D1, typename D2, typename Derived>
class BinaryOp : public Variable<T, Derived>
{
    using ltype= typename ContainerTrait<D1>::type;
    using rtype= typename ContainerTrait<D2>::type;
public:
    ltype lhs;
    rtype rhs;
    template <typename U1, typename U2>
    BinaryOp(U1 &&l, U2 &&r) : lhs( ContainerTrait<D1>::value(std::forward<U1>(l))),
                               rhs( ContainerTrait<D2>::value(std::forward<U2>(r)))
    {
    }
    void usedSymbols(std::vector<SymbolBase*>& list)
    {
        callUsedSymbols(lhs,list);
        callUsedSymbols(rhs,list);
    }
};

template <typename T, typename D1, typename D2>
class AddOperator : public BinaryOp<T, D1, D2, AddOperator<T,D1,D2>>
{
    using ltype= typename ContainerTrait<D1>::type;
    using rtype= typename ContainerTrait<D2>::type;
public:
    template <typename U1, typename U2>
    AddOperator(U1 &&l, U2 &&r) : BinaryOp<T, D1, D2, AddOperator<T,D1,D2>>(std::forward<U1>(l),std::forward<U2>(r))
    {
    }
    // Implement the evaluate function
    T eval()
    {
       return ElementTrait<ltype>::eval(std::forward<ltype>(this->lhs)) 
       + ElementTrait<rtype>::eval(std::forward<rtype>(this->rhs));
    } 
};

template <typename T, typename D1, typename D2>
class ProductOperator : public BinaryOp<T, D1, D2, ProductOperator<T,D1,D2>>
{
    using ltype= typename ContainerTrait<D1>::type;
    using rtype= typename ContainerTrait<D2>::type;
public:
    template <typename U1, typename U2>
    ProductOperator(U1 &&l, U2 &&r) : BinaryOp<T, D1, D2, ProductOperator<T,D1,D2>>(std::forward<U1>(l),std::forward<U2>(r))
    {
    }
    // Implement the evaluate function
    T eval()
    {
        return ElementTrait<typename ContainerTrait<D1>::type>::eval(std::forward<D1>(this->lhs))
         * ElementTrait<typename ContainerTrait<D2>::type>::eval(std::forward<D2>(this->rhs));
    }
};

template <typename T, typename D1, typename D2>
class MinusOperator : public BinaryOp<T, D1, D2, MinusOperator<T, D1, D2>>
{
    using ltype= typename ContainerTrait<D1>::type;
    using rtype= typename ContainerTrait<D2>::type;
 public:
    template <typename U1, typename U2>
    MinusOperator(U1 &&l, U2 &&r) : BinaryOp<T, D1, D2, ProductOperator<T,D1,D2>>(std::forward<U1>(l),std::forward<U2>(r))
    {
    }
    T eval()
    {
        return ElementTrait<typename ContainerTrait<D1>::type>::eval(std::forward<D1>(this->lhs))
        - ElementTrait<typename ContainerTrait<D2>::type>::eval(std::forward<D2>(this->rhs));
    }
};

template <typename T>
using Transformed = typename ElementTrait<T>::type;
//using Transformed = typename ElementTrait<typename ContainerTrait<T>::type>::type;

template <auto Func, typename... Args>
class FunctionOperator : public Variable<
std::invoke_result_t<decltype(Func), Transformed<Args>...>,
FunctionOperator<Func, Args...>> 
{
    std::tuple<Args...> stored_args;
    static constexpr auto lambda = Func; 
    public:
    FunctionOperator(Args... args):stored_args(ContainerTrait<decltype(args)>::value(std::forward<decltype(args)>(args))...)
    {
        
    }
    std::invoke_result_t<decltype(Func), Transformed<Args>...> eval(){
        return std::apply([](auto&&... values) {
           return lambda(ElementTrait<typename ContainerTrait<decltype(values)>::type>::eval(std::forward<decltype(values)>(values))...);
        }, stored_args);
    }
    void usedSymbols(std::vector<SymbolBase*>& list)
    {
        return std::apply([&list](auto&&... values) {
            (callUsedSymbols(values,list),...);
         }, stored_args);
    }
};


template <template <typename...> class OP, typename T, typename U, typename opsymbol>
auto op_binary(T &&lhs, U &&rhs,opsymbol ops)
{
   using LT = typename ContainerTrait<T>::type;
   using RT = typename ContainerTrait<U>::type;
   using RET = decltype( ops(std::declval<typename ElementTrait<T>::type>(), std::declval<typename ElementTrait<U>::type>()));
   return std::make_shared<OP<RET, LT, RT>>(OP<RET, LT, RT>(ContainerTrait<T>::value(std::forward<T>(lhs)), 
   ContainerTrait<U>::value(std::forward<U>(rhs))));
}

template <typename T, typename U,typename = std::enable_if_t<is_variable<T>::value ||is_variable<U>::value  >>
auto operator+(T&& v, U &&rhs)
{
    return op_binary<AddOperator,T,U>(std::forward<T>(v) , std::forward<U>(rhs), std::plus<>());
}
template <typename T, typename U,typename = std::enable_if_t<is_variable<T>::value ||is_variable<U>::value  >>
auto operator-(T&& v, U &&rhs)
{
    return op_binary<MinusOperator,T,U>(std::forward<T>(v) , std::forward<U>(rhs),std::minus<>());
}
template <typename T, typename U,typename = std::enable_if_t<is_variable<T>::value ||is_variable<U>::value  >>
auto operator*(T&& v, U &&rhs)
{
    return op_binary<ProductOperator,T,U>(std::forward<T>(v) , std::forward<U>(rhs),std::multiplies<>());
}

template <auto Func, typename... Args>
auto lambda(Args &&...args)
{
    return std::make_shared<FunctionOperator<Func, Args...>>(std::forward<Args>(args)...);
}
#endif