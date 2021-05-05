#pragma once

#include <cinttypes>

struct MemoryArena {
	uint8_t *memory; // TODO: This is gonna alias all sorts of stuff, add restrict define for multiple platforms and use it here
	int64_t size;
};

struct Allocator {
	MemoryArena arena;

	void* (*allocFn)(MemoryArena *arena, int64_t, uint64_t);
	void (*freeFn)(MemoryArena *arena, void *, int64_t);
};

Allocator make_linear_allocator(int64_t size);
void destroy_linear_allocator(Allocator *allocator);

void init_temporary_allocator(int64_t size);

template<typename T>
T* allocate(Allocator *allocator, int64_t count) {
	return static_cast<T*>(allocator->allocFn(&allocator->arena, sizeof(T) * count, alignof(T)));
}

template<typename T>
void deallocate(Allocator *allocator, T* ptr, int64_t count) {
	allocator->freeFn(&allocator->arena, ptr, sizeof(T) * count);
}

extern Allocator* temporaryAllocator;
