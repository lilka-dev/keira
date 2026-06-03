#include "mem.h"

// ── Global instance ───────────────────────────────────────────────────────────
SpiRamAllocator spiRamAllocator;

// ── SpiRamAllocator method definitions ───────────────────────────────────────
void* SpiRamAllocator::allocate(size_t size) {
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
}

void* SpiRamAllocator::reallocate(void* ptr, size_t size) {
    return heap_caps_realloc(ptr, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
}

void SpiRamAllocator::deallocate(void* ptr) {
    heap_caps_free(ptr);
}