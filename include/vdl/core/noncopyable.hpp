// Non-copyable and Non-movable Utilities
#pragma once

namespace vdl {

/// @brief Base class to make derived class non-copyable
class NonCopyable {
protected:
    NonCopyable() = default;
    ~NonCopyable() = default;

    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
};

/// @brief Base class to make derived class non-movable
class NonMovable {
protected:
    NonMovable() = default;
    ~NonMovable() = default;

    NonMovable(const NonMovable&) = delete;
    NonMovable& operator=(const NonMovable&) = delete;
    
    NonMovable(NonMovable&&) = delete;
    NonMovable& operator=(NonMovable&&) = delete;
};

}  // namespace vdl
