/**
 * @file 08_visa_communication_example.cpp
 * @brief VDL VISA 通信示例
 * 
 * 本示例展示了 VISA Transport 的使用方法和最佳实践
 */

#include <vdl/vdl.hpp>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

// ============================================================================
// 输出工具
// ============================================================================

void print_header(const std::string& title) {
    std::cout << "\n";
    std::cout << "====================================\n";
    std::cout << "  " << title << "\n";
    std::cout << "====================================\n";
}

// ============================================================================
// 示例 1: VISA 通信基础
// ============================================================================

void example_visa_basics() {
    print_header("示例 1: VISA 通信基础");

    std::cout << "\nVISA (Virtual Instrument Software Architecture) 是工业标准\n";
    std::cout << "虚拟仪器编程接口，支持多种传输方式。\n\n";

    std::cout << "支持的 VISA 资源类型:\n";
    std::cout << "  1. GPIB (IEEE 488)\n";
    std::cout << "     示例: GPIB0::192::INSTR\n";
    std::cout << "     用途: 传统 GPIB 接口仪器\n\n";

    std::cout << "  2. TCP/IP 网络\n";
    std::cout << "     示例: TCPIP::192.168.1.100::INSTR\n";
    std::cout << "     用途: 网络连接的仪器\n\n";

    std::cout << "  3. USB 接口\n";
    std::cout << "     示例: USB::0x0699::0x0341::INSTR\n";
    std::cout << "     用途: USB 连接的仪器\n\n";

    std::cout << "  4. 串口通信\n";
    std::cout << "     示例: ASRL1::INSTR\n";
    std::cout << "     用途: 串口连接的仪器\n\n";
}

// ============================================================================
// 示例 2: VDL + VISA 的集成方式
// ============================================================================

void example_visa_integration() {
    print_header("示例 2: VDL + VISA 集成方式");

    std::cout << "\n分层架构:\n\n";
    std::cout << "  应用程序\n";
    std::cout << "      |\n";
    std::cout << "      +-- SCPI 适配器 (既有库)\n";
    std::cout << "      |   提供高级接口\n";
    std::cout << "      |\n";
    std::cout << "      +-- Device 实现 (既有库)\n";
    std::cout << "      |   组合 Transport + Codec\n";
    std::cout << "      |\n";
    std::cout << "      +-- VISA Transport (新增)\n";
    std::cout << "      |   包装 VISA 库调用\n";
    std::cout << "      |\n";
    std::cout << "      +-- NI-VISA 或其他 VISA 库\n";
    std::cout << "          实际硬件通信\n\n";

    std::cout << "优势:\n";
    std::cout << "  ✓ 无需修改库核心\n";
    std::cout << "  ✓ 支持所有 VISA 传输方式\n";
    std::cout << "  ✓ 完全复用现有代码\n";
    std::cout << "  ✓ 清晰的职责分离\n\n";
}

// ============================================================================
// 示例 3: VISA Transport 的使用
// ============================================================================

void example_visa_transport_usage() {
    print_header("示例 3: VISA Transport 的使用");

    std::cout << "\n基本使用步骤:\n\n";

    std::cout << "第 1 步: 创建 VISA Transport\n";
    std::cout << "─────────────────────────────────────\n";
    std::cout << "auto transport = std::make_unique<visa_transport_t>(\n";
    std::cout << "    \"TCPIP::192.168.1.100::INSTR\"\n";
    std::cout << ");\n\n";

    std::cout << "第 2 步: 创建 SCPI Codec\n";
    std::cout << "─────────────────────────────────────\n";
    std::cout << "auto codec = std::make_unique<vdl::binary_codec_t>();\n\n";

    std::cout << "第 3 步: 创建设备\n";
    std::cout << "─────────────────────────────────────\n";
    std::cout << "vdl::device_impl_t device(\n";
    std::cout << "    std::move(transport),\n";
    std::cout << "    std::move(codec)\n";
    std::cout << ");\n\n";

    std::cout << "第 4 步: 连接和使用\n";
    std::cout << "─────────────────────────────────────\n";
    std::cout << "device.connect();\n";
    std::cout << "vdl::scpi_adapter_t scpi(device);\n";
    std::cout << "auto idn = scpi.get_idn();\n";
    std::cout << "device.disconnect();\n\n";
}

