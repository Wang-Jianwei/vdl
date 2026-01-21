/**
 * @file 04_codec_and_protocol.cpp
 * @brief VDL 库编解码和协议示例
 * 
 * 本示例展示如何使用编解码器进行数据编码和解码操作。
 * 
 * 编译: g++ -std=c++11 04_codec_and_protocol.cpp -I../include -o example_codec
 */

#include <vdl/vdl.hpp>
#include <iostream>
#include <iomanip>

// ============================================================================
// 辅助函数
// ============================================================================

void print_hex_data(const vdl::byte_t* data, size_t len) {
    std::cout << "  ";
    for (size_t i = 0; i < len; ++i) {
        std::cout << "0x" << std::hex << std::setfill('0') << std::setw(2) 
                  << (int)data[i] << std::dec << " ";
        if ((i + 1) % 16 == 0) {
            std::cout << "\n  ";
        }
    }
    std::cout << std::endl;
}

// ============================================================================
// 示例1: 命令的创建和操作
// ============================================================================

void example_command_creation() {
    std::cout << "示例1: 命令创建\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    // 创建简单命令
    {
        std::cout << "创建命令 (函数码=0x03, 无数据):\n";
        vdl::command_t cmd;
        cmd.set_function_code(0x03);
        
        std::cout << "  函数码: 0x" << std::hex << (int)cmd.function_code() << std::dec << "\n";
        std::cout << "  数据大小: " << cmd.data().size() << " 字节\n";
    }

    // 创建带数据的命令
    {
        std::cout << "\n创建命令 (函数码=0x10, 带数据):\n";
        vdl::command_t cmd;
        cmd.set_function_code(0x10);
        cmd.set_data({0x00, 0x01, 0x00, 0x0A});
        
        std::cout << "  函数码: 0x" << std::hex << (int)cmd.function_code() << std::dec << "\n";
        std::cout << "  数据大小: " << cmd.data().size() << " 字节\n";
        std::cout << "  数据:\n";
        print_hex_data(cmd.data().data(), cmd.data().size());
    }
}

// ============================================================================
// 示例2: 响应的创建和操作
// ============================================================================

void example_response_creation() {
    std::cout << "示例2: 响应创建\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    vdl::response_t response;
    response.set_status(vdl::response_status_t::success);
    response.set_function_code(0x03);
    response.set_data({0x12, 0x34, 0x56, 0x78});

    std::cout << "创建响应:\n";
    std::cout << "  状态: " << (response.status() == vdl::response_status_t::success ? "成功" : "失败") << "\n";
    std::cout << "  函数码: 0x" << std::hex << (int)response.function_code() << std::dec << "\n";
    std::cout << "  数据大小: " << response.data().size() << " 字节\n";
    std::cout << "  数据:\n";
    print_hex_data(response.data().data(), response.data().size());
}

// ============================================================================
// 示例3: 二进制编码器的使用
// ============================================================================

void example_binary_codec_encode() {
    std::cout << "\n示例3: 二进制编码器 - 编码\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    vdl::binary_codec_t codec;

    // 编码简单命令
    {
        std::cout << "编码命令 (函数码=0x01, 无数据):\n";
        vdl::command_t cmd;
        cmd.set_function_code(0x01);

        auto result = codec.encode(cmd);
        if (result.has_value()) {
            auto& frame = *result;
            std::cout << "  编码成功，帧大小: " << frame.size() << " 字节\n";
            std::cout << "  帧数据:\n";
            print_hex_data(frame.data(), frame.size());
            
            // 解析帧结构
            std::cout << "  帧结构解析:\n";
            std::cout << "    SOF (字节 0): 0x" << std::hex << (int)frame[0] << std::dec << "\n";
            std::cout << "    LEN (字节 1-2): " << ((int)frame[2] << 8 | (int)frame[1]) << "\n";
            std::cout << "    FUNC (字节 3): 0x" << std::hex << (int)frame[3] << std::dec << "\n";
            std::cout << "    CRC (字节 4-5): 0x" << std::hex 
                      << ((int)frame[5] << 8 | (int)frame[4]) << std::dec << "\n";
        } else {
            std::cout << "  编码失败: " << result.error().to_string() << "\n";
        }
    }

    // 编码带数据的命令
    {
        std::cout << "\n编码命令 (函数码=0x03, 数据=5字节):\n";
        vdl::command_t cmd;
        cmd.set_function_code(0x03);
        cmd.set_data({0xAA, 0xBB, 0xCC, 0xDD, 0xEE});

        auto result = codec.encode(cmd);
        if (result.has_value()) {
            auto& frame = *result;
            std::cout << "  编码成功，帧大小: " << frame.size() << " 字节\n";
            std::cout << "  帧数据:\n";
            print_hex_data(frame.data(), frame.size());
        } else {
            std::cout << "  编码失败: " << result.error().to_string() << "\n";
        }
    }
}

