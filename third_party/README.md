# Third Party Libraries

This directory contains third-party libraries integrated into VDL.

## Libraries

### tl::expected (tl_expected/)
- **Source**: https://github.com/TartanLlama/expected
- **License**: CC0 1.0 Universal
- **Usage**: `#include <tl/expected.hpp>`
- **Type**: Single header
- **Purpose**: Type-safe error handling without exceptions

### tl::optional (tl_optional/)
- **Source**: https://github.com/TartanLlama/optional
- **License**: CC0 1.0 Universal
- **Usage**: `#include <tl/optional.hpp>`
- **Type**: Single header
- **Purpose**: C++11 compatible optional type

### spdlog (spdlog/)
- **Source**: https://github.com/gabime/spdlog (simplified)
- **License**: MIT
- **Usage**: `#include <spdlog/spdlog.h>`
- **Type**: Header-only (simplified wrapper)
- **Purpose**: Logging framework

### nlohmann/json (nlohmann_json/)
- **Source**: https://github.com/nlohmann/json
- **License**: MIT
- **Usage**: `#include <nlohmann/json.hpp>`
- **Type**: Single header
- **Purpose**: JSON parsing and serialization

### Catch2 (catch2/)
- **Source**: https://github.com/catchorg/Catch2
- **License**: Boost Software License 1.0
- **Usage**: `#include <catch.hpp>`
- **Type**: Single header
- **Purpose**: Testing framework for C++11

## Integration

All libraries are integrated as header-only libraries. To use them:

1. In CMakeLists.txt, add:
   ```cmake
   include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party)
   ```

2. In your source code:
   ```cpp
   #include <tl/expected.hpp>
   #include <tl/optional.hpp>
   #include <spdlog/spdlog.h>
   #include <nlohmann/json.hpp>
   ```

## Updating Libraries

To update any library:

1. Download the latest version from the source repository
2. Replace the header files
3. Run tests to ensure compatibility
4. Update the license information if needed

## Notes

- All libraries are compatible with C++11
- No additional linking required
- Include paths are already configured in CMakeLists.txt
