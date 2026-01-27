# 📚 VDL 文档完整导航

**欢迎使用 VDL 虚拟设备层库！** 本文档帮助您快速找到所需的内容。

---

## 🎯 按用户类型选择

### 👤 我是库的新用户
**时间**: 15 分钟 | **路径**:
1. 阅读 [README.md](../README.md) (5 分钟) - 项目概览
2. 运行 [example 01](examples/SUMMARY.md) (5 分钟) - 看看代码如何工作
3. 浏览 [快速参考](reference/ADAPTER_QUICK_REFERENCE.md) (5 分钟) - 了解主要 API

👉 **开始**: 打开 [examples/README_CN.md](examples/README_CN.md)

### 👨‍💻 我是应用开发者，想快速使用 VDL
**时间**: 30 分钟 | **路径**:
1. 阅读 [架构概览](architecture/ADAPTER_DESIGN_SUMMARY.md) (5 分钟)
2. 查看 [快速参考示例](reference/ADAPTER_QUICK_REFERENCE.md) (10 分钟)
3. 复制示例代码并修改使用

👉 **开始**: 打开 [reference/ADAPTER_QUICK_REFERENCE.md](reference/ADAPTER_QUICK_REFERENCE.md)

### 🏗️ 我是架构师/高级开发者，想了解设计决策
**时间**: 1 小时 | **路径**:
1. 阅读 [架构重构说明](architecture/ARCHITECTURE_REFACTORING.md) (10 分钟)
2. 深入 [适配器设计文档](architecture/ADAPTER_DESIGN.md) (20 分钟)
3. 查看 [设计总结](architecture/ADAPTER_DESIGN_SUMMARY.md) (5 分钟)
4. 查阅项目状态 [PROJECT_STATUS.md](PROJECT_STATUS.md) (5 分钟)

👉 **开始**: 打开 [architecture/ARCHITECTURE_REFACTORING.md](architecture/ARCHITECTURE_REFACTORING.md)

### 🔧 我想与特定设备集成 (VNA/VISA/等)
**时间**: 1-2 小时 | **路径**:
1. 查看相关的示例代码 [docs/examples](examples/)
2. 阅读对应的指南:
   - VNA 设备: [examples/VNA_README.md](examples/VNA_README.md)
   - VISA 设备: [advanced/VISA_ADAPTATION.md](advanced/VISA_ADAPTATION.md)
3. 学习适配器设计: [architecture/ADAPTER_DESIGN.md](architecture/ADAPTER_DESIGN.md)
4. 编写您的自定义适配器

👉 **开始**: 选择您的设备类型，然后查看对应的示例

### 👨‍🏫 我想学习如何为自己的设备编写适配器
**时间**: 2 小时+ | **路径**:
1. 运行示例 07 查看完整的设计示例
2. 阅读 [ADAPTER_DESIGN.md](architecture/ADAPTER_DESIGN.md)
3. 参考 [ADAPTER_QUICK_REFERENCE.md](reference/ADAPTER_QUICK_REFERENCE.md)
4. 学习 [便利接口](reference/CONVENIENCE_INTERFACES.md)
5. 查看 examples 目录中的 `vna_adapter.hpp` 作为参考实现

👉 **开始**: 打开 [examples/SUMMARY.md](examples/SUMMARY.md) 然后查看示例 07

### 📖 我想了解编译和构建系统
**时间**: 15 分钟 | **路径**:
1. 阅读 [BUILD.md](guides/BUILD.md)
2. 查看编译一致性说明 [COMPILATION_CONSISTENCY.md](advanced/COMPILATION_CONSISTENCY.md)

👉 **开始**: 打开 [guides/BUILD.md](guides/BUILD.md)

---

## 📋 文档完整列表

### 🚀 快速入门 (getting-started)
| 文档 | 用途 | 阅读时间 |
|------|------|--------|
| [INDEX.md](INDEX.md) | 📚 文档索引（原文档导航） | 5 分钟 |
| [PROJECT_STATUS.md](PROJECT_STATUS.md) | 📊 项目进度和状态 | 5 分钟 |
| [PROJECT_COMPLETION.md](PROJECT_COMPLETION.md) | ✅ 项目完成总结 | 10 分钟 |

### 🏛️ 架构设计 (architecture)
| 文档 | 用途 | 阅读时间 |
|------|------|--------|
| [ARCHITECTURE_REFACTORING.md](architecture/ARCHITECTURE_REFACTORING.md) | 🔄 架构重构决策说明 | 10 分钟 |
| [ADAPTER_DESIGN_SUMMARY.md](architecture/ADAPTER_DESIGN_SUMMARY.md) | 📝 一页纸设计概览 | 5 分钟 |
| [ADAPTER_DESIGN.md](architecture/ADAPTER_DESIGN.md) | 📖 完整设计文档 | 20 分钟 |

### 🔍 快速参考 (reference)
| 文档 | 用途 | 阅读时间 |
|------|------|--------|
| [ADAPTER_QUICK_REFERENCE.md](reference/ADAPTER_QUICK_REFERENCE.md) | ⚡ 代码示例和 API 查询 | 10 分钟 |
| [CONVENIENCE_INTERFACES.md](reference/CONVENIENCE_INTERFACES.md) | 🎁 便利方法和快捷接口 | 5 分钟 |

### 🛠️ 构建指南 (guides)
| 文档 | 用途 | 阅读时间 |
|------|------|--------|
| [BUILD.md](guides/BUILD.md) | 🔨 编译和构建说明 | 15 分钟 |

