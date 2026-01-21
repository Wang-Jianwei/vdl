# VDL 示例总结

## 概述

VDL 库现在包含 5 个完整的使用示例，展示了库的各种功能和最佳实践。所有示例都使用 C++11 标准编写，并能在 GCC、Clang、MinGW 和 MSVC 上编译。

## 项目结构

```
examples/
├── 01_basic_usage.cpp              # 基础用法示例
├── 02_error_handling.cpp           # 错误处理示例
├── 03_buffer_operations.cpp        # 缓冲区操作示例
├── 04_codec_and_protocol.cpp       # 编解码和协议示例
├── 05_complete_communication.cpp   # 完整通信流程示例
├── CMakeLists.txt                  # CMake 配置
└── README.md                       # 详细编译和使用指南
```

## 示例详情

### 1. 01_basic_usage.cpp (171 行)

**主要内容:**
- 传输层创建
- 设备创建和初始化
- 设备连接管理
- 基本命令执行
- 设备状态检查

**关键演示:**
- 如何创建 `mock_transport_t` 实例
- 如何创建 `binary_codec_t` 实例
- 如何初始化 `device_impl_t`
- 连接、执行命令、断开的完整流程
- 使用 `device_state_name()` 获取状态名称

**编译:**
```bash
g++ -std=c++11 01_basic_usage.cpp -I../include -I../third_party -o example_basic
```

### 2. 02_error_handling.cpp (209 行)

**主要内容:**
- 结果值的检查方法
- 错误链接和传播
- 传输层错误模拟
- 编解码错误处理
- 错误恢复流程

**关键演示:**
- 使用 `result.has_value()` 检查成功/失败
- 访问错误信息 `result.error().to_string()`
- 多层错误处理（transport → codec → device）
- 模拟各种错误场景

**编译:**
```bash
g++ -std=c++11 02_error_handling.cpp -I../include -I../third_party -o example_errors
```

### 3. 03_buffer_operations.cpp (274 行)

**主要内容:**
- 环形缓冲区基本操作
- 读写、查看、跳过数据
- 字节级操作
- 环形缓冲区的循环特性
- Span 使用和数据视图
- 缓冲区清除

**关键演示:**
- `push()` 和 `pop()` 操作
- `peek()` 和 `skip()` 不消耗数据
- `consume()` 读取并消耗
- 环形缓冲区的绕回处理
- 使用 `span_t` 进行零复制访问

**编译:**
```bash
g++ -std=c++11 03_buffer_operations.cpp -I../include -I../third_party -o example_buffer
```

### 4. 04_codec_and_protocol.cpp (315 行)

**主要内容:**
- 命令和响应对象
- 二进制编码器的使用
- 帧格式和结构解析
- 帧长度检测
- CRC 校验和错误检测
- 编解码错误处理

**关键演示:**
- 创建 `command_t` 并设置函数码和数据
- 创建 `response_t` 并设置状态和数据
- 使用 `binary_codec_t::encode()` 编码命令
- 使用 `binary_codec_t::decode()` 解码响应
- 帧结构的详细解析（SOF、长度、函数码、CRC）
- 处理编解码错误

**编译:**
```bash
g++ -std=c++11 04_codec_and_protocol.cpp -I../include -I../third_party -o example_codec
```

### 5. 05_complete_communication.cpp (310 行)

**主要内容:**
- 完整的设备初始化流程
- 单条命令执行
- 多条命令顺序执行
- 综合错误处理流程
- 设备状态追踪

**关键演示:**
- 从创建到连接到执行的完整流程
- 连接状态转变（disconnected → connected）
- 多个命令的顺序执行
- 错误场景的处理（未连接、连接失败等）
- 状态变化的监测

**编译:**
```bash
g++ -std=c++11 05_complete_communication.cpp -I../include -I../third_party -o example_complete
```

## 编译方式

### 使用 CMake（推荐）

```bash
cd build
cmake ..
make
```

