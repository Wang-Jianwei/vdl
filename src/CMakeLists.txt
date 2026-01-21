# ============================================================================
# VDL Library
# ============================================================================

# Create library with source files
add_library(vdl STATIC
    logging.cpp
    error.cpp
    memory.cpp
    buffer.cpp
)

target_include_directories(vdl 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(vdl PUBLIC cxx_std_11)

# Link third-party libraries
target_link_libraries(vdl PUBLIC
    tl::expected
    tl::optional
    spdlog::spdlog
    nlohmann_json::nlohmann_json
)

# ============================================================================
# Installation
# ============================================================================

install(TARGETS vdl
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
)
