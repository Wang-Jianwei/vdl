/**
 * @file compat.hpp
 * @brief C++11 兼容层
 * 
 * 提供 C++14/17/20 特性的 C++11 兼容实现。
 */

#ifndef VDL_CORE_COMPAT_HPP
#define VDL_CORE_COMPAT_HPP

#include <memory>
#include <utility>
#include <type_traits>

namespace vdl {

// ============================================================================
// C++14 Backports - make_unique
// ============================================================================

/**
 * @brief 创建 unique_ptr (C++14 backport)
 */
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

/**
 * @brief 创建数组类型的 unique_ptr
 */
template<typename T>
std::unique_ptr<T> make_unique_array(std::size_t size) {
    return std::unique_ptr<T>(new typename std::remove_extent<T>::type[size]());
}

// ============================================================================
// Type Traits Extensions (C++14 Backport)
// ============================================================================

template<bool B, typename T = void>
using enable_if_t = typename std::enable_if<B, T>::type;

template<bool B, typename T, typename F>
using conditional_t = typename std::conditional<B, T, F>::type;

template<typename T>
using remove_const_t = typename std::remove_const<T>::type;

template<typename T>
using remove_reference_t = typename std::remove_reference<T>::type;

template<typename T>
using decay_t = typename std::decay<T>::type;

// ============================================================================
// Integral Constants
// ============================================================================

template<bool B>
using bool_constant = std::integral_constant<bool, B>;

// ============================================================================
// void_t (C++17 Backport)
// ============================================================================

namespace detail {
template<typename...>
struct voider {
    typedef void type;
};
}  // namespace detail

template<typename... Args>
using void_t = typename detail::voider<Args...>::type;

// ============================================================================
// Conjunction and Disjunction (C++17 Backport)
// ============================================================================

template<typename... Traits>
struct conjunction : std::true_type {};

template<typename T1>
struct conjunction<T1> : T1 {};

template<typename T1, typename... Rest>
struct conjunction<T1, Rest...>
    : conditional_t<T1::value, conjunction<Rest...>, T1> {};

template<typename... Traits>
struct disjunction : std::false_type {};

template<typename T1>
struct disjunction<T1> : T1 {};

template<typename T1, typename... Rest>
struct disjunction<T1, Rest...>
    : conditional_t<T1::value, T1, disjunction<Rest...>> {};

// negation (C++17 Backport)
template<typename B>
struct negation : bool_constant<!B::value> {};

// ============================================================================
// remove_cvref_t (C++20 Backport)
// ============================================================================

namespace detail {
template<typename T>
struct remove_cvref {
    typedef typename std::remove_cv<
        typename std::remove_reference<T>::type
    >::type type;
};
}  // namespace detail

template<typename T>
using remove_cvref_t = typename detail::remove_cvref<T>::type;

}  // namespace vdl

#endif  // VDL_CORE_COMPAT_HPP
