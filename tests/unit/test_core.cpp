// Unit tests for core module
#include <catch.hpp>
#include <tl/expected.hpp>
#include <tl/optional.hpp>

TEST_CASE("tl::expected basic operations", "[core][expected]") {
    using tl::expected;
    using tl::make_unexpected;
    
    SECTION("successful result") {
        expected<int, std::string> result = 42;
        REQUIRE(result);
        REQUIRE(*result == 42);
    }
    
    SECTION("error result") {
        expected<int, std::string> result = make_unexpected("error");
        REQUIRE(!result);
        REQUIRE(result.error() == "error");
    }
    
    SECTION("map operation") {
        expected<int, std::string> result = 10;
        auto doubled = result.map([](int x) { return x * 2; });
        REQUIRE(doubled);
        REQUIRE(*doubled == 20);
    }
}

TEST_CASE("tl::optional basic operations", "[core][optional]") {
    using tl::optional;
    using tl::nullopt;
    
    SECTION("has value") {
        optional<int> opt = 42;
        REQUIRE(opt.has_value());
        REQUIRE(*opt == 42);
    }
    
    SECTION("no value") {
        optional<int> opt = nullopt;
        REQUIRE(!opt.has_value());
    }
    
    SECTION("value or") {
        optional<int> opt1 = 10;
        optional<int> opt2 = nullopt;
        
        REQUIRE(opt1.value_or(0) == 10);
        REQUIRE(opt2.value_or(0) == 0);
    }
}
