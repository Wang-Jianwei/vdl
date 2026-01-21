// Mock and test utility definitions
#pragma once

#include <catch.hpp>
#include <tl/expected.hpp>
#include <tl/optional.hpp>

namespace vdl {
namespace test {

// Helper struct for test utilities
struct TestConfig {
    bool verbose = false;
    int timeout_ms = 1000;
};

// Common assertions
#define VDL_REQUIRE_OK(expr) REQUIRE((expr).operator bool())
#define VDL_REQUIRE_ERROR(expr) REQUIRE(!(expr).operator bool())

} // namespace test
} // namespace vdl
