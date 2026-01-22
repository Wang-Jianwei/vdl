/**
 * @file 07_vna_adapter_example.cpp
 * @brief VNA 设备适配器架构和最佳实践
 * 
 * 本示例演示了：
 * 1. VDL 库的架构（通用 SCPI 适配器）
 * 2. 用户自定义的设备特定适配器（VNA 适配器）
 * 3. 如何为您的设备编写适配器
 * 4. 库与用户代码的分离原则
 * 
 * 关键设计原则：
 * • scpi_adapter_t 是 VDL 库的一部分 - 通用 SCPI 支持
 * • vna_adapter_t 是用户示例代码 - VNA 设备特定实现
 * • 您应该为自己的设备编写类似的适配器
 * • 库保持通用和可维护，用户编写特定设备逻辑
 */

#include <iostream>
#include <string>
#include <vector>

// ============================================================================
// 输出工具
// ============================================================================

void print_section(const std::string& title) {
    std::cout << "\n";
    std::cout << "=" << std::string(58, '=') << "\n";
    std::cout << "  " << title << "\n";
    std::cout << "=" << std::string(58, '=') << "\n";
}

void print_subsection(const std::string& title) {
    std::cout << "\n" << title << "\n";
    std::cout << std::string(title.length(), '-') << "\n";
}

// ============================================================================
// 示例 1: VDL 库架构
// ============================================================================

void example_architecture() {
    print_section("示例 1: VDL 库架构");

    std::cout << "\nVDL 库的分层设计\n\n";

    std::cout << "层级 1 - 基础接口 (include/vdl/)\n";
    std::cout << "  * transport_base_t - 传输层抽象接口\n";
    std::cout << "    - 定义网络通信的接口\n";
    std::cout << "    - 支持 TCP、UDP、串口等实现\n\n";

    std::cout << "  * codec_base_t - 编解码层抽象接口\n";
    std::cout << "    - 定义协议编解码的接口\n";
    std::cout << "    - 支持 SCPI、ModBus 等实现\n\n";

    std::cout << "  * device_impl_t - 设备实现\n";
    std::cout << "    - 通用的设备实现\n";
    std::cout << "    - 组合 Transport + Codec\n";
    std::cout << "    - 提供便利方法 (write/read/query)\n\n";

    std::cout << "层级 2 - 协议适配器 (include/vdl/device/)\n";
    std::cout << "  * scpi_adapter_t - VDL 库的一部分\n";
    std::cout << "    - 通用 SCPI 协议支持\n";
    std::cout << "    - 任何 SCPI 设备都能使用\n";
    std::cout << "    - 不依赖特定设备\n\n";

    std::cout << "层级 3 - 用户设备适配器 (examples/ 或用户项目)\n";
    std::cout << "  * vna_adapter_t (示例)\n";
    std::cout << "    - VNA 设备特定的高级 API\n";
    std::cout << "    - 展示如何编写设备适配器\n\n";

    std::cout << "  * your_device_adapter_t (您的项目)\n";
    std::cout << "    - 为您的设备编写\n";
    std::cout << "    - 参考 vna_adapter_t 的模式\n\n";

    std::cout << "架构流图:\n\n";
    std::cout << "  应用程序\n";
    std::cout << "      |\n";
    std::cout << "      +-- vna_adapter_t (或您的适配器)\n";
    std::cout << "      |   \\__ 设备特定的高级 API\n";
    std::cout << "      |       |\n";
    std::cout << "      +-- scpi_adapter_t\n";
    std::cout << "      |   \\__ 通用 SCPI 协议支持\n";
    std::cout << "      |       |\n";
    std::cout << "      +-- device_impl_t\n";
    std::cout << "      |   \\__ Transport + Codec 组合\n";
    std::cout << "      |       |        |\n";
    std::cout << "      +-- tcp_transport_t  scpi_codec_t\n";
    std::cout << "          \\__ 具体实现\n\n";
}

// ============================================================================
// 示例 2: SCPI 适配器的使用
// ============================================================================

