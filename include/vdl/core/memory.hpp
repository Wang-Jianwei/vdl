#ifndef VDL_MEMORY_HPP
#define VDL_MEMORY_HPP

#include "error.hpp"
#include "compat.hpp"
#include <cstdint>
#include <cstddef>
#include <new>
#include <vector>
#include <map>
#include <algorithm>
#include <atomic>

namespace vdl {

/**
 * @brief Memory allocation statistics
 */
struct MemoryStats {
    uint64_t total_allocated;    ///< Total allocated bytes
    uint64_t total_deallocated;  ///< Total deallocated bytes
    uint64_t current_allocated;  ///< Current allocated bytes
    uint64_t peak_allocated;     ///< Peak allocated bytes
    uint64_t allocation_count;   ///< Number of allocations
    uint64_t deallocation_count; ///< Number of deallocations

    MemoryStats()
        : total_allocated(0), total_deallocated(0), current_allocated(0),
          peak_allocated(0), allocation_count(0), deallocation_count(0) {}

    void reset() {
        total_allocated = 0;
        total_deallocated = 0;
        current_allocated = 0;
        peak_allocated = 0;
        allocation_count = 0;
        deallocation_count = 0;
    }
};

/**
 * @brief Base allocator interface
 */
class Allocator {
public:
    virtual ~Allocator() = default;

    /**
     * @brief Allocate memory
     * @param size Number of bytes to allocate
     * @return Pointer to allocated memory
     */
    virtual void* allocate(size_t size) = 0;

    /**
     * @brief Deallocate memory
     * @param ptr Pointer to memory to deallocate
     */
    virtual void deallocate(void* ptr) = 0;

    /**
     * @brief Get memory statistics
     * @return Current memory statistics
     */
    virtual MemoryStats getStats() const = 0;

    /**
     * @brief Reset statistics
     */
    virtual void resetStats() = 0;
};

/**
 * @brief Standard allocator using new/delete
 */
class StandardAllocator : public Allocator {
private:
    mutable MemoryStats stats_;

public:
    StandardAllocator() : stats_() {}

    void* allocate(size_t size) override {
        VDL_CHECK(size > 0, vdl::ErrorCode::Invalid, "allocation size must be > 0");
        void* ptr = new char[size];
        VDL_CHECK_NOT_NULL(ptr, "allocation failed");
        
        stats_.total_allocated += size;
        stats_.current_allocated += size;
        stats_.allocation_count++;
        if (stats_.current_allocated > stats_.peak_allocated) {
            stats_.peak_allocated = stats_.current_allocated;
        }
        return ptr;
    }

    void deallocate(void* ptr) override {
        if (ptr != nullptr) {
            // Note: We can't track deallocation size in standard allocator
            // This is a limitation of C++ new/delete without custom overhead
            stats_.total_deallocated += 0;  // Would need size tracking
            stats_.deallocation_count++;
        }
    }

    MemoryStats getStats() const override {
        return stats_;
    }

    void resetStats() override {
        stats_.reset();
    }
};

/**
 * @brief Stack allocator for linear allocation patterns
 */
class StackAllocator : public Allocator {
private:
    char* buffer_;
    size_t capacity_;
    size_t offset_;
    MemoryStats stats_;
    bool owns_memory_;

public:
    /**
     * @brief Construct with external buffer
     */
    StackAllocator(void* buffer, size_t capacity)
        : buffer_(static_cast<char*>(buffer)), capacity_(capacity),
          offset_(0), stats_(), owns_memory_(false) {
        VDL_CHECK_NOT_NULL(buffer, "buffer cannot be null");
        VDL_CHECK(capacity > 0, vdl::ErrorCode::Invalid, "capacity must be > 0");
    }

    /**
     * @brief Construct with internal buffer
     */
    explicit StackAllocator(size_t capacity)
        : buffer_(nullptr), capacity_(capacity), offset_(0), stats_(),
          owns_memory_(true) {
        VDL_CHECK(capacity > 0, vdl::ErrorCode::Invalid, "capacity must be > 0");
        buffer_ = new char[capacity];
        VDL_CHECK_NOT_NULL(buffer_, "failed to allocate internal buffer");
    }

