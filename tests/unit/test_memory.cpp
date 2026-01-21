#include "catch.hpp"
#include "vdl/core/memory.hpp"
#include "vdl/core/error.hpp"
#include <vector>

TEST_CASE("Memory statistics initialization", "[memory][stats]") {
    vdl::MemoryStats stats;
    REQUIRE(stats.total_allocated == 0);
    REQUIRE(stats.total_deallocated == 0);
    REQUIRE(stats.current_allocated == 0);
    REQUIRE(stats.peak_allocated == 0);
    REQUIRE(stats.allocation_count == 0);
    REQUIRE(stats.deallocation_count == 0);
}

TEST_CASE("StandardAllocator basic operations", "[memory][allocator]") {
    vdl::StandardAllocator allocator;

    SECTION("Allocate single block") {
        void* ptr = allocator.allocate(100);
        REQUIRE(ptr != nullptr);
        
        vdl::MemoryStats stats = allocator.getStats();
        REQUIRE(stats.total_allocated == 100);
        REQUIRE(stats.current_allocated == 100);
        REQUIRE(stats.allocation_count == 1);
        
        delete[] static_cast<char*>(ptr);
    }

    SECTION("Multiple allocations") {
        void* ptr1 = allocator.allocate(50);
        void* ptr2 = allocator.allocate(75);
        void* ptr3 = allocator.allocate(100);
        
        vdl::MemoryStats stats = allocator.getStats();
        REQUIRE(stats.total_allocated == 225);
        REQUIRE(stats.current_allocated == 225);
        REQUIRE(stats.peak_allocated == 225);
        REQUIRE(stats.allocation_count == 3);
        
        delete[] static_cast<char*>(ptr1);
        delete[] static_cast<char*>(ptr2);
        delete[] static_cast<char*>(ptr3);
    }

    SECTION("Zero size allocation fails") {
        bool caught_error = false;
        try {
            allocator.allocate(0);
        } catch (const vdl::VdlException&) {
            caught_error = true;
        }
        REQUIRE(caught_error);
    }

    SECTION("Reset statistics") {
        allocator.allocate(100);
        allocator.resetStats();
        
        vdl::MemoryStats stats = allocator.getStats();
        REQUIRE(stats.total_allocated == 0);
        REQUIRE(stats.current_allocated == 0);
        REQUIRE(stats.allocation_count == 0);
    }
}

TEST_CASE("StackAllocator functionality", "[memory][stack]") {
    SECTION("Internal buffer allocation") {
        vdl::StackAllocator allocator(1024);
        
        void* ptr1 = allocator.allocate(256);
        void* ptr2 = allocator.allocate(256);
        
        REQUIRE(ptr1 != nullptr);
        REQUIRE(ptr2 != nullptr);
        REQUIRE(allocator.getUsage() == 512);
        REQUIRE(allocator.getRemaining() == 512);
    }

    SECTION("External buffer allocation") {
        char buffer[512];
        vdl::StackAllocator allocator(buffer, 512);
        
        void* ptr1 = allocator.allocate(128);
        REQUIRE(ptr1 == buffer);
        
        void* ptr2 = allocator.allocate(128);
        REQUIRE(ptr2 == buffer + 128);
    }

    SECTION("Stack exhaustion") {
        vdl::StackAllocator allocator(256);
        
        allocator.allocate(200);
        bool caught_error = false;
        try {
            allocator.allocate(100);
        } catch (const vdl::VdlException& ex) {
            caught_error = (ex.getErrorCode() == vdl::ErrorCode::OutOfMemory);
        }
        REQUIRE(caught_error);
    }

    SECTION("Stack reset") {
        vdl::StackAllocator allocator(1024);
        
        allocator.allocate(256);
        allocator.allocate(256);
        REQUIRE(allocator.getUsage() == 512);
        
        allocator.reset();
        REQUIRE(allocator.getUsage() == 0);
        
        void* ptr3 = allocator.allocate(128);
        REQUIRE(ptr3 != nullptr);
    }

    SECTION("Statistics tracking") {
        vdl::StackAllocator allocator(1024);
        
        allocator.allocate(100);
        allocator.allocate(200);
        
        vdl::MemoryStats stats = allocator.getStats();
        REQUIRE(stats.total_allocated == 300);
        REQUIRE(stats.current_allocated == 300);
        REQUIRE(stats.peak_allocated == 300);
        REQUIRE(stats.allocation_count == 2);
    }
}

