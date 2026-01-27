# VDL 示例代码完整指南

## 📚 文档导航

本目录包含 VDL 库所有示例代码和相关文档的详细说明。

### 快速导航

| 文档 | 内容 | 阅读时间 |
|------|------|--------|
| [SUMMARY.md](SUMMARY.md) | 所有8个示例的快速概览 | 5 分钟 |
| [EXAMPLES_README.md](EXAMPLES_README.md) | 详细编译和运行说明 | 10 分钟 |
| [QUICK_REFERENCE.md](QUICK_REFERENCE.md) | 代码片段快速查找 | 5 分钟 |
| [VNA_README.md](VNA_README.md) | VNA 相关示例专项说明 | 10 分钟 |
| [VNA_SCPI_GUIDE.md](VNA_SCPI_GUIDE.md) | SCPI 协议完整指南 | 15 分钟 |

## 📂 示例代码结构

### 基础示例 (01-05)
学习 VDL 库的核心概念和基本用法

```
01_basic_usage.cpp              ← 从这里开始！基础用法示例
    ↓
02_error_handling.cpp           ← 学习错误处理
    ↓
03_buffer_operations.cpp        ← 学习缓冲区操作
    ↓
04_codec_and_protocol.cpp       ← 学习编解码和协议
    ↓
05_complete_communication.cpp   ← 学习完整通信流程
```

### 进阶示例 (06-08)
学习实际设备通信和适配器设计

```
06_vna_scpi_communication.cpp   ← 实际的 VNA SCPI 通信
    ↓
07_vna_adapter_example.cpp      ← VNA 适配器设计（6个教学示例）
    ↓
08_visa_communication_example.cpp ← VISA 标准接口集成
```

## 🎯 根据您的需求选择示例

### "我想快速上手 VDL"
→ 运行 `01_basic_usage` + 阅读 [QUICK_REFERENCE.md](QUICK_REFERENCE.md)

### "我想学习错误处理"
→ 运行 `02_error_handling` 然后参考对应代码

### "我想与 VNA 通信"
→ 阅读 [VNA_README.md](VNA_README.md) + 运行 `06_vna_scpi_communication`

### "我想编写自己的设备适配器"
→ 运行 `07_vna_adapter_example` + 阅读 [ADAPTER_DESIGN.md](../architecture/ADAPTER_DESIGN.md)

### "我想集成 VISA 设备"
→ 运行 `08_visa_communication_example` + 阅读 [VISA_ADAPTATION.md](../advanced/VISA_ADAPTATION.md)

## 📋 示例详细说明

### 示例 1: 基础用法 (01_basic_usage.cpp)
- **目的**: 了解 VDL 库的基本结构
- **学到**: 如何创建设备、建立连接、执行基本命令
- **代码行数**: 171
- **编译**: `g++ -std=c++11 01_basic_usage.cpp -I../include -I../third_party -o example1`
- **运行**: `./example1`

### 示例 2: 错误处理 (02_error_handling.cpp)
- **目的**: 学习 VDL 的错误处理机制
- **学到**: 如何使用 `result_t<T>` 处理错误、错误检查最佳实践
- **代码行数**: 187
- **先决条件**: 完成示例 1

### 示例 3: 缓冲区操作 (03_buffer_operations.cpp)
- **目的**: 理解 VDL 的环形缓冲区设计
- **学到**: 缓冲区的读写、数据流管理
- **代码行数**: 215
- **先决条件**: 完成示例 1、2

### 示例 4: 编解码和协议 (04_codec_and_protocol.cpp)
- **目的**: 学习编解码层的工作原理
- **学到**: 如何定制编解码器、协议处理
- **代码行数**: 248
- **先决条件**: 完成示例 1、2、3

### 示例 5: 完整通信流程 (05_complete_communication.cpp)
- **目的**: 综合应用前 4 个示例的知识
- **学到**: 完整的设备通信流程、最佳实践
- **代码行数**: 312
- **先决条件**: 完成示例 1-4

### 示例 6: VNA SCPI 通信 (06_vna_scpi_communication.cpp)
- **目的**: 实际的 VNA 设备通信示例
- **学到**: SCPI 协议实际应用、VNA 设备控制
- **代码行数**: 421
- **特点**: 完全可运行的生产级示例
- **参考**: 阅读 [VNA_SCPI_GUIDE.md](VNA_SCPI_GUIDE.md)

### 示例 7: VNA 适配器设计 (07_vna_adapter_example.cpp)
- **目的**: 学习如何设计设备特定的适配器
- **学到**: 适配器模式、设计最佳实践
- **包含**: 6 个详细的教学示例
  1. VDL 库架构
  2. SCPI 适配器使用
  3. VNA 适配器设计
  4. 自定义设备适配器
  5. 库与用户代码分离
  6. 快速参考

### 示例 8: VISA 通信 (08_visa_communication_example.cpp)
- **目的**: 演示如何集成 VISA 设备
- **学到**: VISA 标准接口、多种传输方式支持
- **参考**: 阅读 [VISA_ADAPTATION.md](../advanced/VISA_ADAPTATION.md)

## 🚀 快速编译和运行

### 使用 CMake（推荐）

```bash
# 在项目根目录
cd build
cmake ..
make

# 运行示例
./bin/01_basic_usage
./bin/02_error_handling
# ... 其他示例
```

### 单个文件编译

```bash
cd examples
g++ -std=c++11 01_basic_usage.cpp -I../include -I../third_party -o example1
./example1
```

## 💡 学习建议

1. **按顺序学习**: 示例 01-05 是递进的，建议按顺序进行
2. **边读边练**: 阅读代码后自己修改并运行
3. **参考文档**: 每个示例都可以配合对应的文档阅读
4. **动手实践**: 基于示例修改代码，编写自己的适配器

## 📞 有问题？

查看对应的文档或查阅项目根目录的 [README.md](../README_WITH_DOCS.md)

---

**最后更新**: 2024年1月21日  
**文档版本**: 2.0
