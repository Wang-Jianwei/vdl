# VDL 文档目录结构

## 📂 完整的文档树

```
docs/                                    ← 📚 文档主目录
│
├── README_CN.md                         ⭐ 【必读】文档导航和学习路径
├── ORGANIZATION.md                      📋 文档整理说明（本文档）
├── INDEX.md                             📖 原始文档索引
├── PROJECT_STATUS.md                    📊 项目进度报告
├── PROJECT_COMPLETION.md                ✅ 项目完成总结
│
├── getting-started/                     🚀 快速入门 (预留目录)
│   └── (暂时为空，用于未来扩展)
│
├── architecture/                        🏗️ 架构和设计
│   ├── ARCHITECTURE_REFACTORING.md      └─ 架构重构决策说明 (10 分钟)
│   ├── ADAPTER_DESIGN_SUMMARY.md        └─ 一页纸设计概览 (5 分钟)
│   └── ADAPTER_DESIGN.md                └─ 完整设计文档 (20 分钟)
│
├── reference/                           🔍 快速参考
│   ├── ADAPTER_QUICK_REFERENCE.md       └─ 代码示例和 API 查询 (10 分钟)
│   └── CONVENIENCE_INTERFACES.md        └─ 便利方法和快捷接口 (5 分钟)
│
├── guides/                              🛠️ 指南和教程
│   └── BUILD.md                         └─ 编译和构建说明 (15 分钟)
│
├── advanced/                            🚀 高级主题
│   ├── VISA_ADAPTATION.md               └─ VISA 设备集成方案 (30 分钟)
│   └── COMPILATION_CONSISTENCY.md       └─ 编译一致性说明 (5 分钟)
│
└── examples/                            📖 示例代码和相关文档
    ├── README_CN.md                     ⭐ 【推荐】示例代码完整指南
    ├── SUMMARY.md                       └─ 所有 8 个示例快速概览 (5 分钟)
    ├── EXAMPLES_README.md               └─ 详细编译和运行说明 (10 分钟)
    ├── QUICK_REFERENCE.md               └─ 代码片段快速查找 (5 分钟)
    ├── VNA_README.md                    └─ VNA 相关示例专项说明 (10 分钟)
    └── VNA_SCPI_GUIDE.md                └─ SCPI 协议完整指南 (15 分钟)
```

---

## 📖 文档分类详解

### 🎯 根级导航文件

#### `docs/README_CN.md` ⭐ **强烈推荐首先阅读**
- **用途**: 统一的文档导航和学习路径入口
- **包含内容**:
  - 按用户类型的快速导航（新用户/开发者/架构师等）
  - 多条学习路径（新手路线/深度学习路线等）
  - 文档关系图
  - 按关键词快速查找表
- **阅读时间**: 10-15 分钟
- **何时查看**: 第一次使用时，或需要系统理解文档结构时

#### `docs/ORGANIZATION.md` (本文档)
- **用途**: 说明文档整理的背景、结构和改进效果
- **包含内容**:
  - 新旧结构对比
  - 每个文件夹的功能说明
  - 使用指南
  - 整理效果统计
- **阅读时间**: 10 分钟
- **何时查看**: 对文档组织感到疑惑时

#### `docs/INDEX.md`
- **用途**: 原始文档导航（保留供参考）
- **说明**: 这是整理前的原始导航文件
- **何时查看**: 需要原始导航方式时

---

### 📚 快速入门类

#### `docs/PROJECT_STATUS.md`
- **用途**: 项目进展、完成情况统计
- **内容**: 各模块实现状态、测试统计、已完成功能列表
- **读者**: 项目管理者、贡献者
- **阅读时间**: 5-10 分钟

#### `docs/PROJECT_COMPLETION.md`
- **用途**: 项目完成总结
- **内容**: 核心成就、文档总结
- **读者**: 所有用户
- **阅读时间**: 10-15 分钟

---

### 🏗️ 架构设计类 (docs/architecture/)

这个分类用于理解 VDL 的设计思路和架构决策。

#### `docs/architecture/ARCHITECTURE_REFACTORING.md`
- **用途**: 讲解为什么这样设计，关键架构决策
- **内容**:
  - VDL 库纯通用部分
  - SCPI 适配器为什么保留在库中
  - VNA 适配器为什么放在示例中
  - 架构分离原则