TEST_CASE("MemoryPool fixed-size allocations", "[memory][pool]") {
    SECTION("Pool creation and allocation") {
        vdl::MemoryPool pool(128, 10);
        
        void* ptr1 = pool.allocate(100);
        void* ptr2 = pool.allocate(100);
        
        REQUIRE(ptr1 != nullptr);
        REQUIRE(ptr2 != nullptr);
        REQUIRE(pool.getFreeBlocks() == 8);
        REQUIRE(pool.getTotalBlocks() == 10);
    }

    SECTION("Pool deallocation") {
        vdl::MemoryPool pool(128, 5);
        
        void* ptr1 = pool.allocate(100);
        void* ptr2 = pool.allocate(100);
        (void)ptr2;  // Avoid unused variable warning
        
        REQUIRE(pool.getFreeBlocks() == 3);
        
        pool.deallocate(ptr1);
        REQUIRE(pool.getFreeBlocks() == 4);
    }

    SECTION("Pool exhaustion") {
        vdl::MemoryPool pool(128, 2);
        
        pool.allocate(100);
        pool.allocate(100);
        
        bool caught_error = false;
        try {
            pool.allocate(100);
        } catch (const vdl::VdlException& ex) {
            caught_error = (ex.getErrorCode() == vdl::ErrorCode::OutOfMemory);
        }
        REQUIRE(caught_error);
    }

    SECTION("Size validation") {
        vdl::MemoryPool pool(128, 5);
        
        bool caught_error = false;
        try {
            pool.allocate(200);  // Exceeds block size
        } catch (const vdl::VdlException& ex) {
            caught_error = (ex.getErrorCode() == vdl::ErrorCode::Invalid);
        }
        REQUIRE(caught_error);
    }

    SECTION("Pool statistics") {
        vdl::MemoryPool pool(64, 8);
        
        pool.allocate(50);
        pool.allocate(50);
        pool.allocate(50);
        
        vdl::MemoryStats stats = pool.getStats();
        REQUIRE(stats.total_allocated == 192);  // 3 * 64
        REQUIRE(stats.current_allocated == 192);
        REQUIRE(stats.allocation_count == 3);
    }

    SECTION("Invalid pointer deallocation") {
        vdl::MemoryPool pool(128, 5);
        
        char dummy_buffer[128];
        
        bool caught_error = false;
        try {
            pool.deallocate(dummy_buffer);
        } catch (const vdl::VdlException& ex) {
            caught_error = (ex.getErrorCode() == vdl::ErrorCode::Invalid);
        }
        REQUIRE(caught_error);
    }
}

TEST_CASE("MemoryManager global operations", "[memory][manager]") {
    SECTION("Default allocator") {
        vdl::MemoryManager::resetToStandardAllocator();
        
        vdl::Allocator* allocator = vdl::MemoryManager::getDefaultAllocator();
        REQUIRE(allocator != nullptr);
    }

    SECTION("Global allocation") {
        vdl::MemoryManager::resetToStandardAllocator();
        vdl::MemoryManager::resetStats();
        
        void* ptr = vdl::MemoryManager::allocate(256);
        REQUIRE(ptr != nullptr);
        
        vdl::MemoryStats stats = vdl::MemoryManager::getStats();
        REQUIRE(stats.allocation_count == 1);
        
        delete[] static_cast<char*>(ptr);
    }

    SECTION("Custom allocator") {
        vdl::MemoryPool pool(64, 10);
        vdl::MemoryManager::setDefaultAllocator(&pool);
        
        void* ptr = vdl::MemoryManager::allocate(50);
        REQUIRE(ptr != nullptr);
        
        vdl::MemoryManager::deallocate(ptr);
        vdl::MemoryManager::resetToStandardAllocator();
    }

    SECTION("Global statistics reset") {
        vdl::MemoryManager::resetToStandardAllocator();
        
        vdl::MemoryManager::allocate(100);
        vdl::MemoryManager::allocate(100);
        vdl::MemoryManager::resetStats();
        
        vdl::MemoryStats stats = vdl::MemoryManager::getStats();
        REQUIRE(stats.allocation_count == 0);
    }
}

