#pragma once

#include <ArduinoJson.h>
#include <esp_heap_caps.h>
#include <new>

struct SpiRamAllocator : ArduinoJson::Allocator {
    void* allocate(size_t size) override;
    void* reallocate(void* ptr, size_t size) override;
    void deallocate(void* ptr) override;
};

extern SpiRamAllocator spiRamAllocator;

template <typename T>
struct SPIRamAllocator {
    using value_type = T;

    T* allocate(std::size_t n) {
        void* p = spiRamAllocator.allocate(n * sizeof(T));
        if (!p) throw std::bad_alloc();
        return static_cast<T*>(p);
    }

    void deallocate(T* p, std::size_t) noexcept {
        spiRamAllocator.deallocate(p);
    }
};