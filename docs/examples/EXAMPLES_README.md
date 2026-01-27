# VDL 示例编译和使用指南

本目录包含 VDL 库的多个实用示例，展示了库的各种功能和使用场景。

## 目录结构

- `01_basic_usage.cpp` - 基础用法示例
- `02_error_handling.cpp` - 错误处理示例
- `03_buffer_operations.cpp` - 缓冲区操作示例
- `04_codec_and_protocol.cpp` - 编解码和协议示例
- `05_complete_communication.cpp` - 完整通信流程示例
- `06_vna_scpi_communication.cpp` - VNA 设备 SCPI 通信示例

## 编译方法

### 方法 1: 直接编译单个示例

```bash
cd examples

# 编译示例 1
g++ -std=c++11 01_basic_usage.cpp -I../include -I../third_party -o example_basic

# 编译示例 2
g++ -std=c++11 02_error_handling.cpp -I../include -I../third_party -o example_errors

# 编译示例 3
g++ -std=c++11 03_buffer_operations.cpp -I../include -I../third_party -o example_buffer

# 编译示例 4
g++ -std=c++11 04_codec_and_protocol.cpp -I../include -I../third_party -o example_codec

# 编译示例 5
g++ -std=c++11 05_complete_communication.cpp -I../include -I../third_party -o example_complete
```

### 方法 2: 使用 CMake 编译

如果项目已配置 CMake，可以使用构建系统编译示例：

```bash
cd build
cmake ..
make

# 运行示例
./bin/example_basic
./bin/example_errors
./bin/example_buffer
./bin/example_codec
./bin/example_complete
```

### 方法 3: 批量编译脚本

创建 `build_examples.sh` 脚本批量编译：

```bash
#!/bin/bash
cd examples
for file in *.cpp; do
    name="${file%.cpp}"
    echo "编译 $name ..."
    g++ -std=c++11 "$file" -I../include -o "$name"
done
```

## 运行示例

### 方法 1: 直接运行编译后的二进制

```bash
cd examples
./example_basic
./example_errors
./example_buffer
./example_codec
./example_complete
```

### 方法 2: 在不同编译器下运行

#### GCC
```bash
g++ -std=c++11 01_basic_usage.cpp -I../include -o example && ./example
```

#### Clang
```bash
clang++ -std=c++11 01_basic_usage.cpp -I../include -o example && ./example
```

#### MinGW (Windows)
```cmd
g++ -std=c++11 01_basic_usage.cpp -I../include -o example.exe && example.exe
```

## 示例说明

### 01_basic_usage.cpp - 基础用法

**学习内容:**
- 创建模拟传输层
- 初始化设备
- 执行基本连接和命令操作
- 检查设备连接状态

**主要函数:**
- `example_transport_creation()` - 创建传输层
- `example_device_creation()` - 创建设备
- `example_connection()` - 连接管理
- `example_command_execution()` - 命令执行
- `example_state_checking()` - 状态检查

### 02_error_handling.cpp - 错误处理

**学习内容:**
- 错误检查和 `result_t` 的使用
- 错误信息获取
- 错误上下文链接
- 传输层错误模拟
- 各种错误场景处理

**主要函数:**
- `example_result_checking()` - 结果检查
- `example_error_chaining()` - 错误链接
- `example_transport_errors()` - 传输错误
- `example_codec_errors()` - 编解码错误
- `example_recovery()` - 错误恢复

### 03_buffer_operations.cpp - 缓冲区操作

**学习内容:**
- 缓冲区基本操作（读写、查看、跳过）
- 环形缓冲区的循环特性
- Span 和数据视图
- 缓冲区清除
- 二进制数据操作

**主要函数:**
- `example_basic_operations()` - 基本操作
- `example_peek_and_skip()` - 查看和跳过
- `example_byte_operations()` - 字节操作
- `example_wraparound()` - 环形缓冲区循环
- `example_span_operations()` - Span 操作
- `example_clear()` - 缓冲区清除

### 04_codec_and_protocol.cpp - 编解码和协议

**学习内容:**
- 命令和响应对象的创建
- 二进制编码器的编码和解码功能
- 帧格式和结构
- 帧长度检测
- CRC 和校验错误处理

**主要函数:**
- `example_command_creation()` - 命令创建
- `example_response_creation()` - 响应创建
- `example_binary_codec_encode()` - 编码
- `example_binary_codec_decode()` - 解码
- `example_frame_detection()` - 帧检测
- `example_codec_error_handling()` - 错误处理

### 05_complete_communication.cpp - 完整通信

**学习内容:**
- 设备初始化和连接流程
- 单条和多条命令的执行
- 完整的错误处理流程
- 设备状态追踪

**主要函数:**
- `example_device_initialization()` - 设备初始化
- `example_single_command_execution()` - 单条命令
- `example_sequential_commands()` - 多条命令
- `example_error_handling_flow()` - 错误处理
- `example_device_state_tracking()` - 状态追踪

