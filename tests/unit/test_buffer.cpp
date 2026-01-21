#include "catch.hpp"
#include "vdl/core/buffer.hpp"
#include "vdl/core/memory.hpp"
#include <string>
#include <cstring>

TEST_CASE("CircularBuffer basic operations", "[buffer][circular]") {
    SECTION("Create and write") {
        vdl::CircularBuffer buf(256);
        
        uint8_t data[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        size_t written = buf.write(data, 10);
        
        REQUIRE(written == 10);
        REQUIRE(buf.getAvailableSize() == 10);
        REQUIRE(buf.getRemainingCapacity() == 246);
    }

    SECTION("Write and read") {
        vdl::CircularBuffer buf(256);
        
        uint8_t data[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        buf.write(data, 10);
        
        uint8_t read_data[10];
        size_t read_size = buf.read(read_data, 10);
        
        REQUIRE(read_size == 10);
        REQUIRE(buf.getAvailableSize() == 0);
        REQUIRE(std::memcmp(data, read_data, 10) == 0);
    }

    SECTION("Buffer wrapping") {
        vdl::CircularBuffer buf(10);
        
        // Fill buffer
        uint8_t data1[8] = {1, 2, 3, 4, 5, 6, 7, 8};
        buf.write(data1, 8);
        
        // Read 5 bytes
        uint8_t read_data[5];
        buf.read(read_data, 5);
        
        // Write 5 more bytes (should wrap)
        uint8_t data2[5] = {9, 10, 11, 12, 13};
        size_t written = buf.write(data2, 5);
        
        REQUIRE(written == 5);
        REQUIRE(buf.getAvailableSize() == 8);
    }

    SECTION("Peek without consuming") {
        vdl::CircularBuffer buf(256);
        
        uint8_t data[5] = {1, 2, 3, 4, 5};
        buf.write(data, 5);
        
        uint8_t peek_data[5];
        size_t peeked = buf.peek(peek_data, 5);
        
        REQUIRE(peeked == 5);
        REQUIRE(buf.getAvailableSize() == 5);  // Data still in buffer
        REQUIRE(std::memcmp(data, peek_data, 5) == 0);
    }

    SECTION("Clear buffer") {
        vdl::CircularBuffer buf(256);
        
        uint8_t data[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        buf.write(data, 10);
        
        buf.clear();
        
        REQUIRE(buf.isEmpty());
        REQUIRE(buf.getAvailableSize() == 0);
        REQUIRE(buf.getRemainingCapacity() == 256);
    }

    SECTION("Skip bytes") {
        vdl::CircularBuffer buf(256);
        
        uint8_t data[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        buf.write(data, 10);
        
        buf.skip(3);
        
        REQUIRE(buf.getAvailableSize() == 7);
        
        uint8_t read_data[7];
        buf.read(read_data, 7);
        
        REQUIRE(read_data[0] == 4);  // Should skip to 4th element
    }

    SECTION("Buffer full condition") {
        vdl::CircularBuffer buf(10);
        
        uint8_t data[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        buf.write(data, 10);
        
        REQUIRE(buf.isFull());
        REQUIRE(buf.getRemainingCapacity() == 0);
    }

    SECTION("Partial write when full") {
        vdl::CircularBuffer buf(10);
        
        uint8_t data1[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        buf.write(data1, 10);
        
        uint8_t data2[5] = {11, 12, 13, 14, 15};
        size_t written = buf.write(data2, 5);
        
        REQUIRE(written == 0);  // No space
    }
}

TEST_CASE("CircularBuffer edge cases", "[buffer][circular-edge]") {
    SECTION("Empty buffer read") {
        vdl::CircularBuffer buf(256);
        
        uint8_t data[10];
        size_t read = buf.read(data, 10);
        
        REQUIRE(read == 0);
        REQUIRE(buf.isEmpty());
    }

    SECTION("Multiple small writes") {
        vdl::CircularBuffer buf(100);
        
        for (int i = 0; i < 10; ++i) {
            uint8_t byte = static_cast<uint8_t>(i);
            buf.write(&byte, 1);
        }
        
        REQUIRE(buf.getAvailableSize() == 10);
    }

    SECTION("Interleaved write and read") {
        vdl::CircularBuffer buf(10);
        
        // Write 3
        uint8_t data1[3] = {1, 2, 3};
        buf.write(data1, 3);
        
        // Read 1
        uint8_t read1;
        buf.read(&read1, 1);
        REQUIRE(read1 == 1);
        
        // Write 3 more
        uint8_t data2[3] = {4, 5, 6};
        buf.write(data2, 3);
        
        // Read remaining: should have 2, 3, 4, 5, 6 (5 bytes)
        uint8_t remaining[5];
        size_t read_size = buf.read(remaining, 5);
        REQUIRE(read_size == 5);
        REQUIRE(remaining[0] == 2);
        REQUIRE(remaining[1] == 3);
        REQUIRE(remaining[2] == 4);
        REQUIRE(remaining[3] == 5);
        REQUIRE(remaining[4] == 6);
    }

    SECTION("External buffer") {
        uint8_t ext_buf[64];
        vdl::CircularBuffer buf(ext_buf, 64);
        
        uint8_t data[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        buf.write(data, 10);
        
        REQUIRE(buf.getAvailableSize() == 10);
    }
}

TEST_CASE("RingBuffer template operations", "[buffer][ring]") {
    SECTION("Push and pop integers") {
        vdl::RingBuffer<int> ring(10);
        
        REQUIRE(ring.push(42));
        REQUIRE(ring.push(100));
        REQUIRE(ring.size() == 2);
        
        int value;
        REQUIRE(ring.pop(value));
        REQUIRE(value == 42);
        REQUIRE(ring.size() == 1);
    }

    SECTION("Front and back access") {
        vdl::RingBuffer<int> ring(10);
        
        ring.push(10);
        ring.push(20);
        ring.push(30);
        
        int front, back;
        REQUIRE(ring.front(front));
        REQUIRE(ring.back(back));
        
        REQUIRE(front == 10);
        REQUIRE(back == 30);
    }

    SECTION("Index access") {
        vdl::RingBuffer<int> ring(10);
        
        ring.push(100);
        ring.push(200);
        ring.push(300);
        
        int val0, val1, val2;
        REQUIRE(ring.at(0, val0));
        REQUIRE(ring.at(1, val1));
        REQUIRE(ring.at(2, val2));
        
        REQUIRE(val0 == 100);
        REQUIRE(val1 == 200);
        REQUIRE(val2 == 300);
    }

    SECTION("Ring wrapping with pop") {
        vdl::RingBuffer<int> ring(5);
        
        ring.push(1);
        ring.push(2);
        ring.push(3);
        ring.push(4);
        ring.push(5);
        
        int val;
        ring.pop(val);  // Pop 1
        ring.push(6);
        
        int at0;
        REQUIRE(ring.at(0, at0));
        REQUIRE(at0 == 2);
    }

    SECTION("Full buffer detection") {
        vdl::RingBuffer<int> ring(3);
        
        REQUIRE(ring.push(1));
        REQUIRE(ring.push(2));
        REQUIRE(ring.push(3));
        REQUIRE(!ring.push(4));  // Full
        REQUIRE(ring.full());
    }

    SECTION("Clear buffer") {
        vdl::RingBuffer<int> ring(10);
        
        ring.push(1);
        ring.push(2);
        ring.push(3);
        
        ring.clear();
        
        REQUIRE(ring.empty());
        REQUIRE(ring.size() == 0);
    }

    SECTION("Ring buffer with strings") {
        vdl::RingBuffer<std::string> ring(5);
        
        ring.push("hello");
        ring.push("world");
        
        std::string val;
        REQUIRE(ring.front(val));
        REQUIRE(val == "hello");
        
        ring.pop(val);
        REQUIRE(val == "hello");
    }
}

TEST_CASE("BufferPool management", "[buffer][pool]") {
    SECTION("Acquire and release") {
        vdl::BufferPool pool(256, 4);
        
        uint8_t* buf1 = pool.acquire();
        uint8_t* buf2 = pool.acquire();
        
        REQUIRE(buf1 != nullptr);
        REQUIRE(buf2 != nullptr);
        REQUIRE(buf1 != buf2);
        REQUIRE(pool.getAvailableCount() == 2);
    }

    SECTION("Pool exhaustion") {
        vdl::BufferPool pool(256, 2);
        
        uint8_t* buf1 = pool.acquire();
        uint8_t* buf2 = pool.acquire();
        uint8_t* buf3 = pool.acquire();
        
        REQUIRE(buf1 != nullptr);
        REQUIRE(buf2 != nullptr);
        REQUIRE(buf3 == nullptr);  // No more buffers
    }

    SECTION("Buffer reuse") {
        vdl::BufferPool pool(256, 2);
        
        uint8_t* buf1 = pool.acquire();
        pool.release(buf1);
        
        uint8_t* buf2 = pool.acquire();
        
        REQUIRE(buf1 == buf2);  // Reused same buffer
    }

    SECTION("Invalid release") {
        vdl::BufferPool pool(256, 2);
        
        uint8_t dummy[256];
        bool released = pool.release(dummy);
        
        REQUIRE(!released);  // Not in pool
    }

    SECTION("Pool reset") {
        vdl::BufferPool pool(256, 2);
        
        uint8_t* buf1 = pool.acquire();
        uint8_t* buf2 = pool.acquire();
        (void)buf1;  // Avoid unused variable warning
        (void)buf2;
        
        REQUIRE(pool.getAvailableCount() == 0);
        
        pool.reset();
        
        REQUIRE(pool.getAvailableCount() == 2);
    }

    SECTION("Pool size queries") {
        vdl::BufferPool pool(512, 8);
        
        REQUIRE(pool.getBufferSize() == 512);
        REQUIRE(pool.getTotalCount() == 8);
        REQUIRE(pool.getAvailableCount() == 8);
    }
}

TEST_CASE("BufferGuard RAII protection", "[buffer][guard]") {
    SECTION("Automatic release") {
        vdl::BufferPool pool(256, 2);
        
        {
            uint8_t* buf = pool.acquire();
            vdl::BufferGuard guard(buf, &pool);
            REQUIRE(guard.valid());
            REQUIRE(pool.getAvailableCount() == 1);
        }
        
        REQUIRE(pool.getAvailableCount() == 2);
    }

    SECTION("Guard release") {
        vdl::BufferPool pool(256, 2);
        
        uint8_t* buf = pool.acquire();
        vdl::BufferGuard guard(buf, &pool);
        
        uint8_t* released = guard.release();
        REQUIRE(released == buf);
        REQUIRE(!guard.valid());
        
        pool.release(buf);
    }

    SECTION("Null guard") {
        vdl::BufferGuard guard(nullptr, nullptr);
        REQUIRE(!guard.valid());
    }
}

TEST_CASE("Buffer data preservation", "[buffer][data]") {
    SECTION("Write/read string data") {
        vdl::CircularBuffer buf(256);
        
        std::string original = "Hello, World!";
        buf.write(original.c_str(), original.size());
        
        uint8_t read_buf[256];
        buf.read(read_buf, original.size());
        
        std::string result(reinterpret_cast<char*>(read_buf), original.size());
        REQUIRE(result == original);
    }

    SECTION("Multiple writes and reads") {
        vdl::CircularBuffer buf(1024);
        
        for (int i = 0; i < 10; ++i) {
            uint8_t data[32];
            for (int j = 0; j < 32; ++j) {
                data[j] = static_cast<uint8_t>((i * 32 + j) % 256);
            }
            buf.write(data, 32);
        }
        
        REQUIRE(buf.getAvailableSize() == 320);
        
        for (int i = 0; i < 10; ++i) {
            uint8_t read_buf[32];
            buf.read(read_buf, 32);
            
            for (int j = 0; j < 32; ++j) {
                REQUIRE(read_buf[j] == static_cast<uint8_t>((i * 32 + j) % 256));
            }
        }
    }
}

TEST_CASE("Buffer integration tests", "[buffer][integration]") {
    SECTION("Circular buffer with memory allocator") {
        vdl::MemoryManager::resetToStandardAllocator();
        vdl::MemoryManager::resetStats();
        
        vdl::CircularBuffer buf(512);
        
        uint8_t data[256];
        for (int i = 0; i < 256; ++i) {
            data[i] = static_cast<uint8_t>(i % 256);
        }
        
        buf.write(data, 256);
        uint8_t read_buf[256];
        buf.read(read_buf, 256);
        
        REQUIRE(std::memcmp(data, read_buf, 256) == 0);
    }

    SECTION("Ring buffer with complex data types") {
        struct Point {
            int x, y;
            Point(int x = 0, int y = 0) : x(x), y(y) {}
            bool operator==(const Point& other) const {
                return x == other.x && y == other.y;
            }
        };
        
        vdl::RingBuffer<Point> ring(10);
        
        ring.push(Point(1, 2));
        ring.push(Point(3, 4));
        ring.push(Point(5, 6));
        
        Point p;
        ring.front(p);
        REQUIRE(p == Point(1, 2));
        
        ring.pop(p);
        REQUIRE(p == Point(1, 2));
    }

    SECTION("Buffer pool with custom allocator") {
        vdl::MemoryPool mem_pool(1024, 5);
        
        vdl::BufferPool buf_pool(256, 3, &mem_pool);
        
        uint8_t* buf1 = buf_pool.acquire();
        REQUIRE(buf1 != nullptr);
        
        buf_pool.release(buf1);
        REQUIRE(buf_pool.getAvailableCount() == 3);
    }
}

TEST_CASE("Buffer stress tests", "[buffer][stress]") {
    SECTION("Large sequential writes") {
        vdl::CircularBuffer buf(4096);
        
        uint8_t data[100];
        for (int i = 0; i < 100; ++i) {
            data[i] = static_cast<uint8_t>(i % 256);
        }
        
        buf.write(data, 100);
        REQUIRE(buf.getAvailableSize() == 100);
    }

    SECTION("Ring buffer capacity limits") {
        vdl::RingBuffer<int> ring(100);
        
        for (int i = 0; i < 100; ++i) {
            REQUIRE(ring.push(i));
        }
        
        REQUIRE(ring.full());
        REQUIRE(!ring.push(100));
    }

    SECTION("Peek performance") {
        vdl::CircularBuffer buf(10000);
        
        uint8_t data[1000];
        std::memset(data, 0, 1000);  // Initialize buffer
        buf.write(data, 1000);
        
        uint8_t peek_buf[1000];
        size_t peeked = buf.peek(peek_buf, 1000);
        
        REQUIRE(peeked == 1000);
        REQUIRE(buf.getAvailableSize() == 1000);  // Data unchanged
    }
}