- **读者**: 架构师、高级开发者、想理解设计思路的人
- **阅读时间**: 10 分钟
- **推荐阅读顺序**: 理解架构时，**第一个读这个**

#### `docs/architecture/ADAPTER_DESIGN_SUMMARY.md`
- **用途**: 一页纸快速概览设计
- **内容**: 核心设计要点、层次架构、关键接口
- **读者**: 所有开发者（快速了解）
- **阅读时间**: 5 分钟
- **特点**: 最浓缩的设计概览

#### `docs/architecture/ADAPTER_DESIGN.md`
- **用途**: 完整详细的设计文档
- **内容**: 设计原则、适配器层次、完整实现例子
- **读者**: 实现者、需要深入理解的开发者
- **阅读时间**: 20 分钟
- **推荐**: 在理解了 ARCHITECTURE_REFACTORING 后阅读

---

### 🔍 快速参考类 (docs/reference/)

这个分类用于快速查找 API 和代码示例。

#### `docs/reference/ADAPTER_QUICK_REFERENCE.md`
- **用途**: API 快速查询、常见任务示例
- **内容**: 三层架构快速选择表、常见任务代码示例
- **读者**: 开发者（在编码时查找）
- **使用方式**: `Ctrl+F` 搜索关键词
- **阅读时间**: 随需查阅

#### `docs/reference/CONVENIENCE_INTERFACES.md`
- **用途**: 便利方法和快捷接口文档
- **内容**: Device 便捷接口、SCPI 适配器便利方法
- **读者**: 想了解高级 API 的开发者
- **阅读时间**: 5 分钟

---

### 🛠️ 构建指南类 (docs/guides/)

这个分类用于解决编译、构建、部署等问题。

#### `docs/guides/BUILD.md`
- **用途**: 编译和构建完整指南
- **内容**: 快速开始、构建脚本说明、多平台编译、CMake 使用
- **读者**: 想要编译项目的所有人
- **阅读时间**: 15 分钟
- **何时查看**: 编译遇到问题时

---

### 🚀 高级主题类 (docs/advanced/)

这个分类用于特定场景的深入解决方案。

#### `docs/advanced/VISA_ADAPTATION.md`
- **用途**: 如何使用 VDL 与 VISA 设备集成
- **内容**: VISA 基础、VDL 中的 VISA 实现、完整示例
- **读者**: 需要集成 VISA 设备的开发者
- **阅读时间**: 30 分钟
- **特点**: 最长的文档，包含完整实现细节

#### `docs/advanced/COMPILATION_CONSISTENCY.md`
- **用途**: 编译时常见问题和解决方案
- **内容**: 隐式类型转换、编译标志、跨平台兼容性
- **读者**: 遇到编译问题的开发者
- **阅读时间**: 5 分钟
- **何时查看**: 编译报错时

---

### 📖 示例代码类 (docs/examples/)

这个分类专门组织所有示例代码的说明文档。

#### `docs/examples/README_CN.md` ⭐ **示例用户必读**
- **用途**: 示例代码的完整指南
- **内容**:
  - 快速导航表
  - 按需求选择示例
  - 8 个示例的详细说明
  - 编译和运行方法
  - 学习建议
- **读者**: 想通过示例学习的所有用户
- **阅读时间**: 10-15 分钟
- **推荐**: **示例用户首先读这个**

#### `docs/examples/SUMMARY.md`
- **用途**: 所有 8 个示例的快速概览
- **内容**: 各示例的用途、学到内容、行数
- **读者**: 想了解有哪些示例的用户
- **阅读时间**: 5 分钟

#### `docs/examples/EXAMPLES_README.md`
- **用途**: 详细的编译和运行说明
- **内容**: 多种编译方法、逐个示例的编译步骤
- **读者**: 需要编译示例的开发者
- **阅读时间**: 10 分钟
- **何时查看**: 编译示例时

#### `docs/examples/QUICK_REFERENCE.md`
- **用途**: 代码片段快速查找
- **内容**: 常见任务的代码示例
- **读者**: 开发时需要快速查找代码片段
- **使用方式**: `Ctrl+F` 搜索
- **阅读时间**: 随需查阅

#### `docs/examples/VNA_README.md`
- **用途**: VNA 相关示例的专项说明
- **内容**: VNA 示例介绍、使用说明
- **读者**: 想与 VNA 设备通信的开发者
- **阅读时间**: 10 分钟

