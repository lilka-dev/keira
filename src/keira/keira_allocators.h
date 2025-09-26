#pragma once
///////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Keira OS Header file
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cstddef>
#include <new>
#include <esp_heap_caps.h>

// Allocator to specify SPIRAM as a storage
// Example usage together with vector :
//   std::vector<$TYPE, SPIRamAllocator<$TYPE>> vec;
// like
//   std::vector<int, SPIRamAllocator<int>> vec;
// Theoretically we could create different allocators for different memory types
// by adjusting heap_caps_malloc params

// Note: this won't fix your problems with String :D
template <class T>
struct SPIRamAllocator {
    using value_type = T;

    T* allocate(std::size_t n) {
        void* p = heap_caps_malloc(n * sizeof(T), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        if (!p) throw std::bad_alloc();
        return static_cast<T*>(p);
    }

    void deallocate(T* p, std::size_t) noexcept {
        heap_caps_free(p);
    }
};
