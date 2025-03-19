#ifndef OP_H
#define OP_H
#include <type_traits>


template <typename U, typename = void>
struct has_value : std::false_type
{
};
template <typename U>
struct has_value<U, std::void_t<typename std::decay_t<U>::value_type>> : std::true_type
{
};
template <typename D, typename = void>
struct is_unique_ptr : std::false_type
{
};
template <typename D>
struct is_unique_ptr<D, std::void_t<typename std::decay_t<D>::element_type>> : std::is_same<std::decay_t<D>, std::unique_ptr<typename std::decay_t<D>::element_type>>
{
};

template <typename T, typename Enable = void>
struct OperandTrait;
template <typename T>
struct OperandTrait<T, std::enable_if_t<std::is_lvalue_reference_v<T> && is_unique_ptr<T>::value && has_value<typename T::element_type>::value>>
{
    using container_type = T;  // 그대로 유지
    using element_type=  typename std::decay_t<T>::element_type::value_type;
    static constexpr T value(T x) { return std::move(x); } // 그대로 반환
    static constexpr element_type eval(T x) {return x->eval(); }
};

template <typename T>
struct OperandTrait<T, std::enable_if_t<!std::is_reference_v<T> && is_unique_ptr<T>::value && has_value<typename T::element_type>::value>>
{
    using container_type = T;  // 그대로 유지
    using element_type=  typename std::decay_t<T>::element_type::value_type;
    static constexpr T value(T x) { return std::move(x); } // 그대로 반환
    static constexpr element_type eval(T& x) {return x->eval(); }
};

template <typename T>
struct OperandTrait<T, std::enable_if_t<std::is_rvalue_reference_v<T> && is_unique_ptr<T>::value &&  has_value<typename T::element_type>::value>>
{
    using container_type = T;  // 그대로 유지
    using element_type=  typename std::decay_t<T>::element_type::value_type;
    static constexpr T value(T x) { return std::move(x); } // std::move()로 반환
    static constexpr element_type eval(T x) {return x->eval(); }
};

template <typename T>
struct OperandTrait<T, std::enable_if_t<std::is_lvalue_reference_v<T> && !is_unique_ptr<T>::value && has_value<T>::value>>
{
    using container_type = T;  // 그대로 유지
    using element_type=  std::decay_t<T>::value_type;
    static constexpr T value(T x) { return x; }
    static constexpr element_type eval(T x) {return x.eval(); }
};
template <typename T>
struct OperandTrait<T, std::enable_if_t<std::is_rvalue_reference_v<T> && !is_unique_ptr<T>::value && !has_value<T>::value>>
{
    using container_type = std::unique_ptr<std::decay_t<T>>;  // 그대로 유지
    using element_type=  std::decay_t<T>;
    static constexpr auto value(T x) { return std::make_unique<element_type>(x); }
    static constexpr element_type eval(T x) {return x.eval(); }
    //static constexpr std::unique_ptr<T> value(T x) { return std::make_unique<T>(x); } // std::move()로 반환
};

template <typename T>
struct OperandTrait<T, std::enable_if_t<!std::is_reference_v<T> && std::is_arithmetic_v<T>>>
{
    using container_type = T;  // 
    using element_type=  T;
    static constexpr container_type value(T x) { return x; } 
    static constexpr element_type eval(T x) {return x; }
};

template <template <typename...> class OP, typename T, typename U>
auto op_binary(T &&lhs, U &&rhs)
{
   using LT = OperandTrait<T>::container_type;
   using RT = OperandTrait<U>::container_type;
   using RET = decltype( std::declval<typename OperandTrait<T>::element_type>()+ std::declval<typename OperandTrait<U>::element_type>());
   return std::make_unique<OP<RET, LT, RT>>(OP<RET, LT, RT>(OperandTrait<T>::value(std::forward<T>(lhs)), 
   OperandTrait<U>::value(std::forward<U>(rhs))));
 
}


#endif
