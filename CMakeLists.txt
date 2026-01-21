cmake_minimum_required(VERSION 3.10)
project(vdl VERSION 1.0.0 LANGUAGES CXX)

# ============================================================================
# Project Settings
# ============================================================================

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ============================================================================
# Include CMake Modules
# ============================================================================

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CompilerOptions.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindDependencies.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Catch2Helpers.cmake)

# ============================================================================
# Project Structure
# ============================================================================

# Main library
add_subdirectory(src)

# Enable testing
enable_testing()
add_subdirectory(tests)

# Examples
add_subdirectory(examples)

# ============================================================================
# Export configuration
# ============================================================================

# Create version file
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/VdlConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/VdlConfig.cmake"
    @ONLY
)

# Install headers
install(DIRECTORY include/vdl DESTINATION include)

# Install CMake config
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/VdlConfig.cmake" 
        DESTINATION lib/cmake/vdl)
