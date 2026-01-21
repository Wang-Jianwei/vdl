// Tests for C++11 compatibility layer
#include <catch.hpp>
#include <vdl/core/compat.hpp>
#include <vdl/core/noncopyable.hpp>

#include <memory>
#include <vector>

TEST_CASE("make_unique creates unique_ptr", "[compat][make_unique]") {
    using namespace vdl;
    
    SECTION("basic type") {
        auto ptr = make_unique<int>(42);
        REQUIRE(ptr);
        REQUIRE(*ptr == 42);
    }
    
    SECTION("with multiple arguments") {
        auto ptr = make_unique<std::pair<int, std::string>>(10, "hello");
        REQUIRE(ptr);
        REQUIRE(ptr->first == 10);
        REQUIRE(ptr->second == "hello");
    }
    
    SECTION("vector of unique_ptr") {
        std::vector<std::unique_ptr<int>> vec;
        vec.push_back(make_unique<int>(1));
        vec.push_back(make_unique<int>(2));
        vec.push_back(make_unique<int>(3));
        
        REQUIRE(vec.size() == 3);
        REQUIRE(*vec[0] == 1);
        REQUIRE(*vec[1] == 2);
        REQUIRE(*vec[2] == 3);
    }
}

TEST_CASE("enable_if_t for conditional compilation", "[compat][enable_if_t]") {
    using namespace vdl;
    
    SECTION("type alias works") {
        using result = enable_if_t<true, int>;
        REQUIRE(std::is_same<result, int>::value);
    }
    
    SECTION("conditional type") {
        using is_int = enable_if_t<std::is_same<int, int>::value, int>;
        REQUIRE(std::is_same<is_int, int>::value);
    }
}

TEST_CASE("conditional_t for conditional types", "[compat][conditional_t]") {
    using namespace vdl;
    
    SECTION("select first type") {
        using result = conditional_t<true, int, double>;
        REQUIRE(std::is_same<result, int>::value);
    }
    
    SECTION("select second type") {
        using result = conditional_t<false, int, double>;
        REQUIRE(std::is_same<result, double>::value);
    }
}

TEST_CASE("conjunction for trait AND", "[compat][conjunction]") {
    using namespace vdl;
    
    SECTION("all true") {
        using result = conjunction<std::true_type, std::true_type, std::true_type>;
        REQUIRE(result::value);
    }
    
    SECTION("mixed true and false") {
        using result = conjunction<std::true_type, std::false_type, std::true_type>;
        REQUIRE(!result::value);
    }
    
    SECTION("all false") {
        using result = conjunction<std::false_type, std::false_type>;
        REQUIRE(!result::value);
    }
}

TEST_CASE("disjunction for trait OR", "[compat][disjunction]") {
    using namespace vdl;
    
    SECTION("all true") {
        using result = disjunction<std::true_type, std::true_type>;
        REQUIRE(result::value);
    }
    
    SECTION("mixed true and false") {
        using result = disjunction<std::false_type, std::true_type, std::false_type>;
        REQUIRE(result::value);
    }
    
    SECTION("all false") {
        using result = disjunction<std::false_type, std::false_type>;
        REQUIRE(!result::value);
    }
}

TEST_CASE("void_t for detection idiom", "[compat][void_t]") {
    using namespace vdl;
    
    // Test that void_t can be used
    using result = void_t<int, double, std::string>;
    REQUIRE(std::is_same<result, void>::value);
}

TEST_CASE("NonCopyable prevents copying", "[compat][noncopyable]") {
    using namespace vdl;
    
    class TestClass : public NonCopyable {
    public:
        TestClass() : value(42) {}
        int value;
    };
    
    SECTION("default construction works") {
        TestClass obj;
        REQUIRE(obj.value == 42);
    }
    
    SECTION("copy is deleted") {
        REQUIRE(!std::is_copy_constructible<TestClass>::value);
        REQUIRE(!std::is_copy_assignable<TestClass>::value);
    }
}

TEST_CASE("NonMovable prevents moving", "[compat][nonmovable]") {
    using namespace vdl;
    
    class TestClass : public NonMovable {
    public:
        TestClass() : value(42) {}
        int value;
    };
    
    SECTION("default construction works") {
        TestClass obj;
        REQUIRE(obj.value == 42);
    }
    
    SECTION("copy and move are deleted") {
        REQUIRE(!std::is_copy_constructible<TestClass>::value);
        REQUIRE(!std::is_copy_assignable<TestClass>::value);
        REQUIRE(!std::is_move_constructible<TestClass>::value);
        REQUIRE(!std::is_move_assignable<TestClass>::value);
    }
}

TEST_CASE("remove_cvref_t removes cv and reference", "[compat][remove_cvref_t]") {
    using namespace vdl;
    
    SECTION("remove const reference") {
        using result = remove_cvref_t<const int&>;
        REQUIRE(std::is_same<result, int>::value);
    }
    
    SECTION("remove volatile reference") {
        using result = remove_cvref_t<volatile int&>;
        REQUIRE(std::is_same<result, int>::value);
    }
    
    SECTION("remove rvalue reference") {
        using result = remove_cvref_t<int&&>;
        REQUIRE(std::is_same<result, int>::value);
    }
    
    SECTION("remove all qualifiers") {
        using result = remove_cvref_t<const volatile int&>;
        REQUIRE(std::is_same<result, int>::value);
    }
}

TEST_CASE("bool_constant type alias", "[compat][bool_constant]") {
    using namespace vdl;
    
    SECTION("true constant") {
        using my_true = bool_constant<true>;
        REQUIRE(my_true::value);
    }
    
    SECTION("false constant") {
        using my_false = bool_constant<false>;
        REQUIRE(!my_false::value);
    }
}

// ============================================================================
// Helper templates for detection tests
// ============================================================================

namespace {

template<typename T>
using has_size_t = decltype(std::declval<T>().size());

}

// ============================================================================
// Compatibility Tests
// ============================================================================

TEST_CASE("is_detected detects valid operations", "[compat][detection]") {
    using namespace vdl;
    
    SECTION("detect size method") {
        REQUIRE(is_detected<has_size_t, std::vector<int>>::value);
        REQUIRE(!is_detected<has_size_t, int>::value);
    }
}
