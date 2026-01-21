/**
 * @file scope_guard.hpp
 * @brief RAII 作用域守卫
 * 
 * 提供 scope_guard_t 用于在作用域结束时执行清理操作。
 */

#ifndef VDL_CORE_SCOPE_GUARD_HPP
#define VDL_CORE_SCOPE_GUARD_HPP

#include "noncopyable.hpp"

#include <utility>

namespace vdl {

// ============================================================================
// scope_guard_t - 作用域守卫
// ============================================================================

/**
 * @brief 作用域守卫，在作用域结束时执行清理函数
 * @tparam Func 清理函数类型
 * 
 * @code
 * void example() {
 *     auto* resource = acquire_resource();
 *     auto guard = make_scope_guard([&]() {
 *         release_resource(resource);
 *     });
 *     
 *     // ... 使用 resource ...
 *     
 *     if (success) {
 *         guard.dismiss();  // 成功时不释放
 *     }
 * }  // 失败时自动释放
 * @endcode
 */
template <typename Func>
class scope_guard_t : private noncopyable_t {
public:
    /**
     * @brief 构造函数
     * @param func 退出时执行的清理函数
     */
    explicit scope_guard_t(Func func)
        : m_func(std::move(func))
        , m_active(true) {
    }

    /**
     * @brief 移动构造函数
     */
    scope_guard_t(scope_guard_t&& other)
        : m_func(std::move(other.m_func))
        , m_active(other.m_active) {
        other.dismiss();
    }

    /**
     * @brief 析构函数，执行清理操作
     */
    ~scope_guard_t() {
        if (m_active) {
            try {
                m_func();
            } catch (...) {
                // 忽略析构函数中的异常
            }
        }
    }

    /**
     * @brief 取消清理操作
     */
    void dismiss() {
        m_active = false;
    }

    /**
     * @brief 检查是否活跃
     */
    bool is_active() const {
        return m_active;
    }

private:
    Func m_func;
    bool m_active;
};

/**
 * @brief 创建作用域守卫
 * @tparam Func 清理函数类型
 * @param func 清理函数
 * @return 作用域守卫对象
 */
template <typename Func>
scope_guard_t<Func> make_scope_guard(Func func) {
    return scope_guard_t<Func>(std::move(func));
}

// ============================================================================
// VDL_SCOPE_EXIT 宏
// ============================================================================

namespace detail {

/**
 * @brief 用于创建匿名作用域守卫的辅助类
 */
struct scope_guard_maker_t {
    template <typename Func>
    scope_guard_t<Func> operator<<(Func func) const {
        return scope_guard_t<Func>(std::move(func));
    }
};

}  // namespace detail

// 连接宏辅助
#define VDL_CONCAT_IMPL(a, b) a##b
#define VDL_CONCAT(a, b) VDL_CONCAT_IMPL(a, b)

/**
 * @brief 在作用域退出时执行代码
 * 
 * @code
 * void example() {
 *     FILE* fp = std::fopen("file.txt", "r");
 *     VDL_SCOPE_EXIT { std::fclose(fp); };
 *     
 *     // ... 使用文件 ...
 * }  // 自动关闭文件
 * @endcode
 */
#define VDL_SCOPE_EXIT \
    auto VDL_CONCAT(_scope_guard_, __LINE__) = \
        ::vdl::detail::scope_guard_maker_t() << [&]()

}  // namespace vdl

#endif  // VDL_CORE_SCOPE_GUARD_HPP
