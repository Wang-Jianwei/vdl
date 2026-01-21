/**
 * @file test_compat.cpp
 * @brief 测试 C++11 兼容层
 */

#include <catch.hpp>
#include <vdl/core/compat.hpp>
#include <vdl/core/noncopyable.hpp>

#include <memory>
#include <string>
#include <vector>

// ============================================================================
// make_unique 测试
// ============================================================================

TEST_CASE("make_unique creates unique_ptr", "[core][compat]") {
    SECTION("simple type") {
        auto ptr = vdl::make_unique<int>(42);
        REQUIRE(ptr != nullptr);
        REQUIRE(*ptr == 42);
    }

    SECTION("string type") {
        auto ptr = vdl::make_unique<std::string>("hello");
        REQUIRE(ptr != nullptr);
        REQUIRE(*ptr == "hello");
    }

    SECTION("struct with constructor") {
        struct point_t {
            int x, y;
            point_t(int a, int b) : x(a), y(b) {}
        };

        auto ptr = vdl::make_unique<point_t>(10, 20);
        REQUIRE(ptr != nullptr);
        REQUIRE(ptr->x == 10);
        REQUIRE(ptr->y == 20);
    }

    SECTION("with multiple arguments") {
        auto ptr = vdl::make_unique<std::pair<int, std::string>>(10, "hello");
        REQUIRE(ptr != nullptr);
        REQUIRE(ptr->first == 10);
        REQUIRE(ptr->second == "hello");
    }
    
    SECTION("vector of unique_ptr") {
        std::vector<std::unique_ptr<int>> vec;
        vec.push_back(vdl::make_unique<int>(1));
        vec.push_back(vdl::make_unique<int>(2));
        vec.push_back(vdl::make_unique<int>(3));
        
        REQUIRE(vec.size() == 3);
        REQUIRE(*vec[0] == 1);
        REQUIRE(*vec[1] == 2);
        REQUIRE(*vec[2] == 3);
    }
}

// ============================================================================
// type_traits 测试
// ============================================================================

TEST_CASE("enable_if_t works correctly", "[core][compat]") {
    using enable_for_int = vdl::enable_if_t<std::is_integral<int>::value, int>;
    static_assert(std::is_same<enable_for_int, int>::value, "Should be int");
    REQUIRE(true);
}

TEST_CASE("conditional_t works correctly", "[core][compat]") {
    using type1 = vdl::conditional_t<true, int, double>;
    using type2 = vdl::conditional_t<false, int, double>;
    
    static_assert(std::is_same<type1, int>::value, "Should be int");
    static_assert(std::is_same<type2, double>::value, "Should be double");
    REQUIRE(true);
}

TEST_CASE("remove_cvref_t removes cv and ref qualifiers", "[core][compat]") {
    SECTION("const reference") {
        using base = vdl::remove_cvref_t<const int&>;
        static_assert(std::is_same<base, int>::value, "Should be int");
        REQUIRE(true);
    }

    SECTION("volatile rvalue reference") {
        using base2 = vdl::remove_cvref_t<volatile double&&>;
        static_assert(std::is_same<base2, double>::value, "Should be double");
        REQUIRE(true);
    }

    SECTION("all qualifiers") {
        using base3 = vdl::remove_cvref_t<const volatile int&>;
        static_assert(std::is_same<base3, int>::value, "Should be int");
        REQUIRE(true);
    }
}

// ============================================================================
// void_t 测试
// ============================================================================

template <typename T, typename = void>
struct has_size : std::false_type {};

template <typename T>
struct has_size<T, vdl::void_t<decltype(std::declval<T>().size())>> 
    : std::true_type {};

TEST_CASE("void_t enables SFINAE", "[core][compat]") {
    static_assert(has_size<std::string>::value, "string has size()");
    static_assert(!has_size<int>::value, "int has no size()");
    REQUIRE(true);
}

// ============================================================================
// conjunction/disjunction 测试
// ============================================================================

TEST_CASE("conjunction works correctly", "[core][compat]") {
    constexpr bool all_true = vdl::conjunction<
        std::is_integral<int>,
        std::is_integral<long>,
        std::is_integral<short>
    >::value;
    
    constexpr bool not_all = vdl::conjunction<
        std::is_integral<int>,
        std::is_floating_point<int>
    >::value;
    
    REQUIRE(all_true == true);
    REQUIRE(not_all == false);
}

TEST_CASE("disjunction works correctly", "[core][compat]") {
    constexpr bool any_true = vdl::disjunction<
        std::is_integral<double>,
        std::is_floating_point<double>
    >::value;
    
    constexpr bool none_true = vdl::disjunction<
        std::is_integral<double>,
        std::is_pointer<double>
    >::value;
    
    REQUIRE(any_true == true);
    REQUIRE(none_true == false);
}

TEST_CASE("negation works correctly", "[core][compat]") {
    constexpr bool not_int = vdl::negation<std::is_integral<double>>::value;
    constexpr bool is_int = vdl::negation<std::is_integral<int>>::value;
    
    REQUIRE(not_int == true);
    REQUIRE(is_int == false);
}

// ============================================================================
// noncopyable_t 测试
// ============================================================================

TEST_CASE("noncopyable_t prevents copying", "[core][compat]") {
    class test_class_t : public vdl::noncopyable_t {
    public:
        test_class_t() : value(42) {}
        int value;
    };

    SECTION("default construction works") {
        test_class_t obj;
        REQUIRE(obj.value == 42);
    }

    SECTION("copy is deleted") {
        REQUIRE(!std::is_copy_constructible<test_class_t>::value);
        REQUIRE(!std::is_copy_assignable<test_class_t>::value);
    }

    SECTION("move still works") {
        REQUIRE(std::is_move_constructible<test_class_t>::value);
        REQUIRE(std::is_move_assignable<test_class_t>::value);
    }
}

TEST_CASE("nonmovable_t prevents copying and moving", "[core][compat]") {
    class test_class_t : public vdl::nonmovable_t {
    public:
        test_class_t() : value(42) {}
        int value;
    };

    SECTION("default construction works") {
        test_class_t obj;
        REQUIRE(obj.value == 42);
    }

    SECTION("copy and move are deleted") {
        REQUIRE(!std::is_copy_constructible<test_class_t>::value);
        REQUIRE(!std::is_copy_assignable<test_class_t>::value);
        REQUIRE(!std::is_move_constructible<test_class_t>::value);
        REQUIRE(!std::is_move_assignable<test_class_t>::value);
    }
}
