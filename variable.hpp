#ifndef VARIABLE_H
#define VARIABLE_H
template <typename T, typename Derived>
class Variable
{
public:
    using value_type = T; // T 타입을 알기 위해 추가
    Variable()
    {
    }
    Variable(Derived & derived) = delete;
    Variable(Derived && derived) = delete;
    T eval() { self()->eval(); }
    Derived &self() { return static_cast<Derived &>(*this); }
};

template <typename T>
class Symbol : public Variable<T, Symbol<T>>
{

public:
    T val;
    bool computed = false;
    Symbol()
    {
    }
    Symbol(Symbol&& other) = delete;
    Symbol(Symbol& other) = delete;
    T eval()
    {
        if (computed)
            return val;
        else
            return compute();
    }

    std::function<T()> compute;

    template <typename D>
    auto operator=(std::unique_ptr<D> g)
    {
        computed = false;
        auto ss = std::make_shared<D>(std::move(*g));
        compute = [ss]() -> T
        { return ss->eval(); };
    }
    auto operator=(T val_)
    {
        val = val_;
        computed = true;
    }
};

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
       return OperandTrait<D1>::eval(std::forward<D1>(lhs)) + OperandTrait<D2>::eval(std::forward<D2>(rhs));
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
        if constexpr (is_unique_ptr<D1>::value)
        {
            if constexpr (is_unique_ptr<D2>::value)
            {
                return lhs->eval() * rhs->eval();
            }
            else
            {
                return lhs->eval() * rhs.eval();
            }
        }
        else
        {
            if constexpr (is_unique_ptr<D2>::value)
            {
                return lhs.eval() * rhs->eval();
            }
            else
            {
                return lhs.eval() * rhs.eval();
            }
        }
    }
};

template <typename T>
class Number : public Variable<T, Number<T>>
{
    T val;

public:
    Number(T &va) : val(va)
    {
    }
    Number(T &&va) : val(va)
    {
    }
    T eval() { return val; }
};


#endif