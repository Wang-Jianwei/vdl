/**
 * @file 03_buffer_operations.cpp
 * @brief VDL 库缓冲区操作示例
 * 
 * 本示例展示如何使用 ring_buffer_t 进行缓冲区操作。
 * 
 * 编译: g++ -std=c++11 03_buffer_operations.cpp -I../include -o example_buffer
 */

#include <vdl/vdl.hpp>
#include <iostream>
#include <iomanip>

// ============================================================================
// 打印缓冲区信息的辅助函数
// ============================================================================

void print_buffer_status(const vdl::ring_buffer_t& buffer, const std::string& label) {
    std::cout << label << ":\n";
    std::cout << "  容量: " << buffer.capacity() << " 字节\n";
    std::cout << "  已用: " << buffer.size() << " 字节\n";
    std::cout << "  可用: " << buffer.available() << " 字节\n";
    std::cout << "  是否为空: " << (buffer.empty() ? "是" : "否") << "\n";
    std::cout << "  是否已满: " << (buffer.full() ? "是" : "否") << "\n";
}

void print_hex_data(const vdl::byte_t* data, size_t len, const std::string& label) {
    std::cout << label << ": ";
    for (size_t i = 0; i < len; ++i) {
        std::cout << "0x" << std::hex << std::setfill('0') << std::setw(2) 
                  << (int)data[i] << std::dec << " ";
    }
    std::cout << std::endl;
}

// ============================================================================
// 示例1: 基础读写操作
// ============================================================================

void example_basic_read_write() {
    std::cout << "示例1: 基础读写操作\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    vdl::ring_buffer_t buffer(16);
    print_buffer_status(buffer, "初始状态");

    // 写入数据
    {
        std::cout << "\n写入数据...\n";
        vdl::byte_t write_data[] = {0xAA, 0xBB, 0xCC, 0xDD};
        size_t written = buffer.write(write_data, 4);
        
        std::cout << "  写入了 " << written << " 字节\n";
        print_buffer_status(buffer, "写入后");
    }

    // 读取数据
    {
        std::cout << "\n读取数据...\n";
        vdl::byte_t read_data[4];
        size_t read = buffer.read(read_data, 4);
        
        std::cout << "  读取了 " << read << " 字节\n";
        print_hex_data(read_data, read, "  数据");
        print_buffer_status(buffer, "读取后");
    }
}

// ============================================================================
// 示例2: 预览数据（Peek）
// ============================================================================

void example_peek_operation() {
    std::cout << "\n示例2: 预览数据（Peek）\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    vdl::ring_buffer_t buffer(16);

    // 写入数据
    vdl::byte_t write_data[] = {0x11, 0x22, 0x33, 0x44, 0x55};
    buffer.write(write_data, 5);
    std::cout << "写入了 5 字节数据\n";
    print_buffer_status(buffer, "写入后");

    // 预览数据（不移除）
    {
        std::cout << "\n预览前 2 字节...\n";
        vdl::byte_t peek_data[2];
        size_t peeked = buffer.peek(peek_data, 2);
        
        std::cout << "  预览了 " << peeked << " 字节\n";
        print_hex_data(peek_data, peeked, "  数据");
        print_buffer_status(buffer, "预览后（无变化）");
    }

    // 再次预览（验证数据未被移除）
    {
        std::cout << "\n再次预览前 3 字节...\n";
        vdl::byte_t peek_data[3];
        size_t peeked = buffer.peek(peek_data, 3);
        
        std::cout << "  预览了 " << peeked << " 字节\n";
        print_hex_data(peek_data, peeked, "  数据");
    }

    // 跳过数据
    {
        std::cout << "\n跳过 2 字节...\n";
        size_t skipped = buffer.skip(2);
        
        std::cout << "  跳过了 " << skipped << " 字节\n";
        print_buffer_status(buffer, "跳过后");
    }
}

// ============================================================================
// 示例3: 单字节操作
// ============================================================================