    ~StackAllocator() {
        if (owns_memory_) {
            delete[] buffer_;
        }
    }

    void* allocate(size_t size) override {
        VDL_CHECK(size > 0, vdl::ErrorCode::Invalid, "allocation size must be > 0");
        
        if (offset_ + size > capacity_) {
            VDL_THROW(vdl::ErrorCode::OutOfMemory, "stack allocator exhausted");
        }

        void* ptr = buffer_ + offset_;
        offset_ += size;

        stats_.total_allocated += size;
        stats_.current_allocated += size;
        stats_.allocation_count++;
        if (stats_.current_allocated > stats_.peak_allocated) {
            stats_.peak_allocated = stats_.current_allocated;
        }

        return ptr;
    }

    void deallocate(void* ptr) override {
        // Stack allocator doesn't support individual deallocation
        // Only reset() clears all allocations
        if (ptr != nullptr) {
            stats_.deallocation_count++;
        }
    }

    /**
     * @brief Reset all allocations
     */
    void reset() {
        offset_ = 0;
        stats_.current_allocated = 0;
        stats_.allocation_count = 0;
    }

    MemoryStats getStats() const override {
        return stats_;
    }

    void resetStats() override {
        stats_.reset();
    }

    /**
     * @brief Get current usage
     */
    size_t getUsage() const {
        return offset_;
    }

    /**
     * @brief Get remaining capacity
     */
    size_t getRemaining() const {
        return capacity_ - offset_;
    }
};

/**
 * @brief Memory pool for fixed-size allocations
 */
class MemoryPool : public Allocator {
private:
    struct Block {
        void* ptr;
        bool allocated;
    };

    std::vector<Block> blocks_;
    size_t block_size_;
    char* memory_;
    size_t total_blocks_;
    MemoryStats stats_;
    bool owns_memory_;

public:
    /**
     * @brief Construct with specified block size and count
     */
    MemoryPool(size_t block_size, size_t block_count)
        : block_size_(block_size), memory_(nullptr), total_blocks_(block_count),
          stats_(), owns_memory_(true) {
        VDL_CHECK(block_size > 0, vdl::ErrorCode::Invalid, "block size must be > 0");
        VDL_CHECK(block_count > 0, vdl::ErrorCode::Invalid, "block count must be > 0");

        size_t total_size = block_size * block_count;
        memory_ = new char[total_size];
        VDL_CHECK_NOT_NULL(memory_, "failed to allocate pool memory");

        blocks_.reserve(block_count);
        for (size_t i = 0; i < block_count; ++i) {
            Block block;
            block.ptr = memory_ + (i * block_size);
            block.allocated = false;
            blocks_.push_back(block);
        }
    }

    ~MemoryPool() {
        if (owns_memory_ && memory_ != nullptr) {
            delete[] memory_;
        }
    }

    /**
     * @brief Allocate a fixed-size block of memory
     * 
     * MemoryPool manages fixed-size blocks. The size parameter specifies the
     * minimum size needed; if it exceeds block_size_, the allocation fails.
     * Statistics track the actual allocated size (block_size_), not the
     * requested size.
     * 
     * @param size Requested size in bytes (must be <= block_size_)
     * @return Pointer to the allocated block
     * @throw VdlException if size > block_size_ or pool is exhausted
     */
    void* allocate(size_t size) override {
        VDL_CHECK(size > 0, vdl::ErrorCode::Invalid, "allocation size must be > 0");
        VDL_CHECK(size <= block_size_, vdl::ErrorCode::Invalid,
                  "requested size exceeds block size");

        for (auto& block : blocks_) {
            if (!block.allocated) {
                block.allocated = true;
                // Statistics track actual allocated block_size_, not requested size
                stats_.total_allocated += block_size_;
                stats_.current_allocated += block_size_;
                stats_.allocation_count++;
                if (stats_.current_allocated > stats_.peak_allocated) {
                    stats_.peak_allocated = stats_.current_allocated;
                }
                return block.ptr;
            }
        }

        VDL_THROW(vdl::ErrorCode::OutOfMemory, "memory pool exhausted");
    }

