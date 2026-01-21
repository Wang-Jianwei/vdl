/**
 * @file 05_complete_communication.cpp
 * @brief VDL 库完整通信流程示例
 * 
 * 本示例展示从设备初始化到完整通信流程的端到端演示。
 * 
 * 编译: g++ -std=c++11 05_complete_communication.cpp -I../include -I../third_party -o example_complete
 */

#include <vdl/vdl.hpp>
#include <iostream>
#include <memory>

// ============================================================================
// 示例1: 基础设备初始化和连接
// ============================================================================

void example_device_initialization() {
    std::cout << "示例1: 设备初始化和连接\n";
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

    std::cout << "初始状态: " << (device->is_connected() ? "已连接" : "未连接") << "\n";
    std::cout << "执行连接...\n";
    
    if (device->connect().has_value()) {
        std::cout << "✓ 连接成功\n";
        std::cout << "当前状态: " << (device->is_connected() ? "已连接" : "未连接") << "\n";
    } else {
        std::cout << "✗ 连接失败\n";
    }

    std::cout << "\n";
}

// ============================================================================
// 示例2: 单条命令执行
// ============================================================================

void example_single_command_execution() {
    std::cout << "示例2: 单条命令执行\n";
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

    if (!device->connect().has_value()) {
        std::cout << "连接失败\n\n";
        return;
    }

    std::cout << "已连接到设备\n\n";

    vdl::command_t cmd;
    cmd.set_function_code(0x03);
    cmd.set_data({0x00, 0x10, 0x00, 0x04});

    std::cout << "执行命令 (函数码=0x03)\n";
    
    auto result = device->execute(cmd);
    if (result.has_value()) {
        auto& response = *result;
        std::cout << "✓ 命令执行成功\n";
        std::cout << "  响应函数码: 0x" << std::hex << (int)response.function_code() << std::dec << "\n";
        std::cout << "  响应数据大小: " << response.data().size() << " 字节\n";
    } else {
        std::cout << "✗ 命令执行失败: " << result.error().to_string() << "\n";
    }

    device->disconnect();
    std::cout << "\n";
}

// ============================================================================
// 示例3: 多条命令顺序执行
// ============================================================================

void example_sequential_commands() {
    std::cout << "示例3: 多条命令顺序执行\n";
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

    if (!device->connect().has_value()) {
        std::cout << "连接失败\n\n";
        return;
    }

    std::cout << "已连接，执行命令序列\n\n";

    vdl::byte_t func_codes[] = {0x01, 0x03, 0x05};
    const char* descriptions[] = {
        "读取线圈状态",
        "读取保存寄存器",
        "写入单个线圈"
    };

    for (int i = 0; i < 3; ++i) {
        std::cout << "命令 #" << (i+1) << ": " << descriptions[i] << "\n";

        vdl::command_t cmd;
        cmd.set_function_code(func_codes[i]);
        cmd.set_data({0x00, 0x10});

        auto result = device->execute(cmd);
        if (result.has_value()) {
            std::cout << "  ✓ 执行成功\n";
        } else {
            std::cout << "  ✗ 执行失败: " << result.error().to_string() << "\n";
        }
    }

    device->disconnect();
    std::cout << "\n";
}

// ============================================================================
// 示例4: 错误处理
// ============================================================================

void example_error_handling() {
    std::cout << "示例4: 错误处理\n";
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

    // 场景1: 未连接时执行命令
    {
        std::cout << "场景1: 未连接时执行命令\n";
        vdl::command_t cmd;
        cmd.set_function_code(0x03);

        auto result = device->execute(cmd);
        if (!result.has_value()) {
            std::cout << "  预期的错误: " << result.error().to_string() << "\n\n";
        }
    }

    // 场景2: 连接成功后执行命令
    {
        std::cout << "场景2: 连接后执行命令\n";

        if (!device->connect().has_value()) {
            std::cout << "  连接失败\n\n";
            return;
        }
        std::cout << "  连接成功\n";

        vdl::command_t cmd;
        cmd.set_function_code(0x05);
        auto result = device->execute(cmd);

        if (result.has_value()) {
            std::cout << "  命令执行成功\n\n";
        } else {
            std::cout << "  命令执行失败\n\n";
        }
    }

    device->disconnect();
}

// ============================================================================
// 示例5: 设备状态追踪
// ============================================================================

void example_device_state_tracking() {
    std::cout << "示例5: 设备状态追踪\n";
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
    std::cout << "状态 #1 (初始化后):\n";
    std::cout << "  已连接: " << (device->is_connected() ? "是" : "否") << "\n";
    std::cout << "  设备状态: " << vdl::device_state_name(device->state()) << "\n\n";

    // 连接后
    device->connect();
    std::cout << "状态 #2 (连接后):\n";
    std::cout << "  已连接: " << (device->is_connected() ? "是" : "否") << "\n";
    std::cout << "  设备状态: " << vdl::device_state_name(device->state()) << "\n\n";

    // 执行命令
    vdl::command_t cmd;
    cmd.set_function_code(0x03);
    device->execute(cmd);

    std::cout << "状态 #3 (执行命令后):\n";
    std::cout << "  已连接: " << (device->is_connected() ? "是" : "否") << "\n";
    std::cout << "  设备状态: " << vdl::device_state_name(device->state()) << "\n\n";

    // 断开后
    device->disconnect();
    std::cout << "状态 #4 (断开后):\n";
    std::cout << "  已连接: " << (device->is_connected() ? "是" : "否") << "\n";
    std::cout << "  设备状态: " << vdl::device_state_name(device->state()) << "\n\n";
}

// ============================================================================
// 主函数
// ============================================================================

int main() {
    std::cout << "VDL 库完整通信流程示例\n" << std::endl;

    try {
        example_device_initialization();
        example_single_command_execution();
        example_sequential_commands();
        example_error_handling();
        example_device_state_tracking();

        std::cout << "✓ 所有示例执行完成\n" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ 异常: " << e.what() << std::endl;
        return 1;
    }
}
