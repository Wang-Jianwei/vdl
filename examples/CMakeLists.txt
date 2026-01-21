cmake_minimum_required(VERSION 3.10)

# ============================================================================
# 示例编译配置
# ============================================================================

# 获取所有 .cpp 文件
file(GLOB EXAMPLE_SOURCES "*.cpp")

# 为每个示例创建可执行文件
foreach(SOURCE ${EXAMPLE_SOURCES})
    get_filename_component(BASENAME ${SOURCE} NAME_WE)
    add_executable(${BASENAME} ${SOURCE})
    
    # 设置编译选项
    target_compile_features(${BASENAME} PRIVATE cxx_std_11)
    
    # 链接 VDL 库
    target_link_libraries(${BASENAME} PRIVATE vdl)
endforeach()