#### `docs/examples/VNA_SCPI_GUIDE.md`
- **用途**: SCPI 协议的完整指南
- **内容**: SCPI 基础、实际应用、常见命令
- **读者**: 需要理解 SCPI 协议的开发者
- **阅读时间**: 15 分钟
- **何时查看**: 学习 SCPI 或调试 VNA 通信时

---

## 🎓 推荐阅读顺序

### 新用户 (15-30 分钟)
```
1. docs/README_CN.md          (10 分钟) ← 整体导航
2. 运行 example 01             (5 分钟)
3. docs/examples/README_CN.md  (10 分钟) ← 示例指南
```

### 应用开发者 (1 小时)
```
1. docs/README_CN.md                    (10 分钟)
2. docs/architecture/ADAPTER_DESIGN_SUMMARY.md (5 分钟)
3. docs/reference/ADAPTER_QUICK_REFERENCE.md   (15 分钟)
4. 查看相关示例代码                      (30 分钟)
```

### 架构师 (1.5-2 小时)
```
1. docs/architecture/ARCHITECTURE_REFACTORING.md (10 分钟)
2. docs/architecture/ADAPTER_DESIGN.md           (20 分钟)
3. docs/architecture/ADAPTER_DESIGN_SUMMARY.md   (5 分钟)
4. 查看完整示例代码                              (60 分钟)
```

---

## 🔗 文档之间的关系

```
README.md (项目总览)
    ↓
docs/README_CN.md (文档导航中心)
    ├─→ 快速理解
    │   ├─ docs/architecture/ADAPTER_DESIGN_SUMMARY.md (5 分钟)
    │   └─ docs/examples/README_CN.md (看示例)
    │
    ├─→ 深入学习
    │   ├─ docs/architecture/ARCHITECTURE_REFACTORING.md
    │   ├─ docs/architecture/ADAPTER_DESIGN.md
    │   └─ docs/reference/ADAPTER_QUICK_REFERENCE.md
    │
    ├─→ 实战应用
    │   ├─ docs/examples/ (所有示例)
    │   └─ docs/advanced/ (特定设备集成)
    │
    └─→ 问题解决
        ├─ docs/guides/BUILD.md (编译问题)
        └─ docs/advanced/COMPILATION_CONSISTENCY.md
```

---

## 💡 使用技巧

### 快速查找
- 📱 在 VS Code 中: `Ctrl+P` + 输入文件名
- 🔍 在浏览器中: `Ctrl+F` 搜索页面内容
- 📚 从 `docs/README_CN.md` 的表格中按关键词查找

### 学习建议
1. **第一次使用**: 从 `docs/README_CN.md` 开始
2. **快速查询**: 使用 `docs/reference/ADAPTER_QUICK_REFERENCE.md`
3. **深入学习**: 按照推荐阅读顺序进行
4. **实战练习**: 运行示例并修改代码

---

## ✨ 整理后的改进

| 方面 | 整理前 | 整理后 |
|------|--------|--------|
| **结构** | 13 个 md 散落根目录 | 分类组织在 7 个目录 |
| **导航** | 无统一导航 | 新增 README_CN.md 和 examples/README_CN.md |
| **发现性** | 需要手动搜索文件 | 按用户类型推荐 |
| **可读性** | 难以快速定位 | 清晰的分类结构 |
| **扩展性** | 文件越来越多难以管理 | 预留了目录，易于扩展 |

---

## 📝 文件命名约定

### 中文文档
- `README_CN.md` - 中文版本的 README
- 其他文件名保持英文 + 可选的 `-zh` 后缀

### 文件夹命名
- 全小写 + 短横线 (`getting-started`, `architecture`, 等)

### 链接格式
- 相对路径: `[文本](../path/file.md)`
- 便于文件移动而不破坏链接

---

## 🎯 下一步行动

根据您的角色选择下一步：

- **👤 新用户**: 打开 [`docs/README_CN.md`](README_CN.md)
- **👨‍💻 开发者**: 打开 [`docs/examples/README_CN.md`](examples/README_CN.md)
- **🏗️ 架构师**: 打开 [`docs/architecture/ARCHITECTURE_REFACTORING.md`](architecture/ARCHITECTURE_REFACTORING.md)
- **🔧 需要编译**: 打开 [`docs/guides/BUILD.md`](guides/BUILD.md)

---

**文档版本**: 1.0  
**整理日期**: 2024年1月27日  
**维护者**: VDL 项目团队