void example_byte_operations() {
    std::cout << "\n示例3: 单字节操作\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    vdl::ring_buffer_t buffer(8);

    // 逐字节压入
    {
        std::cout << "逐字节压入: ";
        vdl::byte_t data[] = {0x01, 0x02, 0x03, 0x04};
        for (auto b : data) {
            if (buffer.push(b)) {
                std::cout << "0x" << std::hex << (int)b << " ";
            }
        }
        std::cout << std::dec << "\n";
        print_buffer_status(buffer, "压入后");
    }

    // 逐字节弹出
    {
        std::cout << "\n逐字节弹出: ";
        while (auto byte = buffer.pop()) {
            std::cout << "0x" << std::hex << (int)*byte << " ";
        }
        std::cout << std::dec << "\n";
        print_buffer_status(buffer, "弹出后");
    }

    // 尝试从空缓冲区弹出
    {
        std::cout << "\n尝试从空缓冲区弹出:\n";
        auto result = buffer.pop();
        
        if (result) {
            std::cout << "  获得值: 0x" << std::hex << (int)*result << std::dec << "\n";
        } else {
            std::cout << "  缓冲区为空（返回 nullopt）\n";
        }
    }
}

// ============================================================================
// 示例4: 环形缓冲区的缠绕（Wraparound）
// ============================================================================

void example_wraparound() {
    std::cout << "\n示例4: 环形缓冲区缠绕\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    vdl::ring_buffer_t buffer(8);

    // 写满缓冲区
    {
        std::cout << "写满缓冲区...\n";
        vdl::byte_t data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
        buffer.write(data, 8);
        print_buffer_status(buffer, "已满");
    }

    // 读取部分数据
    {
        std::cout << "\n读取 3 字节...\n";
        vdl::byte_t data[3];
        buffer.read(data, 3);
        print_hex_data(data, 3, "读取数据");
        print_buffer_status(buffer, "读取后");
    }

    // 写入新数据（会导致缠绕）
    {
        std::cout << "\n写入 4 字节新数据（跨越边界）...\n";
        vdl::byte_t data[] = {0xAA, 0xBB, 0xCC, 0xDD};
        size_t written = buffer.write(data, 4);
        std::cout << "  成功写入 " << written << " 字节\n";
        print_buffer_status(buffer, "写入后");
    }

    // 读取所有数据
    {
        std::cout << "\n读取所有数据...\n";
        vdl::bytes_t result(buffer.readable_size());
        if (result.size() > 0) {
            buffer.read(result.data(), result.size());
            print_hex_data(result.data(), result.size(), "数据");
        }
        print_buffer_status(buffer, "读完后");
    }
}

// ============================================================================
// 示例5: 使用 span_t 进行操作
// ============================================================================

void example_span_operations() {
    std::cout << "\n示例5: 使用 span_t 进行操作\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    vdl::ring_buffer_t buffer(16);

    // 使用 const_byte_span_t 写入
    {
        std::cout << "使用 const_byte_span_t 写入数据...\n";
        vdl::byte_t data[] = {0x10, 0x20, 0x30, 0x40};
        vdl::const_byte_span_t span(data, 4);
        
        size_t written = buffer.write(span);
        std::cout << "  写入了 " << written << " 字节\n";
    }

    // 使用 byte_span_t 读取
    {
        std::cout << "\n使用 byte_span_t 读取数据...\n";
        vdl::bytes_t read_buf(4);
        vdl::byte_span_t span(read_buf.data(), read_buf.size());
        
        size_t read = buffer.read(span);
        std::cout << "  读取了 " << read << " 字节\n";
        print_hex_data(read_buf.data(), read, "数据");
    }
}

// ============================================================================
// 示例6: 清空缓冲区
// ============================================================================

void example_clear() {
    std::cout << "\n示例6: 清空缓冲区\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    vdl::ring_buffer_t buffer(16);

    // 写入数据
    {
        vdl::byte_t data[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
        buffer.write(data, 5);
        print_buffer_status(buffer, "写入数据后");
    }

    // 清空缓冲区
    {
        std::cout << "\n清空缓冲区...\n";
        buffer.clear();
        print_buffer_status(buffer, "清空后");
    }
}

// ============================================================================
// 主函数
// ============================================================================

int main() {
    std::cout << "VDL 库缓冲区操作示例\n" << std::endl;

    try {
        example_basic_read_write();
        example_peek_operation();
        example_byte_operations();
        example_wraparound();
        example_span_operations();
        example_clear();
        
        std::cout << "\n✓ 所有示例执行完成\n" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ 异常: " << e.what() << std::endl;
        return 1;
    }
}