### 📚 示例代码 (examples)
| 文档 | 用途 | 阅读时间 |
|------|------|--------|
| [README_CN.md](examples/README_CN.md) | 📖 **[推荐] 示例代码完整指南** | 10 分钟 |
| [SUMMARY.md](examples/SUMMARY.md) | 📋 所有8个示例快速概览 | 5 分钟 |
| [EXAMPLES_README.md](examples/EXAMPLES_README.md) | 🔧 详细编译和运行说明 | 10 分钟 |
| [QUICK_REFERENCE.md](examples/QUICK_REFERENCE.md) | ⚡ 代码片段快速查找 | 5 分钟 |
| [VNA_README.md](examples/VNA_README.md) | 🎛️ VNA 相关示例专项说明 | 10 分钟 |
| [VNA_SCPI_GUIDE.md](examples/VNA_SCPI_GUIDE.md) | 📡 SCPI 协议完整指南 | 15 分钟 |

### 🚀 高级主题 (advanced)
| 文档 | 用途 | 阅读时间 |
|------|------|--------|
| [VISA_ADAPTATION.md](advanced/VISA_ADAPTATION.md) | 🔗 VISA 设备集成方案 | 30 分钟 |
| [COMPILATION_CONSISTENCY.md](advanced/COMPILATION_CONSISTENCY.md) | ⚙️ 编译一致性说明 | 5 分钟 |

---

## 🎓 学习路径

### 新手学习流程 (总耗时: ~1 小时)

```
开始
  ↓
阅读 README.md (5 分钟)
  ↓
运行示例 01 (5 分钟)
  ↓
浏览快速参考 (5 分钟)
  ↓
运行示例 02-05 (20 分钟)
  ↓
阅读架构概览 (10 分钟)
  ↓
选择特定设备示例 (10 分钟)
  ↓
准备编写您的适配器
```

### 深度学习流程 (总耗时: ~3 小时)

```
项目概览
  ↓
架构重构说明 → 架构设计文档 → 设计总结
  ↓
完整示例代码学习 (01-08)
  ↓
设计模式和最佳实践
  ↓
特定设备集成 (VNA/VISA/等)
  ↓
编写自己的适配器
```

---

## 🔗 文档间关系图

```
README.md (总览)
    ↓
├─→ [快速入门] PROJECT_STATUS.md → 运行 examples/01
    ↓
├─→ [架构理解] ARCHITECTURE_REFACTORING.md
    │          ↓
    │          ADAPTER_DESIGN_SUMMARY.md (一页纸)
    │          ↓
    │          ADAPTER_DESIGN.md (深入)
    ↓
├─→ [代码示例] examples/README_CN.md
    │          ↓
    │          选择示例 (01-08)
    ↓
├─→ [快速参考] ADAPTER_QUICK_REFERENCE.md
    │          ↓
    │          CONVENIENCE_INTERFACES.md
    ↓
└─→ [特殊需求] VISA_ADAPTATION.md 或其他高级主题
```

---

## 🔍 按关键词快速查找

| 关键词 | 相关文档 |
|--------|--------|
| **快速上手** | [examples/README_CN.md](examples/README_CN.md) |
| **API 查询** | [reference/ADAPTER_QUICK_REFERENCE.md](reference/ADAPTER_QUICK_REFERENCE.md) |
| **VNA 设备** | [examples/VNA_README.md](examples/VNA_README.md) + [examples/VNA_SCPI_GUIDE.md](examples/VNA_SCPI_GUIDE.md) |
| **VISA 集成** | [advanced/VISA_ADAPTATION.md](advanced/VISA_ADAPTATION.md) |
| **设计决策** | [architecture/ARCHITECTURE_REFACTORING.md](architecture/ARCHITECTURE_REFACTORING.md) |
| **编译问题** | [guides/BUILD.md](guides/BUILD.md) + [advanced/COMPILATION_CONSISTENCY.md](advanced/COMPILATION_CONSISTENCY.md) |
| **错误处理** | [examples/SUMMARY.md](examples/SUMMARY.md) → 示例 02 |
| **缓冲区** | [examples/SUMMARY.md](examples/SUMMARY.md) → 示例 03 |
| **协议适配** | [architecture/ADAPTER_DESIGN.md](architecture/ADAPTER_DESIGN.md) |
| **便利接口** | [reference/CONVENIENCE_INTERFACES.md](reference/CONVENIENCE_INTERFACES.md) |

---

## 💡 快速技巧

- **5 分钟了解 VDL**: 读 [ADAPTER_DESIGN_SUMMARY.md](architecture/ADAPTER_DESIGN_SUMMARY.md)
- **快速查看代码示例**: 查 [ADAPTER_QUICK_REFERENCE.md](reference/ADAPTER_QUICK_REFERENCE.md)
- **理解为什么这样设计**: 读 [ARCHITECTURE_REFACTORING.md](architecture/ARCHITECTURE_REFACTORING.md)
- **与 VNA 通信**: 先看 [examples/VNA_README.md](examples/VNA_README.md)
- **与 VISA 集成**: 先看 [advanced/VISA_ADAPTATION.md](advanced/VISA_ADAPTATION.md)

---

## 📞 需要帮助？

1. **查找特定 API**: 使用上面的表格或 `Ctrl+F` 搜索
2. **理解架构**: 从 [ARCHITECTURE_REFACTORING.md](architecture/ARCHITECTURE_REFACTORING.md) 开始
3. **运行代码**: 查看 [examples/README_CN.md](examples/README_CN.md)
4. **解决问题**: 查看 [guides/BUILD.md](guides/BUILD.md)

---

**最后更新**: 2024年1月27日  
**文档版本**: 3.0 (整理版)
