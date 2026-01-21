#ifndef VDL_BUFFER_HPP
#define VDL_BUFFER_HPP

#include "error.hpp"
#include "memory.hpp"
#include "types.hpp"
#include <cstring>
#include <algorithm>
#include <vector>

namespace vdl {

/**
 * @brief Circular buffer for byte data
 */
class CircularBuffer {
private:
    uint8_t* buffer_;
    size_t capacity_;
    size_t write_pos_;
    size_t read_pos_;
    size_t data_size_;
    Allocator* allocator_;
    bool owns_memory_;

public:
    /**
     * @brief Construct with specified capacity
     */
    explicit CircularBuffer(size_t capacity, Allocator* allocator = nullptr)
        : buffer_(nullptr), capacity_(capacity), write_pos_(0), read_pos_(0),
          data_size_(0), allocator_(allocator), owns_memory_(true) {
        VDL_CHECK(capacity > 0, vdl::ErrorCode::Invalid, "capacity must be > 0");
        
        if (allocator_ == nullptr) {
            allocator_ = MemoryManager::getDefaultAllocator();
        }

        buffer_ = static_cast<uint8_t*>(allocator_->allocate(capacity));
        VDL_CHECK_NOT_NULL(buffer_, "failed to allocate buffer memory");
    }

    /**
     * @brief Construct with external buffer
     */
    CircularBuffer(void* buffer, size_t capacity)
        : buffer_(static_cast<uint8_t*>(buffer)), capacity_(capacity),
          write_pos_(0), read_pos_(0), data_size_(0),
          allocator_(nullptr), owns_memory_(false) {
        VDL_CHECK_NOT_NULL(buffer, "buffer cannot be null");
        VDL_CHECK(capacity > 0, vdl::ErrorCode::Invalid, "capacity must be > 0");
    }

    ~CircularBuffer() {
        if (owns_memory_ && buffer_ != nullptr && allocator_ != nullptr) {
            allocator_->deallocate(buffer_);
        }
    }

    /**
     * @brief Write data to buffer
     */
    size_t write(const void* data, size_t size) {
        VDL_CHECK_NOT_NULL(data, "data pointer cannot be null");
        
        // Available space in buffer
        size_t available = capacity_ - data_size_;
        size_t to_write = std::min(size, available);

        if (to_write == 0) {
            return 0;  // Buffer full
        }

        const uint8_t* src = static_cast<const uint8_t*>(data);

        // Write in two parts if wrapping around
        if (write_pos_ + to_write <= capacity_) {
            // No wrap: direct copy
            std::memcpy(buffer_ + write_pos_, src, to_write);
        } else {
            // Wrap: split into two parts
            size_t first_part = capacity_ - write_pos_;
            std::memcpy(buffer_ + write_pos_, src, first_part);
            std::memcpy(buffer_, src + first_part, to_write - first_part);
        }

        write_pos_ = (write_pos_ + to_write) % capacity_;
        data_size_ += to_write;

        return to_write;
    }

    /**
     * @brief Read data from buffer
     */
    size_t read(void* data, size_t size) {
        VDL_CHECK_NOT_NULL(data, "data pointer cannot be null");
        
        size_t to_read = std::min(size, data_size_);

        if (to_read == 0) {
            return 0;  // Buffer empty
        }

        uint8_t* dst = static_cast<uint8_t*>(data);

        // Read in two parts if wrapping around
        if (read_pos_ + to_read <= capacity_) {
            // No wrap: direct copy
            std::memcpy(dst, buffer_ + read_pos_, to_read);
        } else {
            // Wrap: split into two parts
            size_t first_part = capacity_ - read_pos_;
            std::memcpy(dst, buffer_ + read_pos_, first_part);
            std::memcpy(dst + first_part, buffer_, to_read - first_part);
        }

        read_pos_ = (read_pos_ + to_read) % capacity_;
        data_size_ -= to_read;

        return to_read;
    }

