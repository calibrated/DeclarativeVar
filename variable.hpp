#ifndef VARIABLE_H
#define VARIABLE_H
#include <set>
#include <vector>

#include "op.hpp"
class SymbolBase;


template <typename T, bool Cache=true>
class Symbol;

std::map<SymbolBase*,std::set<SymbolBase*>> affectionMap;

template <typename T>
class VariableBase
{
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
    T eval() { self()->eval(); }
    Derived &self() { return static_cast<Derived &>(*this); }
};

class SymbolBase
{
    public:
    virtual void SetDirty() =0;
    virtual ~SymbolBase() = default;
};
template < typename T>
class Array;

template <typename T>
class Accessor
{
    Array<T>* arr;
    int n;
public:
    Accessor(Array<T>* arr_, int n_):arr(arr_),n(n_)
    {
    }

    template <typename U>
    auto operator = (U&& v) {
        arr->Set(n, OperandTrait<U>::value(v));
    }
};

template <typename T>
class Array : public Variable<T, Array<T>>
{ 
    std::vector<std::unique_ptr<VariableBase<T>>> arr;
public:
    Array()
    {
        //arr.size(n);
    }
    Array(int n)
    {
        arr.size(n);
    }
    Accessor<T> operator [](int n)
    {
        return Accessor<T>(this, n);
    }
    template <typename U>
    auto Set(int n, U&& t)
    {
        //arr[n] =  OperandTrait<U>::value(t);
    }
};
#include <functional>
template <typename T>
class Symbol<T,true> : public Variable<T, Symbol<T>>, public SymbolBase
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
    auto operator=(std::unique_ptr<D> g)
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
class Symbol<T,false> : public Variable<T, Symbol<T>>, public SymbolBase
{
public:
    Symbol()
    {
    }
    Symbol(Symbol&& other) = delete;
    Symbol(Symbol& other) = delete;

    T eval()
    {
        return compute();
    }

    std::function<T()> compute;

    template <typename D>
    auto operator=(std::unique_ptr<D> g)
    {
        auto ss = std::make_shared<D>(std::move(*g));
        compute = [ss]() -> T
        { return ss->eval(); };
    }
    
    auto operator=(T val_)
    {
        compute = [val_]() -> T
        { return val_; };
    }
};

template <typename T>
void callUsedSymbols(T&& obj, std::vector<SymbolBase*>& list) {
    if constexpr (is_unique_ptr<T>::value) {
        obj->usedSymbols(list);
    } else {
        if constexpr (!std::is_arithmetic_v<std::decay_t<T>>)
        {
           obj.usedSymbols(list);
        }            
    }
}
template <typename T, typename D1, typename D2>
class AddOperator : public Variable<T, AddOperator<T, D1, D2>>
{
    D1 lhs;
    D2 rhs;

public:
    template <typename U1, typename U2>
    AddOperator(U1 &&l, U2 &&r) : lhs(std::forward<U1>(l)),
                                  rhs(std::forward<U2>(r))
    {
    }
    // Implement the evaluate function
    T eval()
    {
       return OperandTrait<D1>::eval(lhs) + OperandTrait<D2>::eval(rhs);
    }
    
    void usedSymbols(std::vector<SymbolBase*>& list)
    {
        callUsedSymbols(lhs,list);
        callUsedSymbols(rhs,list);
    }
};

template <typename T, typename D1, typename D2>
class ProductOperator : public Variable<T, ProductOperator<T, D1, D2>>
{
    D1 lhs;
    D2 rhs;

public:
    template <typename U1, typename U2>
    ProductOperator(U1 &&l, U2 &&r) : lhs(std::forward<U1>(l)),
                                      rhs(std::forward<U2>(r))
    {
    }
    // Implement the evaluate function
    T eval()
    {
        return OperandTrait<D1>::eval(lhs) * OperandTrait<D2>::eval(rhs);
    }
    void usedSymbols(std::vector<SymbolBase*>& list)
    {
        callUsedSymbols(lhs,list);
        callUsedSymbols(rhs,list);
    }
};

template <typename T, typename D1, typename D2>
class MinusOperator : public Variable<T, MinusOperator<T, D1, D2>>
{
    D1 lhs;
    D2 rhs;

public:
    template <typename U1, typename U2>
    MinusOperator(U1 &&l, U2 &&r) : lhs(std::forward<U1>(l)),
                                  rhs(std::forward<U2>(r))
    {
    }
    // Implement the evaluate function
    T eval()
    {
       return OperandTrait<D1>::eval(lhs) - OperandTrait<D2>::eval(rhs);
    }
    
    void usedSymbols(std::vector<SymbolBase*>& list)
    {
        callUsedSymbols(lhs,list);
        callUsedSymbols(rhs,list);
    }
};
#endif