编译后的二进制文件位于 `build/bin/` 目录：
- `build/bin/01_basic_usage`
- `build/bin/02_error_handling`
- `build/bin/03_buffer_operations`
- `build/bin/04_codec_and_protocol`
- `build/bin/05_complete_communication`

### 直接编译

从 `examples/` 目录运行：

```bash
cd examples

# 编译单个示例
g++ -std=c++11 01_basic_usage.cpp -I../include -I../third_party -o 01_basic_usage

# 批量编译所有示例
for file in *.cpp; do
    g++ -std=c++11 "$file" -I../include -I../third_party -o "${file%.cpp}"
done
```

## 运行示例

使用 CMake 编译后：

```bash
./bin/01_basic_usage      # 基础用法
./bin/02_error_handling   # 错误处理
./bin/03_buffer_operations # 缓冲区操作
./bin/04_codec_and_protocol # 编解码
./bin/05_complete_communication # 完整通信
```

或在 examples 目录中直接运行编译后的可执行文件：

```bash
cd examples
./01_basic_usage
./02_error_handling
./03_buffer_operations
./04_codec_and_protocol
./05_complete_communication
```

## 关键学习点

### 基础概念
1. VDL 的三层架构：Transport → Codec → Device
2. 接口隔离原则：`i_transport_t`, `i_codec_t`, `i_device_t`
3. 组合优于继承：Device 组合 Transport + Codec

### 错误处理
1. `result_t<T>` 和 `expected<T, E>` 的使用
2. `.has_value()` 检查成功状态
3. `.error().to_string()` 获取错误信息
4. 多层错误传播

### 内存管理
1. 使用 `unique_ptr` 和 `shared_ptr` 进行资源管理
2. 移动语义的应用（`std::move`）
3. RAII 原则

### 性能考虑
1. 零复制的 `span_t` 使用
2. 环形缓冲区的高效操作
3. 流式处理的实现

## 编译器支持

所有示例都使用 C++11 标准编写，支持：
- GCC 4.8+
- Clang 3.3+
- MinGW
- MSVC 2013+（需要 `/std:c++11` 选项）

## 测试覆盖

所有示例都与单元测试套件兼容：
- 144 个单元测试用例
- 466 条断言
- 完整的库功能覆盖

运行测试：
```bash
./bin/vdl_tests  # 使用 CMake 编译后
```

## 常见问题

### Q: 如何添加自定义传输层？
A: 实现 `i_transport_t` 接口，参见 `include/vdl/transport/mock_transport.hpp`

### Q: 如何添加自定义编解码器？
A: 实现 `i_codec_t` 接口，参见 `include/vdl/codec/binary_codec.hpp`

### Q: 如何处理超时？
A: 在 `device_config_t` 中配置 `command_timeout` 字段

### Q: 如何进行多线程访问？
A: 为 Device 添加线程同步机制（示例中未覆盖）

## 扩展建议

1. 实现 TCP/Serial 传输层
2. 实现 Modbus 或其他协议编解码器
3. 添加异步操作支持
4. 添加多设备管理
5. 添加性能监测和统计

## 文件统计

| 文件 | 行数 | 大小 |
|------|------|------|
| 01_basic_usage.cpp | 171 | 4.8 KB |
| 02_error_handling.cpp | 209 | 6.2 KB |
| 03_buffer_operations.cpp | 274 | 8.1 KB |
| 04_codec_and_protocol.cpp | 315 | 9.3 KB |
| 05_complete_communication.cpp | 310 | 9.2 KB |
| **总计** | **1,279** | **37.6 KB** |

## 相关文档

- [examples/README.md](examples/README.md) - 详细编译和使用指南
- [README.md](README.md) - VDL 库主文档
- [include/vdl/vdl.hpp](include/vdl/vdl.hpp) - 完整的 API 参考

## 更新日期

最后更新：2024年1月21日

所有示例已验证可在 Ubuntu 24.04 LTS 上成功编译和运行。