// ============================================================================
// 示例 4: 多种 VISA 连接方式
// ============================================================================

void example_multiple_connections() {
    print_header("示例 4: 多种 VISA 连接方式");

    std::cout << "\n同一代码支持所有 VISA 传输:\n\n";

    std::cout << "// TCP/IP 连接\n";
    std::cout << "auto transport1 = std::make_unique<visa_transport_t>(\n";
    std::cout << "    \"TCPIP::192.168.1.100::INSTR\"\n";
    std::cout << ");\n\n";

    std::cout << "// GPIB 连接\n";
    std::cout << "auto transport2 = std::make_unique<visa_transport_t>(\n";
    std::cout << "    \"GPIB0::192::INSTR\"\n";
    std::cout << ");\n\n";

    std::cout << "// USB 连接\n";
    std::cout << "auto transport3 = std::make_unique<visa_transport_t>(\n";
    std::cout << "    \"USB::0x0699::0x0341::INSTR\"\n";
    std::cout << ");\n\n";

    std::cout << "// 所有连接使用相同的方式:\n";
    std::cout << "vdl::device_impl_t device(std::move(transport), codec);\n";
    std::cout << "device.connect();\n";
    std::cout << "// ... 使用设备 ...\n\n";

    std::cout << "优势:\n";
    std::cout << "  ✓ 无需修改应用代码\n";
    std::cout << "  ✓ 仅需改变资源字符串\n";
    std::cout << "  ✓ 支持所有 VISA 传输\n\n";
}

// ============================================================================
// 示例 5: 完整的示例代码
// ============================================================================

void example_complete_code() {
    print_header("示例 5: 完整的代码示例");

    std::cout << "\n// 包含必要的头文件\n";
    std::cout << "#include <vdl/vdl.hpp>\n";
    std::cout << "#include \"visa_transport.hpp\"\n";
    std::cout << "#include <iostream>\n\n";

    std::cout << "int main() {\n";
    std::cout << "    try {\n";
    std::cout << "        // 创建 VISA Transport\n";
    std::cout << "        auto transport = std::make_unique<visa_transport_t>(\n";
    std::cout << "            \"TCPIP::192.168.1.100::INSTR\"\n";
    std::cout << "        );\n\n";

    std::cout << "        // 创建 SCPI Codec\n";
    std::cout << "        auto codec = std::make_unique<vdl::binary_codec_t>();\n\n";

    std::cout << "        // 创建设备\n";
    std::cout << "        vdl::device_impl_t device(\n";
    std::cout << "            std::move(transport),\n";
    std::cout << "            std::move(codec)\n";
    std::cout << "        );\n\n";

    std::cout << "        // 连接到设备\n";
    std::cout << "        auto result = device.connect();\n";
    std::cout << "        if (!result) {\n";
    std::cout << "            std::cerr << \"Connection failed\" << std::endl;\n";
    std::cout << "            return 1;\n";
    std::cout << "        }\n\n";

    std::cout << "        // 使用 SCPI 适配器\n";
    std::cout << "        vdl::scpi_adapter_t scpi(device);\n\n";

    std::cout << "        // 查询设备\n";
    std::cout << "        auto idn = scpi.get_idn();\n";
    std::cout << "        if (idn) {\n";
    std::cout << "            std::cout << \"Device: \" << *idn << std::endl;\n";
    std::cout << "        }\n\n";

    std::cout << "        // 复位设备\n";
    std::cout << "        scpi.reset();\n";
    std::cout << "        scpi.clear_status();\n\n";

    std::cout << "        // 执行 SCPI 命令\n";
    std::cout << "        auto freq = scpi.query_double(\":FREQ:CENT?\");\n";
    std::cout << "        if (freq) {\n";
    std::cout << "            std::cout << \"Frequency: \" << (*freq / 1e9)\n";
    std::cout << "                      << \" GHz\" << std::endl;\n";
    std::cout << "        }\n\n";

    std::cout << "        // 断开连接\n";
    std::cout << "        device.disconnect();\n";
    std::cout << "        return 0;\n\n";

    std::cout << "    } catch (const std::exception& e) {\n";
    std::cout << "        std::cerr << \"Error: \" << e.what() << std::endl;\n";
    std::cout << "        return 1;\n";
    std::cout << "    }\n";
    std::cout << "}\n\n";
}