// ============================================================================
// 示例4: 二进制解码器的使用
// ============================================================================

void example_binary_codec_decode() {
    std::cout << "\n示例4: 二进制编码器 - 解码\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    vdl::binary_codec_t codec;

    // 先编码，再解码
    {
        std::cout << "编码然后解码命令:\n";
        
        // 编码
        vdl::command_t cmd;
        cmd.set_function_code(0x10);
        cmd.set_data({0x01, 0x02, 0x03});

        auto encode_result = codec.encode(cmd);
        if (!encode_result.has_value()) {
            std::cout << "  编码失败\n";
            return;
        }

        auto& frame = *encode_result;
        std::cout << "  已编码帧: " << frame.size() << " 字节\n";

        // 解码
        vdl::size_t consumed = 0;
        vdl::const_byte_span_t frame_span(frame.data(), frame.size());
        auto decode_result = codec.decode(frame_span, consumed);

        if (decode_result.has_value()) {
            auto& response = *decode_result;
            std::cout << "  解码成功\n";
            std::cout << "    消耗字节: " << consumed << "\n";
            std::cout << "    函数码: 0x" << std::hex << (int)response.function_code() << std::dec << "\n";
            std::cout << "    数据大小: " << response.data().size() << " 字节\n";
            if (!response.data().empty()) {
                std::cout << "    数据:\n";
                print_hex_data(response.data().data(), response.data().size());
            }
        } else {
            std::cout << "  解码失败: " << decode_result.error().to_string() << "\n";
        }
    }
}

// ============================================================================
// 示例5: 帧检测
// ============================================================================

void example_frame_detection() {
    std::cout << "\n示例5: 帧长度检测\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    vdl::binary_codec_t codec;

    // 测试不同大小的帧头
    {
        std::cout << "检测帧长度:\n";

        // 帧头: SOF=0xAA, LEN=0x10(16), FUNC=0x01
        vdl::byte_t header[] = {0xAA, 0x10, 0x00, 0x01};
        vdl::const_byte_span_t span(header, 4);

        size_t frame_len = codec.frame_length(span);
        std::cout << "  帧头数据: 0xAA 0x10 0x00 0x01\n";
        std::cout << "  检测到的帧长度: " << frame_len << " 字节\n";
        std::cout << "  帧结构: HEADER(4) + DATA(" << (frame_len - 6) << ") + CRC(2)\n";
    }
}

// ============================================================================
// 示例6: 错误处理
// ============================================================================

void example_codec_error_handling() {
    std::cout << "\n示例6: 编解码错误处理\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    vdl::binary_codec_t codec;

    // 测试不完整的帧
    {
        std::cout << "测试不完整的帧:\n";
        vdl::byte_t incomplete[] = {0xAA, 0x05, 0x00};  // 缺少数据和CRC
        vdl::const_byte_span_t span(incomplete, 3);

        vdl::size_t consumed = 0;
        auto result = codec.decode(span, consumed);

        if (!result.has_value()) {
            std::cout << "  解码失败（预期行为）\n";
            std::cout << "  错误: " << result.error().to_string() << "\n";
        }
    }

    // 测试CRC错误
    {
        std::cout << "\n测试 CRC 错误:\n";
        
        // 先创建一个有效的帧
        vdl::command_t cmd;
        cmd.set_function_code(0x01);

        auto encode_result = codec.encode(cmd);
        if (!encode_result.has_value()) {
            std::cout << "  编码失败\n";
            return;
        }

        auto frame = *encode_result;
        // 破坏 CRC
        frame[frame.size() - 1] ^= 0xFF;

        vdl::const_byte_span_t span(frame.data(), frame.size());
        vdl::size_t consumed = 0;
        auto result = codec.decode(span, consumed);

        if (!result.has_value()) {
            std::cout << "  解码失败（预期行为）\n";
            std::cout << "  错误: " << result.error().to_string() << "\n";
        }
    }
}

// ============================================================================
// 主函数
// ============================================================================

int main() {
    std::cout << "VDL 库编解码和协议示例\n" << std::endl;

    try {
        example_command_creation();
        example_response_creation();
        example_binary_codec_encode();
        example_binary_codec_decode();
        example_frame_detection();
        example_codec_error_handling();
        
        std::cout << "\n✓ 所有示例执行完成\n" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ 异常: " << e.what() << std::endl;
        return 1;
    }
}