    /**
     * @brief Peek data without removing from buffer
     */
    size_t peek(void* data, size_t size) const {
        VDL_CHECK_NOT_NULL(data, "data pointer cannot be null");
        
        size_t to_peek = std::min(size, data_size_);

        if (to_peek == 0) {
            return 0;
        }

        uint8_t* dst = static_cast<uint8_t*>(data);

        if (read_pos_ + to_peek <= capacity_) {
            std::memcpy(dst, buffer_ + read_pos_, to_peek);
        } else {
            size_t first_part = capacity_ - read_pos_;
            std::memcpy(dst, buffer_ + read_pos_, first_part);
            std::memcpy(dst + first_part, buffer_, to_peek - first_part);
        }

        return to_peek;
    }

    /**
     * @brief Clear all data from buffer
     */
    void clear() {
        write_pos_ = 0;
        read_pos_ = 0;
        data_size_ = 0;
    }

    /**
     * @brief Skip bytes in the buffer
     */
    void skip(size_t count) {
        size_t to_skip = std::min(count, data_size_);
        read_pos_ = (read_pos_ + to_skip) % capacity_;
        data_size_ -= to_skip;
    }

    /**
     * @brief Get available data size
     */
    size_t getAvailableSize() const {
        return data_size_;
    }

    /**
     * @brief Get remaining capacity
     */
    size_t getRemainingCapacity() const {
        return capacity_ - data_size_;
    }

    /**
     * @brief Get total capacity
     */
    size_t getCapacity() const {
        return capacity_;
    }

    /**
     * @brief Check if buffer is empty
     */
    bool isEmpty() const {
        return data_size_ == 0;
    }

    /**
     * @brief Check if buffer is full
     */
    bool isFull() const {
        return data_size_ == capacity_;
    }

    /**
     * @brief Get write position
     */
    size_t getWritePos() const {
        return write_pos_;
    }

    /**
     * @brief Get read position
     */
    size_t getReadPos() const {
        return read_pos_;
    }
};

/**
 * @brief Template-based ring buffer for typed data
 */
template<typename T>
class RingBuffer {
private:
    std::vector<T> buffer_;
    size_t capacity_;
    size_t write_pos_;
    size_t read_pos_;
    size_t data_count_;

public:
    /**
     * @brief Construct with specified capacity
     */
    explicit RingBuffer(size_t capacity)
        : buffer_(capacity), capacity_(capacity), write_pos_(0),
          read_pos_(0), data_count_(0) {
        VDL_CHECK(capacity > 0, vdl::ErrorCode::Invalid, "capacity must be > 0");
    }

    /**
     * @brief Push element to buffer
     */
    bool push(const T& value) {
        if (data_count_ >= capacity_) {
            return false;  // Buffer full
        }

        buffer_[write_pos_] = value;
        write_pos_ = (write_pos_ + 1) % capacity_;
        data_count_++;

        return true;
    }

    /**
     * @brief Pop element from buffer
     */
    bool pop(T& value) {
        if (data_count_ == 0) {
            return false;  // Buffer empty
        }

        value = buffer_[read_pos_];
        read_pos_ = (read_pos_ + 1) % capacity_;
        data_count_--;

        return true;
    }

    /**
     * @brief Peek front element
     */
    bool front(T& value) const {
        if (data_count_ == 0) {
            return false;
        }

        value = buffer_[read_pos_];
        return true;
    }

    /**
     * @brief Peek back element
     */
    bool back(T& value) const {
        if (data_count_ == 0) {
            return false;
        }

        size_t back_pos = (write_pos_ + capacity_ - 1) % capacity_;
        value = buffer_[back_pos];
        return true;
    }

    /**
     * @brief Get element at index (0 = front)
     */
    bool at(size_t index, T& value) const {
        if (index >= data_count_) {
            return false;
        }

        size_t pos = (read_pos_ + index) % capacity_;
        value = buffer_[pos];
        return true;
    }

    /**
     * @brief Clear all elements
     */
    void clear() {
        write_pos_ = 0;
        read_pos_ = 0;
        data_count_ = 0;
    }

