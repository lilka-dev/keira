#include "mem.h"

// ── Global instance ───────────────────────────────────────────────────────────
SpiRamAllocator spiRamAllocator;

// ── SpiRamAllocator method definitions ───────────────────────────────────────
void* SpiRamAllocator::allocate(size_t size) {
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
}

void* SpiRamAllocator::reallocate(void* ptr, size_t size) {
    void* p = heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (p && ptr) memcpy(p, ptr, size);
    heap_caps_free(ptr);
    return p;
}

void SpiRamAllocator::deallocate(void* ptr) {
    heap_caps_free(ptr);
}