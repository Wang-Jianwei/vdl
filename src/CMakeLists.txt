# ============================================================================
# VDL Library (Header-Only)
# ============================================================================

# 创建 INTERFACE 库（header-only）
add_library(vdl INTERFACE)

target_include_directories(vdl 
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../third_party>
        $<INSTALL_INTERFACE:include>
)

# 链接 tl::expected 和 tl::optional (核心依赖)
target_link_libraries(vdl INTERFACE tl::expected tl::optional)

# C++11 标准
target_compile_features(vdl INTERFACE cxx_std_11)

# ============================================================================
# Installation (简化版本，避免导出依赖问题)
# ============================================================================

install(TARGETS vdl
        EXPORT vdlTargets
        INCLUDES DESTINATION include
)

# 注意: 完整的导出需要处理 tl::expected 依赖
# 对于开发阶段，我们暂时省略导出配置
# install(EXPORT vdlTargets
#         FILE vdlTargets.cmake
#         NAMESPACE vdl::
#         DESTINATION lib/cmake/vdl
# )
