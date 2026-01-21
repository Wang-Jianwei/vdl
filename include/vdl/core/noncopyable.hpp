/**
 * @file noncopyable.hpp
 * @brief 不可复制和不可移动基类
 */

#ifndef VDL_CORE_NONCOPYABLE_HPP
#define VDL_CORE_NONCOPYABLE_HPP

namespace vdl {

/**
 * @brief 不可复制基类
 * 
 * 继承此类的派生类将禁用复制构造和复制赋值。
 */
class noncopyable_t {
protected:
    noncopyable_t() = default;
    ~noncopyable_t() = default;

    // 禁用复制
    noncopyable_t(const noncopyable_t&) = delete;
    noncopyable_t& operator=(const noncopyable_t&) = delete;

    // 允许移动
    noncopyable_t(noncopyable_t&&) = default;
    noncopyable_t& operator=(noncopyable_t&&) = default;
};

/**
 * @brief 不可复制且不可移动基类
 * 
 * 继承此类的派生类将禁用所有复制和移动操作。
 */
class nonmovable_t {
protected:
    nonmovable_t() = default;
    ~nonmovable_t() = default;

    // 禁用复制
    nonmovable_t(const nonmovable_t&) = delete;
    nonmovable_t& operator=(const nonmovable_t&) = delete;

    // 禁用移动
    nonmovable_t(nonmovable_t&&) = delete;
    nonmovable_t& operator=(nonmovable_t&&) = delete;
};

}  // namespace vdl

#endif  // VDL_CORE_NONCOPYABLE_HPP
