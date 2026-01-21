# VDL Test Framework

This directory contains the test suite for the VDL project using Catch2.

## Structure

```
tests/
├── test_main.cpp           # Catch2 main entry point
├── test_utils.hpp          # Test utilities and helpers
├── test_third_party.cpp    # Third-party library verification tests
├── unit/
│   ├── test_core.cpp       # Core module tests (expected, optional)
│   └── test_examples.cpp   # Example tests (JSON, arithmetic)
├── integration/            # Integration tests (future)
└── mocks/                  # Mock implementations (future)
```

## Running Tests

### Build the test executable
```bash
cd /workspaces/vdl
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

### Run all tests
```bash
./bin/vdl_tests
# or
ctest --verbose
```

### Run specific test cases by tag
```bash
# Run only core tests
./bin/vdl_tests [core]

# Run only basic tests
./bin/vdl_tests [basic]

# Run only JSON tests
./bin/vdl_tests [json]
```

### Run with detailed output
```bash
./bin/vdl_tests -v
./bin/vdl_tests --show-successes
```

## Test Framework - Catch2

### Key Features
- Single-header library (643 KB)
- C++11 compatible
- BDD-style assertions
- Test tagging and filtering
- Flexible test discovery
- Rich output formatting

### Basic Test Structure

```cpp
#include <catch.hpp>

TEST_CASE("description", "[tag1][tag2]") {
    SECTION("section 1") {
        // Test code here
        REQUIRE(condition);
    }
    
    SECTION("section 2") {
        // More test code
        CHECK(condition);
    }
}
```

### Common Assertions

- `REQUIRE(condition)` - Hard assertion (fails immediately)
- `CHECK(condition)` - Soft assertion (continues after failure)
- `REQUIRE_THROWS(expr)` - Exception handling
- `REQUIRE_NOTHROW(expr)` - No exception
- `REQUIRE_THROWS_AS(expr, ExceptionType)` - Specific exception

### Test Organization

Tests are organized by tags:
- `[core]` - Core module tests
- `[basic]` - Basic functionality tests
- `[json]` - JSON-related tests
- `[integration]` - Integration tests
- `[third_party]` - Third-party library verification

## Test Utilities

### Common Helpers

File: `test_utils.hpp`

```cpp
// VDL-specific assertions
#define VDL_REQUIRE_OK(expr)      // Assert Result is Ok
#define VDL_REQUIRE_ERROR(expr)   // Assert Result is Error

// Test configuration
struct TestConfig {
    bool verbose;
    int timeout_ms;
};
```

## Writing New Tests

1. Create a new file in `tests/unit/` or `tests/integration/`
2. Include `<catch.hpp>`
3. Write test cases with appropriate tags
4. Build and run with `make test`

Example:

```cpp
// tests/unit/test_my_feature.cpp
#include <catch.hpp>
#include <vdl/my_feature.hpp>

TEST_CASE("my feature works", "[my_feature]") {
    REQUIRE(vdl::my_feature() == expected_value);
}
```

Then add to `tests/CMakeLists.txt`:

```cmake
add_executable(vdl_tests
    test_main.cpp
    unit/test_my_feature.cpp  # Add this line
    ...
)
```

## Continuous Integration

The test framework is designed to integrate with CI/CD pipelines:

```bash
# Generate XML report (compatible with Jenkins, etc.)
./bin/vdl_tests -r junit -o test_results.xml

# Generate JSON report
./bin/vdl_tests -r json -o test_results.json
```

## Performance Testing

For performance-critical tests:

```cpp
TEST_CASE("performance test", "[.performance]") {
    // Use [.performance] tag to skip in normal runs
    // Run with: ./bin/vdl_tests [.performance]
    
    auto start = std::chrono::high_resolution_clock::now();
    // Code to measure
    auto end = std::chrono::high_resolution_clock::now();
    
    REQUIRE(std::chrono::duration_cast<std::chrono::milliseconds>(
        end - start).count() < 100);
}
```

## Debugging Tests

### Run specific test by name
```bash
./bin/vdl_tests "test name"
```

### Run with extra debugging
```bash
./bin/vdl_tests -b  # Break on assertion failure (if debugger attached)
```

### Suppress output
```bash
./bin/vdl_tests -q  # Quiet output
```

## Future Enhancements

- [ ] Add Google Test integration
- [ ] Implement performance benchmarks
- [ ] Add coverage reporting
- [ ] Create fixture support for complex setups
- [ ] Add parameterized test support
