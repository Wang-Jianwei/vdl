/**
 * @file vdl.hpp
 * @brief VDL 库主头文件
 * 
 * 包含此文件即可使用 VDL 的所有功能。
 * 
 * @code
 * #include <vdl/vdl.hpp>
 * 
 * using namespace vdl;
 * 
 * int main() {
 *     // 使用 mock 传输层和二进制编解码器
 *     auto transport = vdl::make_unique<mock_transport_t>();
 *     auto codec = vdl::make_unique<binary_codec_t>();
 *     
 *     device_impl_t device(std::move(transport), std::move(codec));
 *     device.connect();
 *     
 *     auto result = device.execute(make_read_command(0x03, 0x0000, 10));
 *     if (result) {
 *         // 处理响应
 *     }
 *     
 *     return 0;
 * }
 * @endcode
 */

#ifndef VDL_VDL_HPP
#define VDL_VDL_HPP

// ============================================================================
// 核心模块
// ============================================================================

#include "core/compat.hpp"
#include "core/noncopyable.hpp"
#include "core/types.hpp"
#include "core/error.hpp"
#include "core/buffer.hpp"
#include "core/memory.hpp"
#include "core/logging.hpp"
#include "core/scope_guard.hpp"

// ============================================================================
// 协议模块
// ============================================================================

#include "protocol/command.hpp"
#include "protocol/response.hpp"

// ============================================================================
// 编解码模块
// ============================================================================

#include "codec/codec.hpp"
#include "codec/binary_codec.hpp"

// ============================================================================
// 传输层模块
// ============================================================================

#include "transport/transport.hpp"
#include "transport/mock_transport.hpp"

// ============================================================================
// 设备层模块
// ============================================================================

#include "device/device.hpp"
#include "device/device_impl.hpp"
#include "device/device_guard.hpp"

// ============================================================================
// 版本信息
// ============================================================================

namespace vdl {

/**
 * @brief VDL 版本信息
 */
struct version_t {
    static constexpr int major = 0;
    static constexpr int minor = 3;
    static constexpr int patch = 0;
    static constexpr const char* string = "0.3.0";
};

}  // namespace vdl

#endif  // VDL_VDL_HPP
