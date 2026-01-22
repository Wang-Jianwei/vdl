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
    unit/test_types.cpp
    unit/test_error.cpp
    unit/test_buffer.cpp
    unit/test_memory.cpp
    unit/test_logging.cpp
    unit/test_scope_guard.cpp
    unit/test_transport.cpp
    unit/test_codec.cpp
    unit/test_command.cpp
    unit/test_response.cpp
    unit/test_device.cpp
    unit/test_heartbeat.cpp
)

target_include_directories(vdl_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(vdl_tests PRIVATE 
    vdl
    Catch2::Catch2
)

# Standard CTest integration
add_test(NAME "All Tests" COMMAND vdl_tests WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# ============================================================================
# Test Categories
# ============================================================================

# Run only core tests
add_test(NAME "Core Tests" COMMAND vdl_tests "[core]" WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Run only transport tests
add_test(NAME "Transport Tests" COMMAND vdl_tests "[transport]" WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Run only codec tests
add_test(NAME "Codec Tests" COMMAND vdl_tests "[codec]" WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Run only device tests
add_test(NAME "Device Tests" COMMAND vdl_tests "[device]" WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Run only heartbeat tests
add_test(NAME "Heartbeat Tests" COMMAND vdl_tests "[heartbeat]" WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# ============================================================================
# Test directories
# ============================================================================

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/integration")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/integration")
endif()

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/mocks")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/mocks")
endif()
