#ifndef TRAIT_H
#define TRAIT_H
#include <type_traits>
#include <memory>
#include <ranges>
template <typename T>
class Number;

template <typename U, typename = void>
struct is_variable : std::false_type
{
};
template <typename U>
struct is_variable<U, std::void_t<typename std::decay_t<U>::variable_type>> : std::true_type
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

template <typename D, typename = void>
struct is_shared_ptr : std::false_type
{
};
template <typename D>
struct is_shared_ptr<D, std::void_t<typename std::decay_t<D>::element_type>> : std::is_same<std::decay_t<D>, std::shared_ptr<typename std::decay_t<D>::element_type>>
{
};
template <typename T>
struct is_iterable_range : std::bool_constant<std::ranges::range<T>> {};

template <typename T, typename Enable = void>
struct ContainerTrait;
template <typename T, typename Enable = void>
struct ElementTrait;


template <typename T>
struct ContainerTrait<T, std::enable_if_t<is_shared_ptr<T>::value && is_variable<typename T::element_type>::value>>
{
    using type = T;  // 그대로 유지
    static constexpr type value(T x) { return x; } // 그대로 반환
};
template <typename T>
struct ContainerTrait<T, std::enable_if_t<!is_shared_ptr<T>::value && !is_variable<T>::value && !is_iterable_range<T>::value>>
{
    using type = std::shared_ptr<Number<T>>;  // 그대로 유지
    static constexpr type value(T x) { return std::make_shared<Number<T>>(x); } // 그대로 반환
};
template <typename T>
struct ContainerTrait<T, std::enable_if_t<is_variable<T>::value&& std::is_reference_v<T>>>
{
    using type = T;  // 그대로 유지
    static constexpr type value(T x) { return x; } // 그대로 반환
};
template <typename T>
struct ContainerTrait<T, std::enable_if_t<is_variable<T>::value&& !std::is_reference_v<T>>>
{
    using type = std::shared_ptr<T>;  //shared_ptr로 힙할당하고 수명관리
    static constexpr type value(T x) { return std::make_shared<T>(x); } // 
};

template <typename T>
struct ContainerTrait<T, std::enable_if_t<is_iterable_range<T>::value,void>>
{
    using type = T;  //shared_ptr로 힙할당하고 수명관리
    static constexpr type value(T x) { return x; } // 
};

template <typename T>
struct ElementTrait<T, std::enable_if_t<is_iterable_range<T>::value,void>>
{
    using type = typename std::ranges::range_value_t<T>;
    static constexpr auto eval(T list) {
        return std::views::transform(list, [](auto const& a) {
            
        });
    }
};

template <typename T>
struct ElementTrait<T, std::enable_if_t<is_shared_ptr<T>::value &&  is_variable<typename T::element_type>::value>>
{
    using type= typename std::decay_t<T>::element_type::variable_type;
    static constexpr type eval(T&& x) {return x->eval(); }
};

//a+a
template <typename T>
struct ElementTrait<T, std::enable_if_t<is_variable<T>::value>>
{
    using type= typename std::decay_t<T>::variable_type;
    static constexpr type eval(T x) {return x.eval(); } 
};
template <typename T>
struct ElementTrait<T, std::enable_if_t<!is_shared_ptr<T>::value && !is_variable<T>::value && !is_iterable_range<T>::value>>
{
    using type= T;
    static constexpr type eval(T x) {return x; } 
};

#endif
