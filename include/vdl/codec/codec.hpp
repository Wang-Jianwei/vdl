/**
 * @file codec.hpp
 * @brief 编解码层接口
 * 
 * 定义命令编码和响应解码的抽象接口。
 */

#ifndef VDL_CODEC_CODEC_HPP
#define VDL_CODEC_CODEC_HPP

#include "../core/types.hpp"
#include "../core/error.hpp"
#include "../core/noncopyable.hpp"
#include "../protocol/command.hpp"
#include "../protocol/response.hpp"

#include <memory>

namespace vdl {

// ============================================================================
// i_codec_t - 编解码器接口
// ============================================================================

/**
 * @brief 编解码器抽象接口
 * 
 * 负责：
 * - 将命令编码为字节流
 * - 从字节流解码响应
 * - 帧边界检测
 * 
 * 实现类：
 * - binary_codec_t: 二进制协议编解码
 * - ascii_codec_t: ASCII 协议编解码（如 SCPI）
 */
class i_codec_t : private noncopyable_t {
public:
    virtual ~i_codec_t() = default;

    // ========================================================================
    // 编码
    // ========================================================================

    /**
     * @brief 编码命令
     * @param cmd 命令对象
     * @return 成功返回编码后的字节流，失败返回错误
     */
    virtual result_t<bytes_t> encode(const command_t& cmd) = 0;

    // ========================================================================
    // 解码
    // ========================================================================

    /**
     * @brief 尝试从缓冲区解码响应
     * @param buffer 输入缓冲区
     * @param consumed [out] 消耗的字节数
     * @return 成功返回响应对象，数据不完整返回 incomplete_frame，失败返回其他错误
     * 
     * @note 如果返回 incomplete_frame 错误，调用者应继续读取数据后重试
     */
    virtual result_t<response_t> decode(const_byte_span_t buffer, 
                                         size_t& consumed) = 0;

    /**
     * @brief 检查缓冲区是否包含完整帧
     * @param buffer 输入缓冲区
     * @return 完整帧的长度，如果不完整返回 0
     */
    virtual size_t frame_length(const_byte_span_t buffer) const = 0;

    // ========================================================================
    // 配置
    // ========================================================================

    /**
     * @brief 获取最大帧长度
     */
    virtual size_t max_frame_size() const = 0;

    /**
     * @brief 设置最大帧长度
     */
    virtual void set_max_frame_size(size_t size) = 0;

    /**
     * @brief 获取编解码器名称
     */
    virtual const char* name() const = 0;
};

// ============================================================================
// codec_ptr_t - 编解码器智能指针
// ============================================================================

using codec_ptr_t = std::unique_ptr<i_codec_t>;

// ============================================================================
// codec_base_t - 编解码器基类
// ============================================================================

/**
 * @brief 编解码器基类，提供通用功能
 */
class codec_base_t : public i_codec_t {
public:
    size_t max_frame_size() const override {
        return m_max_frame_size;
    }

    void set_max_frame_size(size_t size) override {
        m_max_frame_size = size;
    }

protected:
    size_t m_max_frame_size = 65536;
};

}  // namespace vdl

#endif  // VDL_CODEC_CODEC_HPP
