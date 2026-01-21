# ============================================================================
# VDL Tests with Catch2
# ============================================================================

enable_testing()

# Set CMake policy for test names with spaces
if(POLICY CMP0110)
    cmake_policy(SET CMP0110 NEW)
endif()

# ============================================================================
# Unit Tests
# ============================================================================

add_executable(vdl_tests
    test_main.cpp
    unit/test_compat.cpp
    unit/test_core.cpp
    unit/test_examples.cpp
    unit/test_types.cpp
    unit/test_logging.cpp
    unit/test_error.cpp
    unit/test_memory.cpp
    unit/test_buffer.cpp
)

target_include_directories(vdl_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(vdl_tests PRIVATE 
    vdl
    Catch2::Catch2
)

# Add Catch2 test discovery
# catch_discover_tests(vdl_tests WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Standard CTest integration
add_test(NAME "All Tests" COMMAND vdl_tests WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# ============================================================================
# Test Categories
# ============================================================================

# Run only core tests
add_test(NAME "Core Tests" COMMAND vdl_tests "[core]" WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Run only basic tests
add_test(NAME "Basic Tests" COMMAND vdl_tests "[basic]" WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Run only JSON tests
add_test(NAME "JSON Tests" COMMAND vdl_tests "[json]" WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Run only compatibility tests
add_test(NAME "Compat Tests" COMMAND vdl_tests "[compat]" WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Run only types tests
add_test(NAME "Types Tests" COMMAND vdl_tests "[types]" WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Run only logging tests
add_test(NAME "Logging Tests" COMMAND vdl_tests "[logging]" WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Run only error tests
add_test(NAME "Error Tests" COMMAND vdl_tests "[error]" WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Run only memory tests
add_test(NAME "Memory Tests" COMMAND vdl_tests "[memory]" WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Run only buffer tests
add_test(NAME "Buffer Tests" COMMAND vdl_tests "[buffer]" WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# ============================================================================
# Test directories
# ============================================================================

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/integration")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/integration")
endif()

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/mocks")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/mocks")
endif()
