# ============================================================================
# Compiler Options and Flags
# ============================================================================

# Common flags for all compilers
if(MSVC)
    # Visual Studio
    add_compile_options(/W4)           # Warning level 4
    add_compile_options(/WX)           # Treat warnings as errors
    add_compile_options(/permissive-)  # Strict conformance
    # C4191: unsafe conversion
    # C4365: implicit conversion, possible loss of data
    add_compile_options(/w14191 /w14365)
else()
    # GCC / Clang
    add_compile_options(-Wall)         # Enable all common warnings
    add_compile_options(-Wextra)       # Enable extra warnings
    add_compile_options(-Wpedantic)    # Enable pedantic warnings
    add_compile_options(-Werror)       # Treat warnings as errors
    # 严格的类型转换检查 - 必须在所有编译环境中一致
    add_compile_options(-Wconversion)  # Warn on implicit conversions
    add_compile_options(-Wsign-conversion)  # Warn on sign conversions
    add_compile_options(-Wdouble-promotion) # Warn on implicit float->double
    add_compile_options(-Woverloaded-virtual)
    add_compile_options(-Wnon-virtual-dtor)
    add_compile_options(-fno-rtti)     # Disable RTTI (optional)
endif()

# ============================================================================
# Strict Conversion Checking - Ensures consistency across compilers
# ============================================================================
# This ensures that all implicit type conversions that could lose precision
# are caught during compilation, preventing bugs that only appear in
# different compilation environments with stricter settings.
message(STATUS "Strict type conversion checking enabled (-Wconversion / /w14365)")

# ============================================================================
# Debug and Release flags
# ============================================================================

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_definitions(VDL_DEBUG=1)
    if(MSVC)
        add_compile_options(/Zi /Od)
    else()
        add_compile_options(-g -O0)
    endif()
elseif(CMAKE_BUILD_TYPE MATCHES Release)
    if(MSVC)
        add_compile_options(/Ox /Oi)
    else()
        add_compile_options(-O3 -DNDEBUG)
    endif()
endif()

# ============================================================================
# Address Sanitizer (optional)
# ============================================================================

if(ENABLE_ASAN)
    if(MSVC)
        message(WARNING "Address Sanitizer not fully supported on MSVC")
    else()
        add_compile_options(-fsanitize=address)
        add_link_options(-fsanitize=address)
    endif()
endif()

# ============================================================================
# Code Coverage (optional)
# ============================================================================

if(ENABLE_COVERAGE)
    if(MSVC)
        message(WARNING "Code coverage not supported on MSVC")
    else()
        add_compile_options(-fprofile-arcs -ftest-coverage)
        add_link_options(--coverage)
    endif()
endif()
