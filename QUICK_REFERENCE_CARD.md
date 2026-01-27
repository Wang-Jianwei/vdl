# 📚 VDL 文档快速参考卡

## 🎯 我想要... → 打开这个

| 您的需求 | 推荐文档 | 时间 | 链接 |
|---------|--------|------|------|
| **快速了解VDL** | docs/README_CN.md | 10 min | [打开](docs/README_CN.md) |
| **学习示例代码** | docs/examples/README_CN.md | 15 min | [打开](docs/examples/README_CN.md) |
| **理解架构设计** | docs/architecture/ARCHITECTURE_REFACTORING.md | 10 min | [打开](docs/architecture/ARCHITECTURE_REFACTORING.md) |
| **查询API用法** | docs/reference/ADAPTER_QUICK_REFERENCE.md | 查询 | [打开](docs/reference/ADAPTER_QUICK_REFERENCE.md) |
| **编译项目** | docs/guides/BUILD.md | 15 min | [打开](docs/guides/BUILD.md) |
| **与VNA通信** | docs/examples/VNA_README.md | 10 min | [打开](docs/examples/VNA_README.md) |
| **与VISA集成** | docs/advanced/VISA_ADAPTATION.md | 30 min | [打开](docs/advanced/VISA_ADAPTATION.md) |
| **查看文档结构** | docs/STRUCTURE.md | 10 min | [打开](docs/STRUCTURE.md) |
| **了解整理说明** | docs/ORGANIZATION.md | 10 min | [打开](docs/ORGANIZATION.md) |
| **看项目进度** | docs/PROJECT_STATUS.md | 5 min | [打开](docs/PROJECT_STATUS.md) |

---

## 🚀 按用户角色快速导航

### 👤 新用户 (15 分钟)
```
1. 打开根目录 README.md (了解项目)
   ↓
2. 打开 docs/README_CN.md (了解文档组织)
   ↓
3. 打开 docs/examples/README_CN.md (选择示例)
   ↓
4. 运行第一个示例
```

### 👨‍💻 应用开发者 (1 小时)
```
打开 docs/README_CN.md 并选择"应用开发者"路径
  ↓
docs/architecture/ADAPTER_DESIGN_SUMMARY.md (5 分钟)
  ↓
docs/reference/ADAPTER_QUICK_REFERENCE.md (15 分钟)
  ↓
选择相关示例代码学习 (30 分钟)
```

### 🏗️ 架构师 (1-2 小时)
```
docs/architecture/ARCHITECTURE_REFACTORING.md (10 分钟)
  ↓
docs/architecture/ADAPTER_DESIGN.md (20 分钟)
  ↓
docs/architecture/ADAPTER_DESIGN_SUMMARY.md (5 分钟对比)
  ↓
查看完整示例代码 (60+ 分钟)
```

### 🛠️ 需要与特定设备通信 (1-2 小时)
```
打开 docs/README_CN.md 并选择"特定设备集成"路径
  ↓
选择您的设备类型：
  • VNA → docs/examples/VNA_README.md
  • VISA → docs/advanced/VISA_ADAPTATION.md
  • 其他 → docs/architecture/ADAPTER_DESIGN.md
```

---

## 📂 文档目录地图

```
docs/
├── README_CN.md              ⭐ 主入口 - 从这里开始!
├── STRUCTURE.md              📋 文档结构说明
├── ORGANIZATION.md           📋 整理详解
├── PROJECT_STATUS.md         📊 项目进度
├── PROJECT_COMPLETION.md     ✅ 项目总结
│
├── architecture/
│   ├── ARCHITECTURE_REFACTORING.md    🔄 为什么这样设计
│   ├── ADAPTER_DESIGN_SUMMARY.md      📝 一页纸概览
│   └── ADAPTER_DESIGN.md              📖 完整设计
│
├── reference/
│   ├── ADAPTER_QUICK_REFERENCE.md     ⚡ API速查
│   └── CONVENIENCE_INTERFACES.md      🎁 便利接口
│
├── guides/
│   └── BUILD.md                       🔨 编译指南
│
├── advanced/
│   ├── VISA_ADAPTATION.md             🔗 VISA集成
│   └── COMPILATION_CONSISTENCY.md     ⚙️ 编译问题
│
└── examples/
    ├── README_CN.md                   ⭐ 示例指南
    ├── SUMMARY.md                     📋 快速概览
    ├── EXAMPLES_README.md             🔧 编译说明
    ├── QUICK_REFERENCE.md             ⚡ 代码片段
    ├── VNA_README.md                  🎛️ VNA说明
    └── VNA_SCPI_GUIDE.md              📡 SCPI指南
```

