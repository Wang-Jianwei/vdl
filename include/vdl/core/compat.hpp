// C++11 Compatibility Layer
// Provides utilities for C++11 that are available in later standards

#pragma once

#include <memory>
#include <utility>
#include "noncopyable.hpp"

namespace vdl {

// ============================================================================
// C++14 Backports - make_unique
// ============================================================================

// make_unique - Create unique_ptr with perfect forwarding
// Available natively in C++14, backported for C++11
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

// Specialization for array types
template<typename T>
std::unique_ptr<T> make_unique_array(size_t size) {
    return std::unique_ptr<T>(new typename std::remove_extent<T>::type[size]());
}

// ============================================================================
// Type Traits Extensions
// ============================================================================

// enable_if_t - Simplified version of std::enable_if_t (C++14)
// Available natively in C++14, backported for C++11
template<bool B, typename T = void>
using enable_if_t = typename std::enable_if<B, T>::type;

// Conditional type alias (C++14)
template<bool B, typename T, typename F>
using conditional_t = typename std::conditional<B, T, F>::type;

// ============================================================================
// Integral Constants
// ============================================================================

// bool_constant - Integral constant wrapper for bool
template<bool B>
using bool_constant = std::integral_constant<bool, B>;

// true_type - Shorthand for std::integral_constant<bool, true>
using true_type = std::true_type;

// false_type - Shorthand for std::integral_constant<bool, false>
using false_type = std::false_type;

// ============================================================================
// void_t Simulation (C++17)
// ============================================================================

// Useful for SFINAE detection
namespace detail {
template<typename...>
struct voider {
    using type = void;
};
}

template<typename... Args>
using void_t = typename detail::voider<Args...>::type;

// ============================================================================
// Conjunction and Disjunction (C++17 Backport)
// ============================================================================

// Logical AND for type traits
template<typename... Traits>
struct conjunction : std::true_type {};

template<typename T1>
struct conjunction<T1> : T1 {};

template<typename T1, typename... Rest>
struct conjunction<T1, Rest...>
    : conditional_t<T1::value, conjunction<Rest...>, T1> {};

// Logical OR for type traits
template<typename... Traits>
struct disjunction : std::false_type {};

template<typename T1>
struct disjunction<T1> : T1 {};

template<typename T1, typename... Rest>
struct disjunction<T1, Rest...>
    : conditional_t<T1::value, T1, disjunction<Rest...>> {};

// ============================================================================
// Detection Idiom (C++17 Backport)
// ============================================================================

namespace detail {

// Primary template for detection
template<typename Default, typename AlwaysVoid, template<typename...> class Op,
         typename... Args>
struct detector {
    using value_t = std::false_type;
    using type = Default;
};

// Specialization for detected type
template<typename Default, template<typename...> class Op, typename... Args>
struct detector<Default, void_t<Op<Args...>>, Op, Args...> {
    using value_t = std::true_type;
    using type = Op<Args...>;
};

}  // namespace detail

// Detection helper
template<template<typename...> class Op, typename... Args>
using is_detected = typename detail::detector<void, void, Op, Args...>::value_t;

template<template<typename...> class Op, typename... Args>
using detected_t = typename detail::detector<void, void, Op, Args...>::type;

template<typename Default, template<typename...> class Op, typename... Args>
using detected_or = detail::detector<Default, void, Op, Args...>;

template<typename Default, template<typename...> class Op, typename... Args>
using detected_or_t = typename detected_or<Default, Op, Args...>::type;

// ============================================================================
// Remove CVRef (C++20 feature backport)
// ============================================================================

namespace detail {

template<typename T>
struct remove_cvref {
    using type = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
};

}

template<typename T>
using remove_cvref_t = typename detail::remove_cvref<T>::type;

}  // namespace vdl