## 编译器兼容性

所有示例都使用 C++11 标准编写，能够在以下编译器上编译和运行：

- GCC 4.8+ (`g++`)
- Clang 3.3+ (`clang++`)
- MinGW (Windows)
- MSVC 2013+ (需要使用 `/std:c++11` 或更高)

### 编译器特定的编译命令

**GCC:**
```bash
g++ -std=c++11 example.cpp -I../include -I../third_party -o example
```

**Clang:**
```bash
clang++ -std=c++11 example.cpp -I../include -I../third_party -o example
```

**MinGW:**
```bash
mingw32-g++ -std=c++11 example.cpp -I../include -I../third_party -o example.exe
```

**MSVC:**
```bash
cl /std:c++11 example.cpp /I../include /I../third_party
```

## 运行时检查

### 内存检查 (使用 valgrind)

```bash
g++ -std=c++11 -g 01_basic_usage.cpp -I../include -o example
valgrind ./example
```

### 静态代码分析 (使用 clang-tidy)

```bash
clang-tidy --header-filter=.* 01_basic_usage.cpp -- -I../include
```

## 故障排除

### 问题 1: 找不到头文件

**症状:** `fatal error: vdl/vdl.hpp: No such file or directory` 或 `fatal error: tl/expected.hpp: No such file or directory`

**解决方案:** 确保使用了 `-I../include` 和 `-I../third_party` 选项（从 examples 目录编译时）

```bash
g++ -std=c++11 01_basic_usage.cpp -I../include -I../third_party -o example
```

### 问题 2: C++ 标准不兼容

**症状:** 编译错误，涉及 C++11 特性

**解决方案:** 使用 `-std=c++11` 编译选项

```bash
g++ -std=c++11 01_basic_usage.cpp -I../include -o example
```

### 问题 3: MinGW 编译失败

**症状:** MinGW 编译错误

**解决方案:** 使用 MinGW 特定的编译器和选项

```bash
mingw32-g++ -std=c++11 01_basic_usage.cpp -I../include -o example.exe
```

## 扩展示例

可以通过以下方式扩展这些示例：

1. **自定义传输层** - 实现 TCP 或 Serial 传输
2. **自定义编解码器** - 实现不同的协议格式
3. **多线程** - 添加线程安全的命令发送
4. **循环测试** - 在循环中执行命令
5. **性能测试** - 测量吞吐量和延迟

### 06_vna_scpi_communication.cpp - VNA 设备 SCPI 通信

本示例展示如何使用 VDL 库通过 TCP/IP 与 VNA 设备进行 SCPI 通信。

**功能特性:**
- 自定义 TCP 传输层实现
- SCPI 协议编解码器
- VNA 设备连接和识别
- 频率设置和查询
- 扫描参数配置
- S 参数测量
- 错误处理和检查

**编译:**
```bash
# Linux/macOS
g++ -std=c++11 06_vna_scpi_communication.cpp -I../include -I../third_party -o vna_scpi_example

# Windows (MinGW)
g++ -std=c++11 06_vna_scpi_communication.cpp -I../include -I../third_party -lws2_32 -o vna_scpi_example.exe
```

**运行:**
```bash
# 使用默认 IP 和端口 (192.168.1.100:5025)
./vna_scpi_example

# 指定 VNA IP 地址和端口
./vna_scpi_example 192.168.1.100 5025
```

**示例包含的测试场景:**
1. 设备识别查询 (*IDN?)
2. 频率范围设置 (起始/终止频率)
3. 扫描参数配置 (点数、带宽)
4. S 参数测量
5. 错误队列检查

**SCPI 命令示例:**
```cpp
// 识别查询
auto cmd = make_scpi_command("*IDN?");

// 设置频率
auto cmd = make_scpi_command("SENS:FREQ:STAR 1E9");

// 查询数据
auto cmd = make_scpi_command("CALC:DATA? FDAT");
```

**注意事项:**
- 确保 VNA 设备已开启 SCPI 远程控制功能
- 默认 SCPI 端口通常是 5025
- 根据实际设备调整 IP 地址
- 某些 VNA 可能需要特定的命令语法，请参考设备手册

## 示例输出

每个示例在执行时会输出详细的执行过程，例如：

```
VDL 库基础用法示例

示例1: 传输层创建
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

创建模拟传输层...
✓ 传输层创建成功

...

✓ 所有示例执行完成
```

## 学习路径建议

1. **初学者** → `01_basic_usage.cpp` → `05_complete_communication.cpp`
2. **中级用户** → `02_error_handling.cpp` → `04_codec_and_protocol.cpp`
3. **高级用户** → `03_buffer_operations.cpp` → 自定义传输层实现

## 更多信息

- 查看 `../README.md` 了解库的总体架构
- 查看 `../include/vdl/` 了解 API 详细信息
- 查看 `../tests/` 了解单元测试示例

## 许可证

这些示例代码遵循与 VDL 库相同的许可证条款。