TEST_CASE("MemoryGuard RAII protection", "[memory][guard]") {
    SECTION("Guard releases on destruction") {
        vdl::StandardAllocator allocator;
        
        {
            void* ptr = allocator.allocate(100);
            vdl::MemoryGuard guard(ptr, &allocator);
            REQUIRE(guard.valid());
            REQUIRE(guard.get() != nullptr);
        }
        
        // Guard destructor should deallocate
    }

    SECTION("Guard release ownership") {
        vdl::StandardAllocator allocator;
        
        void* ptr = allocator.allocate(100);
        vdl::MemoryGuard guard(ptr, &allocator);
        
        void* released = guard.release();
        REQUIRE(released == ptr);
        REQUIRE(!guard.valid());
        
        delete[] static_cast<char*>(ptr);
    }

    SECTION("Guard with null pointer") {
        vdl::MemoryGuard guard(nullptr, nullptr);
        REQUIRE(!guard.valid());
    }

    SECTION("Guard with default allocator") {
        vdl::MemoryManager::resetToStandardAllocator();
        
        void* ptr = vdl::MemoryManager::allocate(100);
        vdl::MemoryGuard guard(ptr, nullptr);
        REQUIRE(guard.valid());
    }
}

TEST_CASE("Memory allocation patterns", "[memory][patterns]") {
    SECTION("Sequential allocation pattern") {
        vdl::StackAllocator allocator(1024);
        std::vector<void*> ptrs;
        
        for (int i = 0; i < 10; ++i) {
            ptrs.push_back(allocator.allocate(50));
        }
        
        REQUIRE(allocator.getUsage() == 500);
        vdl::MemoryStats stats = allocator.getStats();
        REQUIRE(stats.allocation_count == 10);
    }

    SECTION("Pool-based pattern") {
        vdl::MemoryPool pool(128, 20);
        std::vector<void*> ptrs;
        
        // Allocate
        for (int i = 0; i < 15; ++i) {
            ptrs.push_back(pool.allocate(100));
        }
        REQUIRE(pool.getFreeBlocks() == 5);
        
        // Deallocate
        for (void* ptr : ptrs) {
            pool.deallocate(ptr);
        }
        REQUIRE(pool.getFreeBlocks() == 20);
    }

    SECTION("Memory efficiency comparison") {
        vdl::StandardAllocator std_alloc;
        vdl::MemoryPool pool(256, 20);
        
        // Pool pre-allocates all memory
        vdl::MemoryStats pool_stats = pool.getStats();
        REQUIRE(pool_stats.peak_allocated == 0);  // Not allocated yet
        
        // Allocate from pool
        pool.allocate(200);
        pool.allocate(200);
        
        vdl::MemoryStats after = pool.getStats();
        REQUIRE(after.allocation_count == 2);
    }
}

