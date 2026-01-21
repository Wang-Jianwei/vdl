/**
 * @file 01_basic_usage.cpp
 * @brief VDL 库基础用法示例
 * 
 * 本示例展示如何使用 VDL 库进行基本的设备通信操作。
 * 
 * 编译: g++ -std=c++11 01_basic_usage.cpp -I../include -I../third_party -o example_basic
 */

#include <vdl/vdl.hpp>
#include <iostream>
#include <memory>

// ============================================================================
// 示例1: 传输层创建
// ============================================================================

void example_transport_creation() {
    std::cout << "示例1: 传输层创建\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    std::cout << "创建模拟传输层...\n";
    auto transport = std::unique_ptr<vdl::i_transport_t>(
        new vdl::mock_transport_t()
    );
    std::cout << "✓ 传输层创建成功\n\n";
}

// ============================================================================
// 示例2: 设备创建
// ============================================================================

void example_device_creation() {
    std::cout << "示例2: 设备创建\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    auto transport = std::unique_ptr<vdl::i_transport_t>(
        new vdl::mock_transport_t()
    );
    auto codec = std::unique_ptr<vdl::i_codec_t>(
        new vdl::binary_codec_t()
    );

    // 创建设备
    auto device = std::unique_ptr<vdl::i_device_t>(
        new vdl::device_impl_t(std::move(transport), std::move(codec))
    );

    std::cout << "✓ 设备创建成功\n\n";
}

// ============================================================================
// 示例3: 设备连接
// ============================================================================

void example_connection() {
    std::cout << "示例3: 设备连接\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    auto transport = std::unique_ptr<vdl::i_transport_t>(
        new vdl::mock_transport_t()
    );
    auto codec = std::unique_ptr<vdl::i_codec_t>(
        new vdl::binary_codec_t()
    );

    auto device = std::unique_ptr<vdl::i_device_t>(
        new vdl::device_impl_t(std::move(transport), std::move(codec))
    );

    // 连接设备
    std::cout << "连接设备...\n";
    auto result = device->connect();
    if (result.has_value()) {
        std::cout << "✓ 连接成功\n\n";
    } else {
        std::cout << "✗ 连接失败: " << result.error().to_string() << "\n\n";
    }
}

// ============================================================================
// 示例4: 命令执行
// ============================================================================

void example_command_execution() {
    std::cout << "示例4: 命令执行\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    auto transport = std::unique_ptr<vdl::i_transport_t>(
        new vdl::mock_transport_t()
    );
    auto codec = std::unique_ptr<vdl::i_codec_t>(
        new vdl::binary_codec_t()
    );

    auto device = std::unique_ptr<vdl::i_device_t>(
        new vdl::device_impl_t(std::move(transport), std::move(codec))
    );

    // 连接
    if (!device->connect().has_value()) {
        std::cout << "连接失败\n\n";
        return;
    }

    std::cout << "已连接到设备\n\n";

    // 创建命令
    vdl::command_t cmd;
    cmd.set_function_code(0x03);
    cmd.set_data({0x00, 0x10, 0x00, 0x08});

    std::cout << "执行命令...\n";
    std::cout << "  函数码: 0x" << std::hex << (int)cmd.function_code() << std::dec << "\n";
    std::cout << "  数据: ";
    for (auto b : cmd.data()) {
        std::cout << "0x" << std::hex << (int)b << std::dec << " ";
    }
    std::cout << "\n\n";

    auto result = device->execute(cmd);
    if (result.has_value()) {
        auto& response = *result;
        std::cout << "✓ 命令执行成功\n";
        std::cout << "  函数码: 0x" << std::hex << (int)response.function_code() << std::dec << "\n";
        std::cout << "  数据大小: " << response.data().size() << " 字节\n\n";
    } else {
        std::cout << "✗ 命令执行失败: " << result.error().to_string() << "\n\n";
    }
}

// ============================================================================
// 示例5: 状态检查
// ============================================================================

void example_state_checking() {
    std::cout << "示例5: 状态检查\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    auto transport = std::unique_ptr<vdl::i_transport_t>(
        new vdl::mock_transport_t()
    );
    auto codec = std::unique_ptr<vdl::i_codec_t>(
        new vdl::binary_codec_t()
    );

    auto device = std::unique_ptr<vdl::i_device_t>(
        new vdl::device_impl_t(std::move(transport), std::move(codec))
    );

    // 初始状态
    std::cout << "初始状态:\n";
    std::cout << "  已连接: " << (device->is_connected() ? "是" : "否") << "\n";
    std::cout << "  状态: " << vdl::device_state_name(device->state()) << "\n\n";

    // 连接后
    device->connect();
    std::cout << "连接后状态:\n";
    std::cout << "  已连接: " << (device->is_connected() ? "是" : "否") << "\n";
    std::cout << "  状态: " << vdl::device_state_name(device->state()) << "\n\n";

    // 断开后
    device->disconnect();
    std::cout << "断开后状态:\n";
    std::cout << "  已连接: " << (device->is_connected() ? "是" : "否") << "\n";
    std::cout << "  状态: " << vdl::device_state_name(device->state()) << "\n\n";
}

// ============================================================================
// 主函数
// ============================================================================

int main() {
    std::cout << "VDL 库基础用法示例\n" << std::endl;

    try {
        example_transport_creation();
        example_device_creation();
        example_connection();
        example_command_execution();
        example_state_checking();
        
        std::cout << "✓ 所有示例执行完成\n" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ 异常: " << e.what() << std::endl;
        return 1;
    }
}
