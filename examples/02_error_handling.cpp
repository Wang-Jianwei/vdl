/**
 * @file 02_error_handling.cpp
 * @brief VDL 库错误处理示例
 * 
 * 本示例展示如何处理 VDL 库中的错误情况。
 * 
 * 编译: g++ -std=c++11 02_error_handling.cpp -I../include -o example_error
 */

#include <vdl/vdl.hpp>
#include <iostream>

// ============================================================================
// 错误处理辅助函数
// ============================================================================

/**
 * @brief 打印错误信息
 */
void print_error(const vdl::error_t& error) {
    std::cout << "❌ 错误\n";
    std::cout << "  代码: " << static_cast<int>(error.code()) << "\n";
    std::cout << "  名称: " << vdl::get_error_name(error.code()) << "\n";
    std::cout << "  消息: " << error.message() << "\n";
    std::cout << "  类别: ";
    
    switch (error.category()) {
        case vdl::error_category_t::none: std::cout << "无"; break;
        case vdl::error_category_t::general: std::cout << "一般"; break;
        case vdl::error_category_t::memory: std::cout << "内存"; break;
        case vdl::error_category_t::argument: std::cout << "参数"; break;
        case vdl::error_category_t::io: std::cout << "I/O"; break;
        case vdl::error_category_t::device: std::cout << "设备"; break;
        case vdl::error_category_t::concurrency: std::cout << "并发"; break;
        case vdl::error_category_t::protocol: std::cout << "协议"; break;
        case vdl::error_category_t::config: std::cout << "配置"; break;
        case vdl::error_category_t::logic: std::cout << "逻辑"; break;
        case vdl::error_category_t::transport: std::cout << "传输"; break;
    }
    std::cout << std::endl;
}

// ============================================================================
// 示例1: 基础错误检查
// ============================================================================

void example_basic_error_checking() {
    std::cout << "\n示例1: 基础错误检查\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";

    vdl::error_t ok_error;
    vdl::error_t timeout_error(vdl::error_code_t::timeout, "Operation timed out");
    
    std::cout << "检查 ok_error:\n";
    std::cout << "  is_ok(): " << (ok_error.is_ok() ? "true" : "false") << "\n";
    std::cout << "  is_error(): " << (ok_error.is_error() ? "true" : "false") << "\n";
    std::cout << "  operator bool(): " << (static_cast<bool>(ok_error) ? "true" : "false") << "\n";
    
    std::cout << "\n检查 timeout_error:\n";
    std::cout << "  is_ok(): " << (timeout_error.is_ok() ? "true" : "false") << "\n";
    std::cout << "  is_error(): " << (timeout_error.is_error() ? "true" : "false") << "\n";
    std::cout << "  operator bool(): " << (static_cast<bool>(timeout_error) ? "true" : "false") << "\n";
    print_error(timeout_error);
}

// ============================================================================
// 示例2: Result 类型处理
// ============================================================================

vdl::result_t<int> safe_divide(int a, int b) {
    if (b == 0) {
        return vdl::make_error<int>(vdl::error_code_t::invalid_argument,
                                     "Division by zero");
    }
    return a / b;
}

void example_result_handling() {
    std::cout << "\n示例2: Result 类型处理\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";

    // 成功情况
    {
        std::cout << "计算 10 ÷ 2:\n";
        auto result = safe_divide(10, 2);
        
        if (result.has_value()) {
            std::cout << "  ✓ 结果: " << *result << "\n";
        } else {
            print_error(result.error());
        }
    }

    // 失败情况
    {
        std::cout << "\n计算 10 ÷ 0:\n";
        auto result = safe_divide(10, 0);
        
        if (result.has_value()) {
            std::cout << "  ✓ 结果: " << *result << "\n";
        } else {
            print_error(result.error());
        }
    }

    // 使用 value_or
    {
        std::cout << "\n使用 value_or():\n";
        auto result1 = safe_divide(10, 2);
        auto result2 = safe_divide(10, 0);
        
        std::cout << "  10 ÷ 2 = " << result1.value_or(-1) << "\n";
        std::cout << "  10 ÷ 0 = " << result2.value_or(-1) << "\n";
    }
}

// ============================================================================
// 示例3: 错误链（Error Chaining）
// ============================================================================

void example_error_context() {
    std::cout << "\n示例3: 错误上下文\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";

    vdl::error_t error(vdl::error_code_t::read_error, "Failed to read data");
    error.with_context("in file handler")
         .with_context("at line 42");
    
    std::cout << "错误信息:\n";
    std::cout << "  " << error.to_string() << "\n";
    std::cout << "\n错误详情:\n";
    std::cout << "  代码: " << static_cast<int>(error.code()) << "\n";
    std::cout << "  消息: " << error.message() << "\n";
    std::cout << "  上下文: " << error.context() << "\n";
}

// ============================================================================
// 示例4: 传输层错误
// ============================================================================

void example_transport_errors() {
    std::cout << "\n示例4: 传输层错误处理\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";

    auto transport = std::make_shared<vdl::mock_transport_t>();

    // 测试连接失败
    {
        std::cout << "测试连接失败:\n";
        transport->set_fail_open(true);
        
        auto result = transport->open();
        if (!result) {
            print_error(result.error());
        }
        transport->set_fail_open(false);
    }

    // 测试读取失败
    {
        std::cout << "\n测试读取失败:\n";
        transport->open();
        transport->set_response({0x01, 0x02});
        transport->set_fail_read(true);
        
        vdl::bytes_t buffer(10);
        vdl::byte_span_t span(buffer.data(), buffer.size());
        auto result = transport->read(span);
        
        if (!result) {
            print_error(result.error());
        }
        transport->set_fail_read(false);
    }

    // 测试写入失败
    {
        std::cout << "\n测试写入失败:\n";
        transport->set_fail_write(true);
        
        vdl::byte_t data[] = {0x01, 0x02};
        vdl::const_byte_span_t span(data, 2);
        auto result = transport->write(span);
        
        if (!result) {
            print_error(result.error());
        }
    }

    transport->close();
}

// ============================================================================
// 主函数
// ============================================================================

int main() {
    std::cout << "VDL 库错误处理示例\n" << std::endl;

    try {
        example_basic_error_checking();
        example_result_handling();
        example_error_context();
        example_transport_errors();
        
        std::cout << "\n✓ 所有示例执行完成\n" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ 异常: " << e.what() << std::endl;
        return 1;
    }
}
