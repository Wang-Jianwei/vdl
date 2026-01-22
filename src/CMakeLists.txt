# ============================================================================
# VDL Library (Static Library)
# ============================================================================

# 收集源文件
set(VDL_SOURCES
    heartbeat/heartbeat_runner.cpp
)

# 创建静态库
add_library(vdl STATIC ${VDL_SOURCES})

target_include_directories(vdl 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../third_party>
        $<INSTALL_INTERFACE:include>
)

# 链接 tl::expected 和 tl::optional (核心依赖)
target_link_libraries(vdl PUBLIC tl::expected tl::optional)

# C++11 标准
target_compile_features(vdl PUBLIC cxx_std_11)

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
