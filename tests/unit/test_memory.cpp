/**
 * @file test_memory.cpp
 * @brief 测试内存管理工具
 */

#include <catch.hpp>
#include <vdl/core/memory.hpp>

// ============================================================================
// buffer_pool_t 测试
// ============================================================================

TEST_CASE("buffer_pool_t construction", "[core][memory]") {
    vdl::buffer_pool_t pool(1024, 4);

    REQUIRE(pool.buffer_size() == 1024);
    REQUIRE(pool.pool_size() == 4);
}

TEST_CASE("buffer_pool_t acquire and release", "[core][memory]") {
    vdl::buffer_pool_t pool(256, 2);

    // 获取缓冲区
    auto buf1 = pool.acquire();
    REQUIRE(buf1.size() == 256);
    REQUIRE(pool.pool_size() == 1);

    auto buf2 = pool.acquire();
    REQUIRE(pool.pool_size() == 0);

    // 再次获取会创建新的
    auto buf3 = pool.acquire();
    REQUIRE(buf3.size() == 256);

    // 归还
    pool.release(std::move(buf1));
    REQUIRE(pool.pool_size() == 1);

    pool.release(std::move(buf2));
    REQUIRE(pool.pool_size() == 2);
}

TEST_CASE("buffer_pool_t max pool size", "[core][memory]") {
    vdl::buffer_pool_t pool(64, 2);
    pool.set_max_pool_size(3);

    // 归还超过最大数量的缓冲区
    auto buf1 = pool.acquire();
    auto buf2 = pool.acquire();
    auto buf3 = pool.acquire();
    auto buf4 = pool.acquire();

    pool.release(std::move(buf1));
    pool.release(std::move(buf2));
    pool.release(std::move(buf3));
    pool.release(std::move(buf4));  // 超过限制，不会保留

    REQUIRE(pool.pool_size() == 3);
}

TEST_CASE("buffer_pool_t clear", "[core][memory]") {
    vdl::buffer_pool_t pool(128, 4);

    REQUIRE(pool.pool_size() == 4);

    pool.clear();

    REQUIRE(pool.pool_size() == 0);
}

// ============================================================================
// pooled_buffer_t 测试
// ============================================================================

TEST_CASE("pooled_buffer_t auto release", "[core][memory]") {
    vdl::buffer_pool_t pool(64, 1);

    {
        auto buf = vdl::acquire_buffer(pool);
        REQUIRE(pool.pool_size() == 0);
        REQUIRE(buf.size() == 64);
    }

    // 自动归还
    REQUIRE(pool.pool_size() == 1);
}

TEST_CASE("pooled_buffer_t data access", "[core][memory]") {
    vdl::buffer_pool_t pool(32);

    auto buf = vdl::acquire_buffer(pool);

    // 写入数据
    buf.data()[0] = 0xAA;
    buf.data()[1] = 0xBB;

    REQUIRE(buf.data()[0] == 0xAA);
    REQUIRE(buf.data()[1] == 0xBB);
}

TEST_CASE("pooled_buffer_t span access", "[core][memory]") {
    vdl::buffer_pool_t pool(16);

    auto buf = vdl::acquire_buffer(pool);
    auto span = buf.span();

    REQUIRE(span.size() == 16);
}

TEST_CASE("pooled_buffer_t manual release", "[core][memory]") {
    vdl::buffer_pool_t pool(32, 1);

    vdl::bytes_t extracted;

    {
        auto buf = vdl::acquire_buffer(pool);
        extracted = buf.release();  // 手动释放
    }

    // 不会归还到池
    REQUIRE(pool.pool_size() == 0);
    REQUIRE(extracted.size() == 32);
}

TEST_CASE("pooled_buffer_t move semantics", "[core][memory]") {
    vdl::buffer_pool_t pool(64, 1);

    auto buf1 = vdl::acquire_buffer(pool);
    REQUIRE(pool.pool_size() == 0);

    auto buf2 = std::move(buf1);
    
    // buf1 已失效
    REQUIRE(buf2.size() == 64);
}

// ============================================================================
// 内存操作函数测试
// ============================================================================

TEST_CASE("mem_copy", "[core][memory]") {
    char src[] = "Hello";
    char dest[6] = {0};

    vdl::mem_copy(dest, src, 5);

    REQUIRE(dest[0] == 'H');
    REQUIRE(dest[4] == 'o');
}

TEST_CASE("mem_copy null safety", "[core][memory]") {
    char src[] = "Test";
    
    // 不应该崩溃
    vdl::mem_copy(nullptr, src, 4);
    vdl::mem_copy(src, nullptr, 4);
    vdl::mem_copy(src, src, 0);
}

TEST_CASE("mem_set", "[core][memory]") {
    char buffer[10];

    vdl::mem_set(buffer, 'X', 10);

    for (int i = 0; i < 10; ++i) {
        REQUIRE(buffer[i] == 'X');
    }
}

TEST_CASE("mem_zero", "[core][memory]") {
    char buffer[8] = {1, 2, 3, 4, 5, 6, 7, 8};

    vdl::mem_zero(buffer, 8);

    for (int i = 0; i < 8; ++i) {
        REQUIRE(buffer[i] == 0);
    }
}

TEST_CASE("mem_compare", "[core][memory]") {
    char a[] = "ABC";
    char b[] = "ABC";
    char c[] = "ABD";

    REQUIRE(vdl::mem_compare(a, b, 3) == 0);
    REQUIRE(vdl::mem_compare(a, c, 3) < 0);
    REQUIRE(vdl::mem_compare(c, a, 3) > 0);
}

TEST_CASE("mem_compare null safety", "[core][memory]") {
    char a[] = "Test";

    REQUIRE(vdl::mem_compare(nullptr, a, 4) == 0);
    REQUIRE(vdl::mem_compare(a, nullptr, 4) == 0);
    REQUIRE(vdl::mem_compare(a, a, 0) == 0);
}
