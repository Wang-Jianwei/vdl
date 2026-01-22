/**
 * @file binary_codec.hpp
 * @brief 二进制协议编解码器
 * 
 * 提供一个简单的二进制帧格式编解码实现。
 */

#ifndef VDL_CODEC_BINARY_CODEC_HPP
#define VDL_CODEC_BINARY_CODEC_HPP

#include "codec.hpp"
#include "../core/buffer.hpp"

namespace vdl {

// ============================================================================
// 帧格式常量
// ============================================================================

namespace binary_frame {

/**
 * 简单二进制帧格式:
 * 
 * | 字段   | 长度 | 说明           |
 * |--------|------|----------------|
 * | SOF    | 1    | 帧起始 0xAA    |
 * | LEN    | 2    | 数据长度(LE)   |
 * | FUNC   | 1    | 功能码         |
 * | DATA   | N    | 数据           |
 * | CRC    | 2    | CRC16(LE)      |
 */

constexpr byte_t SOF = 0xAA;          ///< 帧起始标志
constexpr size_t HEADER_SIZE = 4;     ///< 头部大小 (SOF + LEN + FUNC)
constexpr size_t CRC_SIZE = 2;        ///< CRC 大小
constexpr size_t MIN_FRAME_SIZE = 6;  ///< 最小帧大小

/**
 * @brief 计算 CRC16 (CCITT)
 */
inline uint16_t crc16(const byte_t* data, size_t len) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; ++i) {
        crc ^= static_cast<uint16_t>(static_cast<uint16_t>(data[i]) << 8);
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x8000) {
                crc = static_cast<uint16_t>((crc << 1) ^ 0x1021);
            } else {
                crc = static_cast<uint16_t>(crc << 1);
            }
        }
    }
    return crc;
}

}  // namespace binary_frame

// ============================================================================
// binary_codec_t - 二进制编解码器
// ============================================================================

/**
 * @brief 简单二进制协议编解码器
 * 
 * @code
 * binary_codec_t codec;
 * 
 * // 编码命令
 * command_t cmd;
 * cmd.set_function_code(0x03).set_data({0x00, 0x01, 0x00, 0x0A});
 * auto frame = codec.encode(cmd);
 * 
 * // 解码响应
 * size_t consumed = 0;
 * auto response = codec.decode(received_data, consumed);
 * @endcode
 */
class binary_codec_t : public codec_base_t {
public:
    // ========================================================================
    // i_codec_t 实现
    // ========================================================================

    result_t<bytes_t> encode(const command_t& cmd) override {
        const bytes_t& data = cmd.data();
        size_t data_len = data.size();
        size_t frame_len = binary_frame::HEADER_SIZE + data_len + 
                          binary_frame::CRC_SIZE;

        if (frame_len > m_max_frame_size) {
            return make_error<bytes_t>(error_code_t::frame_too_large,
                                       "Frame size exceeds maximum");
        }

        bytes_t frame(frame_len);

        // SOF
        frame[0] = binary_frame::SOF;

        // LEN (小端序)
        frame[1] = static_cast<byte_t>(data_len & 0xFF);
        frame[2] = static_cast<byte_t>((data_len >> 8) & 0xFF);

        // FUNC
        frame[3] = cmd.function_code();

        // DATA
        std::copy(data.begin(), data.end(), frame.begin() + 4);

        // CRC (小端序)
        uint16_t crc = binary_frame::crc16(frame.data(), frame_len - 2);
        frame[frame_len - 2] = static_cast<byte_t>(crc & 0xFF);
        frame[frame_len - 1] = static_cast<byte_t>((crc >> 8) & 0xFF);

        return frame;
    }

    result_t<response_t> decode(const_byte_span_t buffer, 
                                 size_t& consumed) override {
        consumed = 0;

        // 检查最小长度
        if (buffer.size() < binary_frame::MIN_FRAME_SIZE) {
            return make_error<response_t>(error_code_t::incomplete_frame,
                                          "Incomplete frame: need more data");
        }

        // 查找 SOF
        size_t sof_pos = 0;
        while (sof_pos < buffer.size() && 
               buffer[sof_pos] != binary_frame::SOF) {
            ++sof_pos;
        }

        if (sof_pos > 0) {
            consumed = sof_pos;
            return make_error<response_t>(error_code_t::invalid_frame,
                                          "Invalid data before SOF");
        }

        // 读取长度
        uint16_t data_len = static_cast<uint16_t>(static_cast<uint16_t>(buffer[1]) |
                           (static_cast<uint16_t>(buffer[2]) << 8));

        size_t frame_len = binary_frame::HEADER_SIZE + data_len + 
                          binary_frame::CRC_SIZE;

        // 检查帧是否完整
        if (buffer.size() < frame_len) {
            return make_error<response_t>(error_code_t::incomplete_frame,
                                          "Incomplete frame: need more data");
        }

        // 检查帧大小
        if (frame_len > m_max_frame_size) {
            consumed = 1;  // 跳过无效的 SOF
            return make_error<response_t>(error_code_t::frame_too_large,
                                          "Frame size exceeds maximum");
        }

        // 验证 CRC
        uint16_t expected_crc = binary_frame::crc16(buffer.data(), 
                                                     frame_len - 2);
        uint16_t actual_crc = static_cast<uint16_t>(static_cast<uint16_t>(buffer[frame_len - 2]) |
                             (static_cast<uint16_t>(buffer[frame_len - 1]) << 8));

        if (expected_crc != actual_crc) {
            consumed = 1;
            return make_error<response_t>(error_code_t::checksum_error,
                                          "CRC mismatch");
        }

        // 解码成功
        response_t response;
        response.set_status(response_status_t::success);
        response.set_function_code(buffer[3]);

        if (data_len > 0) {
            bytes_t data(data_len);
            std::copy(buffer.begin() + 4, 
                     buffer.begin() + 4 + data_len,
                     data.begin());
            response.set_data(std::move(data));
        }

        // 保存原始帧
        bytes_t raw(frame_len);
        std::copy(buffer.begin(), buffer.begin() + frame_len, raw.begin());
        response.set_raw_frame(std::move(raw));

        consumed = frame_len;
        return response;
    }

    size_t frame_length(const_byte_span_t buffer) const override {
        if (buffer.size() < 3) {
            return 0;
        }

        if (buffer[0] != binary_frame::SOF) {
            return 0;
        }

        uint16_t data_len = static_cast<uint16_t>(static_cast<uint16_t>(buffer[1]) |
                           (static_cast<uint16_t>(buffer[2]) << 8));

        size_t frame_len = binary_frame::HEADER_SIZE + data_len + 
                          binary_frame::CRC_SIZE;

        return frame_len;
    }

    const char* name() const override {
        return "binary";
    }
};

}  // namespace vdl

#endif  // VDL_CODEC_BINARY_CODEC_HPP
