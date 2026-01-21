/**
 * @file test_scope_guard.cpp
 * @brief 测试 scope_guard_t
 */

#include <catch.hpp>
#include <vdl/core/scope_guard.hpp>

// ============================================================================
// scope_guard_t 测试
// ============================================================================

TEST_CASE("scope_guard_t executes on destruction", "[core][scope_guard]") {
    bool executed = false;

    {
        auto guard = vdl::make_scope_guard([&]() {
            executed = true;
        });
        
        REQUIRE_FALSE(executed);
    }

    REQUIRE(executed);
}

TEST_CASE("scope_guard_t can be dismissed", "[core][scope_guard]") {
    bool executed = false;

    {
        auto guard = vdl::make_scope_guard([&]() {
            executed = true;
        });
        
        guard.dismiss();
    }

    REQUIRE_FALSE(executed);
}

TEST_CASE("scope_guard_t is_active check", "[core][scope_guard]") {
    auto guard = vdl::make_scope_guard([]() {});
    
    REQUIRE(guard.is_active());
    
    guard.dismiss();
    
    REQUIRE_FALSE(guard.is_active());
}

TEST_CASE("scope_guard_t can be moved", "[core][scope_guard]") {
    bool executed = false;

    {
        auto guard1 = vdl::make_scope_guard([&]() {
            executed = true;
        });
        
        auto guard2 = std::move(guard1);
        
        // guard1 已失效
        REQUIRE_FALSE(guard1.is_active());
        REQUIRE(guard2.is_active());
    }

    REQUIRE(executed);
}

// ============================================================================
// VDL_SCOPE_EXIT 宏测试
// ============================================================================

TEST_CASE("VDL_SCOPE_EXIT macro works", "[core][scope_guard]") {
    int counter = 0;

    {
        VDL_SCOPE_EXIT { counter++; };
        
        REQUIRE(counter == 0);
    }

    REQUIRE(counter == 1);
}

TEST_CASE("Multiple VDL_SCOPE_EXIT execute in reverse order", "[core][scope_guard]") {
    std::vector<int> order;

    {
        VDL_SCOPE_EXIT { order.push_back(1); };
        VDL_SCOPE_EXIT { order.push_back(2); };
        VDL_SCOPE_EXIT { order.push_back(3); };
    }

    REQUIRE(order.size() == 3);
    REQUIRE(order[0] == 3);  // 最后声明的最先执行
    REQUIRE(order[1] == 2);
    REQUIRE(order[2] == 1);
}

TEST_CASE("VDL_SCOPE_EXIT captures variables", "[core][scope_guard]") {
    int value = 10;

    {
        VDL_SCOPE_EXIT { value *= 2; };
        
        value += 5;
        REQUIRE(value == 15);
    }

    REQUIRE(value == 30);  // 15 * 2
}