---

## ⌨️ 快速命令参考

### 打开主导航
```bash
# 在 VS Code 中
code docs/README_CN.md

# 或用浏览器打开 Markdown 预览
```

### 快速查找文档
```bash
# 在 VS Code 中使用 Ctrl+P 快速打开
Ctrl+P → 输入 "README_CN"

# 或用命令行查找
find docs -name "*.md" | grep -i <关键词>
```

### 编译示例
```bash
cd build
cmake ..
make -j$(nproc)
./bin/01_basic_usage
```

---

## 🔍 按关键词快速查找

| 关键词 | 查看这个 |
|--------|---------|
| 快速上手 | docs/README_CN.md |
| 三层架构 | docs/architecture/ADAPTER_DESIGN_SUMMARY.md |
| 设计决策 | docs/architecture/ARCHITECTURE_REFACTORING.md |
| API示例 | docs/reference/ADAPTER_QUICK_REFERENCE.md |
| 错误处理 | docs/examples/ (示例 02) |
| 缓冲区 | docs/examples/ (示例 03) |
| 编解码 | docs/examples/ (示例 04) |
| SCPI | docs/examples/VNA_SCPI_GUIDE.md |
| VNA | docs/examples/VNA_README.md |
| VISA | docs/advanced/VISA_ADAPTATION.md |
| 编译问题 | docs/advanced/COMPILATION_CONSISTENCY.md |

---

## 💡 贴心提示

### 首次使用
1. ✅ 打开 [docs/README_CN.md](docs/README_CN.md)
2. ✅ 找到适合您的路径
3. ✅ 按顺序阅读
4. ✅ 运行相关示例

### 快速查询
- 💻 使用 `Ctrl+F` 在文档中搜索
- 📱 使用 `Ctrl+P` 在 VS Code 中快速打开文件
- 🔍 使用 grep 命令行工具

### 遇到问题
1. 查看对应分类的文档
2. 运行相关示例
3. 查看 BUILD.md 解决编译问题
4. 查看 COMPILATION_CONSISTENCY.md 解决编译不一致

---

## 📊 整理成果一览

✨ **从混乱到有序**

| 指标 | 整理前 | 整理后 |
|------|--------|--------|
| 文档数量 | 13 个 | 20 个 (含导航) |
| 组织方式 | 散乱 | 7 个分类 |
| 入口 | 无统一入口 | 3 个导航文件 |
| 导航能力 | 手动搜索 | 自动推荐 |
| 用户体验 | 困惑 | 清晰 |

---

## 🎓 学习路径总结

### 路径 1: 快速体验 (15 分钟)
```
README.md → docs/README_CN.md → 运行示例01
```

### 路径 2: 深入学习 (2-3 小时)
```
README.md 
  → docs/README_CN.md
  → docs/architecture/ (全部读)
  → docs/examples/ (选择相关的)
  → 修改示例代码尝试
```

### 路径 3: 完全掌握 (半天)
```
所有文档逐个阅读
  + 所有示例全部运行
  + 自己编写适配器
```

---

## 🔗 重要链接速记

| 文件 | 快捷链接 |
|------|---------|
| 主导航 | [docs/README_CN.md](docs/README_CN.md) |
| 示例指南 | [docs/examples/README_CN.md](docs/examples/README_CN.md) |
| 架构设计 | [docs/architecture/ARCHITECTURE_REFACTORING.md](docs/architecture/ARCHITECTURE_REFACTORING.md) |
| API参考 | [docs/reference/ADAPTER_QUICK_REFERENCE.md](docs/reference/ADAPTER_QUICK_REFERENCE.md) |
| 构建指南 | [docs/guides/BUILD.md](docs/guides/BUILD.md) |

---

## 🎉 开始使用

### 现在就打开:
👉 **[docs/README_CN.md](docs/README_CN.md)**

### 或根据您的角色选择:
- 👤 新用户 → 按"新用户"部分
- 👨‍💻 开发者 → 按"应用开发者"部分  
- 🏗️ 架构师 → 按"架构师"部分
- 🛠️ 需要特定功能 → 按"特定需求"部分

---

**最后更新**: 2024年1月27日  
**版本**: 1.0  
**用途**: 快速参考卡（可打印或书签）