void example_scpi_adapter_usage() {
    print_section("示例 2: SCPI 适配器的使用");

    std::cout << "\nSCPI 适配器提供的功能\n\n";

    std::cout << "1. 基础命令\n";
    std::cout << "   scpi.reset()                 // 复位设备\n";
    std::cout << "   scpi.clear_status()          // 清除状态\n";
    std::cout << "   scpi.wait()                  // 等待操作完成\n\n";

    std::cout << "2. 查询设备\n";
    std::cout << "   scpi.get_idn()               // 查询 IDN 字符串\n";
    std::cout << "   scpi.query(\"*STB?\")         // 查询状态字节\n\n";

    std::cout << "3. 类型转换查询\n";
    std::cout << "   scpi.query_double(\":FREQ:CENT?\")  // 查询并转换为 double\n";
    std::cout << "   scpi.query_int(\":*OPC?\")          // 查询并转换为 int\n";
    std::cout << "   scpi.query_bool(\":STAT:QUES?\")    // 查询并转换为 bool\n\n";

    std::cout << "4. 数据解析\n";
    std::cout << "   scpi.parse_data_doubles(data)  // 解析数据为 double 数组\n\n";

    std::cout << "典型使用模式\n\n";
    std::cout << "代码:\n";
    std::cout << "  device_impl_t device(transport, codec);\n";
    std::cout << "  device.connect();\n";
    std::cout << "  \n";
    std::cout << "  scpi_adapter_t scpi(device);\n";
    std::cout << "  \n";
    std::cout << "  // 初始化\n";
    std::cout << "  scpi.reset();\n";
    std::cout << "  \n";
    std::cout << "  // 查询\n";
    std::cout << "  auto idn = scpi.get_idn();\n";
    std::cout << "  if (idn) {\n";
    std::cout << "      std::cout << \"Device: \" << *idn << std::endl;\n";
    std::cout << "  }\n\n";
}

// ============================================================================
// 示例 3: VNA 适配器的设计
// ============================================================================

void example_vna_adapter_design() {
    print_section("示例 3: VNA 适配器的设计");

    std::cout << "\nVNA 适配器 - 一个用户自定义适配器的例子\n\n";

    std::cout << "为什么需要 VNA 适配器?\n";
    std::cout << "  * scpi_adapter_t 提供通用的 SCPI 命令\n";
    std::cout << "  * vna_adapter_t 提供 VNA 特定的高级接口\n";
    std::cout << "  * 使 VNA 操作更简单、更易于理解\n\n";

    std::cout << "VNA 适配器的结构\n\n";
    std::cout << "class vna_adapter_t {\n";
    std::cout << "private:\n";
    std::cout << "    scpi_adapter_t m_scpi;    // 内部使用 SCPI 适配器\n";
    std::cout << "\n";
    std::cout << "public:\n";
    std::cout << "    // 频率控制\n";
    std::cout << "    result_t<void> set_start_freq(double freq_hz);\n";
    std::cout << "    result_t<double> get_start_freq();\n";
    std::cout << "    result_t<void> set_stop_freq(double freq_hz);\n";
    std::cout << "    result_t<double> get_stop_freq();\n";
    std::cout << "\n";
    std::cout << "    // 扫描参数\n";
    std::cout << "    result_t<void> set_sweep_points(int points);\n";
    std::cout << "    result_t<int> get_sweep_points();\n";
    std::cout << "\n";
    std::cout << "    // 测量参数\n";
    std::cout << "    result_t<void> set_measurement_param(const std::string& param);\n";
    std::cout << "    result_t<std::string> get_measurement_param();\n";
    std::cout << "\n";
    std::cout << "    // 测量操作\n";
    std::cout << "    result_t<void> trigger_sweep();\n";
    std::cout << "    result_t<bytes_t> get_formatted_data();\n";
    std::cout << "\n";
    std::cout << "    // 访问底层 SCPI 适配器\n";
    std::cout << "    scpi_adapter_t& scpi() { return m_scpi; }\n";
    std::cout << "};\n\n";

    std::cout << "关键设计特点\n";
    std::cout << "  1. 内部使用 SCPI 适配器\n";
    std::cout << "     - 复用 SCPI 的功能\n";
    std::cout << "     - 保持一致性\n\n";

    std::cout << "  2. 提供 VNA 特定的方法\n";
    std::cout << "     - 隐藏复杂的 SCPI 命令\n";
    std::cout << "     - 提供简洁的 API\n\n";

    std::cout << "  3. 返回 result_t<T>\n";
    std::cout << "     - 统一的错误处理\n";
    std::cout << "     - 与 VDL 库一致\n\n";

    std::cout << "  4. 提供 scpi() 访问器\n";
    std::cout << "     - 需要时可以访问底层\n";
    std::cout << "     - 灵活扩展\n\n";
}

