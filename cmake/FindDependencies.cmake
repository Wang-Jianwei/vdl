# ============================================================================
# Find and configure third-party dependencies for VDL
# ============================================================================

# Add third_party directory to include path
set(THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party")

# ============================================================================
# TL::Expected (核心依赖)
# ============================================================================

if(NOT TARGET tl::expected)
    add_library(tl_expected INTERFACE)
    target_include_directories(tl_expected INTERFACE ${THIRD_PARTY_DIR}/tl_expected)
    add_library(tl::expected ALIAS tl_expected)
endif()

# ============================================================================
# TL::Optional (可选，用于兼容性)
# ============================================================================

if(NOT TARGET tl::optional)
    add_library(tl_optional INTERFACE)
    target_include_directories(tl_optional INTERFACE ${THIRD_PARTY_DIR}/tl_optional)
    add_library(tl::optional ALIAS tl_optional)
endif()

# ============================================================================
# Catch2 (仅测试使用)
# ============================================================================

if(NOT TARGET Catch2::Catch2)
    add_library(Catch2 INTERFACE)
    target_include_directories(Catch2 INTERFACE ${THIRD_PARTY_DIR}/catch2)
    add_library(Catch2::Catch2 ALIAS Catch2)
endif()

message(STATUS "Third-party libraries configured:")
message(STATUS "  - tl::expected")
message(STATUS "  - tl::optional")
message(STATUS "  - Catch2::Catch2")
