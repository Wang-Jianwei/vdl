#include "vdl/core/memory.hpp"
#include <cstring>

namespace vdl {

// Static member initialization
Allocator* MemoryManager::default_allocator_ = nullptr;
StandardAllocator MemoryManager::standard_allocator_;

// Initialize on first use
namespace {
    struct MemoryManagerInitializer {
        MemoryManagerInitializer() {
            if (MemoryManager::default_allocator_ == nullptr) {
                MemoryManager::setDefaultAllocator(&MemoryManager::standard_allocator_);
            }
        }
    };
    
    static MemoryManagerInitializer initializer;
}

} // namespace vdl