// ============================================================================
// 示例 4: 如何为您的设备编写适配器
// ============================================================================

void example_write_custom_adapter() {
    print_section("示例 4: 为您的设备编写适配器");

    std::cout << "\n步骤 1: 创建适配器文件\n";
    std::cout << "  文件: my_device_adapter.hpp\n\n";

    std::cout << "步骤 2: 包含必要的头文件\n";
    std::cout << "  #include <vdl/device/scpi_adapter.hpp>\n";
    std::cout << "  #include <vdl/device/device_impl.hpp>\n\n";

    std::cout << "步骤 3: 定义您的适配器类\n";
    std::cout << "  class my_device_adapter_t {\n";
    std::cout << "  private:\n";
    std::cout << "      scpi_adapter_t m_scpi;\n";
    std::cout << "  \n";
    std::cout << "  public:\n";
    std::cout << "      explicit my_device_adapter_t(device_impl_t& device)\n";
    std::cout << "          : m_scpi(device) {}\n";
    std::cout << "  \n";
    std::cout << "      // 您的设备特定方法\n";
    std::cout << "      result_t<double> read_temperature();\n";
    std::cout << "      result_t<void> set_output_power(double power_dbm);\n";
    std::cout << "      // ...\n";
    std::cout << "  \n";
    std::cout << "      // 提供对 SCPI 适配器的访问\n";
    std::cout << "      scpi_adapter_t& scpi() { return m_scpi; }\n";
    std::cout << "  };\n\n";

    std::cout << "步骤 4: 实现您的方法\n";
    std::cout << "  result_t<double> my_device_adapter_t::read_temperature() {\n";
    std::cout << "      return m_scpi.query_double(\":MEAS:TEMP?\");\n";
    std::cout << "  }\n";
    std::cout << "  \n";
    std::cout << "  result_t<void> my_device_adapter_t::set_output_power(\n";
    std::cout << "      double power_dbm) {\n";
    std::cout << "      return m_scpi.write(\":POW \" +\n";
    std::cout << "                          std::to_string(power_dbm));\n";
    std::cout << "  }\n\n";

    std::cout << "步骤 5: 在您的应用中使用\n";
    std::cout << "  #include \"my_device_adapter.hpp\"\n";
    std::cout << "  \n";
    std::cout << "  int main() {\n";
    std::cout << "      device_impl_t device(transport, codec);\n";
    std::cout << "      device.connect();\n";
    std::cout << "      \n";
    std::cout << "      my_device_adapter_t adapter(device);\n";
    std::cout << "      auto temp = adapter.read_temperature();\n";
    std::cout << "      if (temp) {\n";
    std::cout << "          std::cout << \"Temperature: \" << *temp\n";
    std::cout << "                    << \" C\" << std::endl;\n";
    std::cout << "      }\n";
    std::cout << "  }\n\n";

    std::cout << "最佳实践\n";
    std::cout << "  1. 参考 vna_adapter.hpp 的结构\n";
    std::cout << "  2. 使用 scpi_adapter_t 提供的基础功能\n";
    std::cout << "  3. 返回 result_t<T> 以支持错误处理\n";
    std::cout << "  4. 提供对底层适配器的访问\n";
    std::cout << "  5. 编写明确的文档\n";
    std::cout << "  6. 将适配器放在您的项目中，不在 VDL 库中\n\n";
}

// ============================================================================
// 示例 5: 库与用户代码的分离原则
// ============================================================================