// ============================================================================
// 示例 6: 错误处理
// ============================================================================

void example_error_handling() {
    print_header("示例 6: 错误处理最佳实践");

    std::cout << "\n模式 1: 检查返回值\n";
    std::cout << "────────────────────────\n";
    std::cout << "auto result = device.connect();\n";
    std::cout << "if (!result) {\n";
    std::cout << "    std::cerr << \"Error: \" << result.error().message();\n";
    std::cout << "    return false;\n";
    std::cout << "}\n\n";

    std::cout << "模式 2: 异常处理\n";
    std::cout << "────────────────────────\n";
    std::cout << "try {\n";
    std::cout << "    auto result = device.connect();\n";
    std::cout << "    if (!result) throw std::runtime_error(\"Connection failed\");\n";
    std::cout << "    // ... 使用设备 ...\n";
    std::cout << "} catch (const std::exception& e) {\n";
    std::cout << "    std::cerr << \"Exception: \" << e.what();\n";
    std::cout << "}\n\n";

    std::cout << "最佳实践:\n";
    std::cout << "  ✓ 总是检查 result_t 的有效性\n";
    std::cout << "  ✓ 使用 RAII 确保资源释放\n";
    std::cout << "  ✓ 提供清晰的错误信息\n";
    std::cout << "  ✓ 传播错误而不是吞掉\n\n";
}

// ============================================================================
// 示例 7: 配置管理
// ============================================================================

void example_configuration() {
    print_header("示例 7: 配置和扩展");

    std::cout << "\nVISA 资源配置助手:\n\n";

    std::cout << "// GPIB 配置\n";
    std::cout << "std::string gpib_resource = \n";
    std::cout << "    \"GPIB0::\" + std::to_string(address) + \"::INSTR\";\n\n";

    std::cout << "// TCP/IP 配置\n";
    std::cout << "std::string tcp_resource = \n";
    std::cout << "    \"TCPIP::\" + host + \"::5025::SOCKET\";\n\n";

    std::cout << "// USB 配置\n";
    std::cout << "std::string usb_resource = \n";
    std::cout << "    \"USB::0x\" + hex(vendor_id) + \"::0x\" + hex(product_id) \n";
    std::cout << "    + \"::INSTR\";\n\n";

    std::cout << "// Serial 配置\n";
    std::cout << "std::string serial_resource = \n";
    std::cout << "    \"ASRL\" + std::to_string(port) + \"::INSTR\";\n\n";

    std::cout << "优势:\n";
    std::cout << "  ✓ 统一的配置方式\n";
    std::cout << "  ✓ 减少错误\n";
    std::cout << "  ✓ 易于维护\n\n";
}

// ============================================================================
// 主函数
// ============================================================================

int main() {
    std::cout << "\n";
    std::cout << "============================================\n";
    std::cout << "  VDL VISA 通信完整示例\n";
    std::cout << "============================================\n";

    // 运行所有示例
    example_visa_basics();
    example_visa_integration();
    example_visa_transport_usage();
    example_multiple_connections();
    example_complete_code();
    example_error_handling();
    example_configuration();

    std::cout << "============================================\n";
    std::cout << "  所有示例完成\n";
    std::cout << "============================================\n\n";

    std::cout << "关键要点:\n";
    std::cout << "  1. VISA Transport 处理底层通信\n";
    std::cout << "  2. SCPI Codec 处理协议解析\n";
    std::cout << "  3. Device 层组合两者\n";
    std::cout << "  4. 应用层无需了解 VISA 细节\n";
    std::cout << "  5. 支持所有 VISA 传输方式\n\n";

    std::cout << "参考文档: VISA_ADAPTATION_PLAN.md\n";
    std::cout << "参考实现: examples/visa_transport.hpp\n\n";

    return 0;
}