    void deallocate(void* ptr) override {
        if (ptr == nullptr) {
            return;
        }

        for (auto& block : blocks_) {
            if (block.ptr == ptr && block.allocated) {
                block.allocated = false;
                stats_.total_deallocated += block_size_;
                stats_.current_allocated -= block_size_;
                stats_.deallocation_count++;
                return;
            }
        }

        VDL_THROW(vdl::ErrorCode::Invalid, "pointer not in this pool");
    }

    MemoryStats getStats() const override {
        return stats_;
    }

    void resetStats() override {
        stats_.reset();
    }

    /**
     * @brief Get number of free blocks
     */
    size_t getFreeBlocks() const {
        size_t free = 0;
        for (const auto& block : blocks_) {
            if (!block.allocated) {
                ++free;
            }
        }
        return free;
    }

    /**
     * @brief Get total number of blocks
     */
    size_t getTotalBlocks() const {
        return total_blocks_;
    }

    /**
     * @brief Get block size
     */
    size_t getBlockSize() const {
        return block_size_;
    }
};

/**
 * @brief Global memory manager
 */
class MemoryManager {
public:
    static Allocator* default_allocator_;
    static StandardAllocator standard_allocator_;

    /**
     * @brief Set default allocator
     */
    static void setDefaultAllocator(Allocator* allocator) {
        if (allocator != nullptr) {
            default_allocator_ = allocator;
        } else {
            default_allocator_ = &standard_allocator_;
        }
    }

    /**
     * @brief Get default allocator
     */
    static Allocator* getDefaultAllocator() {
        return default_allocator_;
    }

    /**
     * @brief Reset to standard allocator
     */
    static void resetToStandardAllocator() {
        default_allocator_ = &standard_allocator_;
    }

    /**
     * @brief Allocate memory using default allocator
     */
    static void* allocate(size_t size) {
        return default_allocator_->allocate(size);
    }

    /**
     * @brief Deallocate memory using default allocator
     */
    static void deallocate(void* ptr) {
        default_allocator_->deallocate(ptr);
    }

    /**
     * @brief Get memory statistics
     */
    static MemoryStats getStats() {
        return default_allocator_->getStats();
    }

    /**
     * @brief Reset statistics
     */
    static void resetStats() {
        default_allocator_->resetStats();
    }
};

/**
 * @brief RAII memory guard for exception safety
 */
class MemoryGuard {
private:
    void* ptr_;
    Allocator* allocator_;

public:
    MemoryGuard(void* ptr, Allocator* allocator)
        : ptr_(ptr), allocator_(allocator) {
        if (allocator_ == nullptr) {
            allocator_ = MemoryManager::getDefaultAllocator();
        }
    }

    ~MemoryGuard() {
        if (ptr_ != nullptr && allocator_ != nullptr) {
            allocator_->deallocate(ptr_);
        }
    }

    /**
     * @brief Release ownership
     */
    void* release() {
        void* ptr = ptr_;
        ptr_ = nullptr;
        return ptr;
    }

    /**
     * @brief Get pointer
     */
    void* get() const {
        return ptr_;
    }

    /**
     * @brief Check if valid
     */
    bool valid() const {
        return ptr_ != nullptr;
    }
};

/**
 * @brief Convenience macro for allocating and deallocating
 */
#define VDL_ALLOCATE(size) vdl::MemoryManager::allocate(size)
#define VDL_DEALLOCATE(ptr) vdl::MemoryManager::deallocate(ptr)

/**
 * @brief Allocate with guard for exception safety
 */
#define VDL_ALLOCATE_GUARDED(size, allocator) \
    vdl::MemoryGuard(vdl::MemoryManager::allocate(size), allocator)

} // namespace vdl

#endif // VDL_MEMORY_HPP