TEST_CASE("Memory error handling", "[memory][error]") {
    SECTION("Allocation failure handling") {
        vdl::StandardAllocator allocator;
        
        bool caught_error = false;
        try {
            allocator.allocate(0);
        } catch (const vdl::VdlException& ex) {
            caught_error = (ex.getErrorCode() == vdl::ErrorCode::Invalid);
        }
        REQUIRE(caught_error);
    }

    SECTION("Stack overflow handling") {
        vdl::StackAllocator allocator(100);
        
        bool caught_error = false;
        try {
            allocator.allocate(200);
        } catch (const vdl::VdlException& ex) {
            caught_error = (ex.getErrorCode() == vdl::ErrorCode::OutOfMemory);
        }
        REQUIRE(caught_error);
    }

    SECTION("Pool exhaustion handling") {
        vdl::MemoryPool pool(128, 1);
        
        pool.allocate(100);
        
        bool caught_error = false;
        try {
            pool.allocate(100);
        } catch (const vdl::VdlException& ex) {
            caught_error = (ex.getErrorCode() == vdl::ErrorCode::OutOfMemory);
        }
        REQUIRE(caught_error);
    }

    SECTION("Invalid size handling") {
        vdl::MemoryPool pool(256, 5);
        
        bool caught_error = false;
        try {
            pool.allocate(512);  // Exceeds block size
        } catch (const vdl::VdlException& ex) {
            caught_error = (ex.getErrorCode() == vdl::ErrorCode::Invalid);
        }
        REQUIRE(caught_error);
    }
}

TEST_CASE("Memory integration with logging and error", "[memory][integration]") {
    SECTION("Error reporting with context") {
        vdl::MemoryPool pool(256, 2);
        
        try {
            pool.allocate(100);
            pool.allocate(100);
            pool.allocate(100);  // Should fail - pool exhausted
        } catch (const vdl::VdlException& ex) {
            REQUIRE(ex.getErrorCode() == vdl::ErrorCode::OutOfMemory);
            std::string msg = ex.getFullMessage();
            REQUIRE(msg.length() > 0);
        }
    }

    SECTION("Memory tracking across operations") {
        vdl::StackAllocator allocator(2048);
        
        // Simulate device operation sequence
        void* config = allocator.allocate(256);
        void* buffer1 = allocator.allocate(512);
        void* buffer2 = allocator.allocate(512);
        (void)config;    // Avoid unused variable warning
        (void)buffer1;
        (void)buffer2;
        
        vdl::MemoryStats stats = allocator.getStats();
        REQUIRE(stats.allocation_count == 3);
        REQUIRE(stats.current_allocated == 1280);
        
        allocator.reset();
        REQUIRE(allocator.getUsage() == 0);
    }

    SECTION("Memory guard with custom allocator") {
        vdl::MemoryPool pool(128, 10);
        
        {
            void* ptr = pool.allocate(100);
            vdl::MemoryGuard guard(ptr, &pool);
            REQUIRE(guard.valid());
        }
        
        // Verify deallocation
        REQUIRE(pool.getFreeBlocks() == 10);
    }
}

TEST_CASE("Memory statistics accuracy", "[memory][stats-accuracy]") {
    SECTION("Peak tracking") {
        vdl::StandardAllocator allocator;
        
        allocator.allocate(100);
        allocator.allocate(200);
        vdl::MemoryStats stats1 = allocator.getStats();
        REQUIRE(stats1.peak_allocated == 300);
    }

    SECTION("Deallocation counting") {
        vdl::MemoryPool pool(128, 5);
        
        void* ptr1 = pool.allocate(100);
        void* ptr2 = pool.allocate(100);
        
        pool.deallocate(ptr1);
        pool.deallocate(ptr2);
        
        vdl::MemoryStats stats = pool.getStats();
        REQUIRE(stats.deallocation_count == 2);
    }

    SECTION("Multiple allocator stats independence") {
        vdl::StandardAllocator alloc1;
        vdl::StandardAllocator alloc2;
        
        alloc1.allocate(100);
        alloc2.allocate(200);
        
        vdl::MemoryStats stats1 = alloc1.getStats();
        vdl::MemoryStats stats2 = alloc2.getStats();
        
        REQUIRE(stats1.total_allocated == 100);
        REQUIRE(stats2.total_allocated == 200);
    }
}