    /**
     * @brief Get element count
     */
    size_t size() const {
        return data_count_;
    }

    /**
     * @brief Get capacity
     */
    size_t capacity() const {
        return capacity_;
    }

    /**
     * @brief Check if empty
     */
    bool empty() const {
        return data_count_ == 0;
    }

    /**
     * @brief Check if full
     */
    bool full() const {
        return data_count_ == capacity_;
    }

    /**
     * @brief Get remaining capacity
     */
    size_t remaining() const {
        return capacity_ - data_count_;
    }
};

/**
 * @brief Buffer pool for efficient allocation
 */
class BufferPool {
private:
    struct BufferNode {
        uint8_t* data;
        size_t size;
        bool available;
    };

    std::vector<BufferNode> buffers_;
    size_t buffer_size_;
    Allocator* allocator_;

public:
    /**
     * @brief Construct buffer pool
     */
    BufferPool(size_t buffer_size, size_t buffer_count, Allocator* allocator = nullptr)
        : buffer_size_(buffer_size), allocator_(allocator) {
        VDL_CHECK(buffer_size > 0, vdl::ErrorCode::Invalid, "buffer size must be > 0");
        VDL_CHECK(buffer_count > 0, vdl::ErrorCode::Invalid, "buffer count must be > 0");

        if (allocator_ == nullptr) {
            allocator_ = MemoryManager::getDefaultAllocator();
        }

        buffers_.reserve(buffer_count);
        for (size_t i = 0; i < buffer_count; ++i) {
            BufferNode node;
            node.data = static_cast<uint8_t*>(allocator_->allocate(buffer_size));
            VDL_CHECK_NOT_NULL(node.data, "failed to allocate buffer");
            node.size = buffer_size;
            node.available = true;
            buffers_.push_back(node);
        }
    }

    ~BufferPool() {
        for (auto& node : buffers_) {
            if (node.data != nullptr && allocator_ != nullptr) {
                allocator_->deallocate(node.data);
            }
        }
    }

    /**
     * @brief Acquire a buffer
     */
    uint8_t* acquire() {
        for (auto& node : buffers_) {
            if (node.available) {
                node.available = false;
                return node.data;
            }
        }

        return nullptr;  // No available buffer
    }

    /**
     * @brief Release a buffer
     */
    bool release(uint8_t* buffer) {
        if (buffer == nullptr) {
            return false;
        }

        for (auto& node : buffers_) {
            if (node.data == buffer && !node.available) {
                node.available = true;
                return true;
            }
        }

        return false;  // Buffer not in pool
    }

    /**
     * @brief Get buffer size
     */
    size_t getBufferSize() const {
        return buffer_size_;
    }

    /**
     * @brief Get number of available buffers
     */
    size_t getAvailableCount() const {
        size_t count = 0;
        for (const auto& node : buffers_) {
            if (node.available) {
                ++count;
            }
        }
        return count;
    }

    /**
     * @brief Get total buffer count
     */
    size_t getTotalCount() const {
        return buffers_.size();
    }

    /**
     * @brief Clear all buffers (mark as available)
     */
    void reset() {
        for (auto& node : buffers_) {
            node.available = true;
        }
    }
};

/**
 * @brief RAII buffer guard for automatic buffer release
 */
class BufferGuard {
private:
    uint8_t* buffer_;
    BufferPool* pool_;

public:
    BufferGuard(uint8_t* buffer, BufferPool* pool)
        : buffer_(buffer), pool_(pool) {}

    ~BufferGuard() {
        if (buffer_ != nullptr && pool_ != nullptr) {
            pool_->release(buffer_);
        }
    }

    uint8_t* get() const {
        return buffer_;
    }

    uint8_t* release() {
        uint8_t* ptr = buffer_;
        buffer_ = nullptr;
        return ptr;
    }

    bool valid() const {
        return buffer_ != nullptr;
    }
};

} // namespace vdl

#endif // VDL_BUFFER_HPP