void example_separation_principles() {
    print_section("示例 5: 库与用户代码的分离原则");

    std::cout << "\nWHY 分离?\n\n";

    std::cout << "问题：如果在库中包含所有设备适配器\n";
    std::cout << "  * 库会变得非常大\n";
    std::cout << "  * 库会依赖许多特定设备\n";
    std::cout << "  * 很难维护\n";
    std::cout << "  * 用户需要下载他们不需要的代码\n\n";

    std::cout << "解决方案：清晰的分离\n";
    std::cout << "  * VDL 库: 通用基础 + scpi_adapter_t\n";
    std::cout << "  * Examples: 展示如何为设备编写适配器\n";
    std::cout << "  * User Project: 您的设备适配器\n\n";

    std::cout << "文件组织\n\n";

    std::cout << "VDL 库 (include/vdl/)\n";
    std::cout << "  ├── device/\n";
    std::cout << "  │   ├── device.hpp\n";
    std::cout << "  │   ├── device_impl.hpp\n";
    std::cout << "  │   └── scpi_adapter.hpp  <-- 通用 SCPI 协议\n";
    std::cout << "  ├── transport/\n";
    std::cout << "  ├── codec/\n";
    std::cout << "  └── ...\n\n";

    std::cout << "Examples (examples/)\n";
    std::cout << "  ├── vna_adapter.hpp      <-- 示例: VNA 适配器\n";
    std::cout << "  ├── 01_basic_usage.cpp\n";
    std::cout << "  ├── 06_vna_scpi_communication.cpp\n";
    std::cout << "  └── 07_vna_adapter_example.cpp\n\n";

    std::cout << "用户项目 (您的代码)\n";
    std::cout << "  ├── my_device_adapter.hpp  <-- 您的设备适配器\n";
    std::cout << "  ├── my_device_adapter.cpp\n";
    std::cout << "  ├── main.cpp\n";
    std::cout << "  └── CMakeLists.txt\n\n";

    std::cout << "关键原则\n";
    std::cout << "  1. 库保持通用和独立\n";
    std::cout << "     - 不包含特定设备代码\n";
    std::cout << "     - 易于维护\n\n";

    std::cout << "  2. Examples 展示模式\n";
    std::cout << "     - 如何使用 SCPI 适配器\n";
    std::cout << "     - 如何编写设备适配器\n";
    std::cout << "     - 最佳实践\n\n";

    std::cout << "  3. 用户代码保持独立\n";
    std::cout << "     - 您的适配器在您的项目中\n";
    std::cout << "     - 您控制实现\n";
    std::cout << "     - 便于定制\n\n";
}

// ============================================================================
// 示例 6: 快速参考
// ============================================================================

void example_quick_reference() {
    print_section("示例 6: 快速参考");

    std::cout << "\n使用 SCPI 适配器\n";
    std::cout << "  #include <vdl/device/scpi_adapter.hpp>\n";
    std::cout << "  \n";
    std::cout << "  device_impl_t device(transport, codec);\n";
    std::cout << "  scpi_adapter_t scpi(device);\n";
    std::cout << "  scpi.reset();\n\n";

    std::cout << "编写设备适配器\n";
    std::cout << "  class my_adapter_t {\n";
    std::cout << "      scpi_adapter_t m_scpi;\n";
    std::cout << "  public:\n";
    std::cout << "      my_adapter_t(device_impl_t& d) : m_scpi(d) {}\n";
    std::cout << "      result_t<double> my_method() {\n";
    std::cout << "          return m_scpi.query_double(\"...\");\n";
    std::cout << "      }\n";
    std::cout << "  };\n\n";

    std::cout << "文件位置\n";
    std::cout << "  * VDL 库: /workspaces/vdl/include/vdl/\n";
    std::cout << "  * Examples: /workspaces/vdl/examples/\n";
    std::cout << "  * 参考: examples/vna_adapter.hpp\n\n";

    std::cout << "关键文件\n";
    std::cout << "  * vdl/device/device.hpp\n";
    std::cout << "  * vdl/device/device_impl.hpp\n";
    std::cout << "  * vdl/device/scpi_adapter.hpp\n";
    std::cout << "  * examples/vna_adapter.hpp\n\n";
}

// ============================================================================
// 主函数
// ============================================================================

int main() {
    std::cout << "\n";
    std::cout << "=" << std::string(58, '=') << "\n";
    std::cout << "  VNA 适配器示例 - 设备特定适配器的架构和最佳实践\n";
    std::cout << "=" << std::string(58, '=') << "\n";

    // 运行示例
    example_architecture();
    example_scpi_adapter_usage();
    example_vna_adapter_design();
    example_write_custom_adapter();
    example_separation_principles();
    example_quick_reference();

    std::cout << "\n";
    std::cout << "=" << std::string(58, '=') << "\n";
    std::cout << "  所有示例完成！\n";
    std::cout << "=" << std::string(58, '=') << "\n";
    std::cout << "\n核心要点:\n";
    std::cout << "  1. scpi_adapter_t 是库的一部分 (通用)\n";
    std::cout << "  2. vna_adapter_t 是用户示例 (特定设备)\n";
    std::cout << "  3. 参考示例为您的设备编写适配器\n";
    std::cout << "  4. 库保持通用，用户编写特定逻辑\n";
    std::cout << "\n";

    return 0;
}
