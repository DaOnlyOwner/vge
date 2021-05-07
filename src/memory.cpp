#include "memory.h"

#include <cstdlib>

struct LinearArenaHeader {
	uint8_t *end;
};

static uint8_t* align_ptr(uint8_t *ptr, uint64_t align) {
	return reinterpret_cast<uint8_t*>(reinterpret_cast<uint64_t>(ptr + align - 1) & (~(align - 1)));
}

static void* allocate_from_linear_arena(MemoryArena *arena, int64_t size, uint64_t align) {
	auto header = reinterpret_cast<LinearArenaHeader*>(arena->memory);

	auto result = align_ptr(header->end, align);
	auto nEnd = result + size;

	if (nEnd - arena->memory > arena->size) {
		// Ran out of memory
		return nullptr;
	}

	header->end = nEnd;

	return result;

}

static void deallocate_from_linear_arena(MemoryArena *arena, void *ptr, int64_t size) {
	// Do nothing
}

static void clear_linear_arena(MemoryArena *arena) {
	auto header = reinterpret_cast<LinearArenaHeader*>(arena->memory);
	header->end = arena->memory + sizeof(LinearArenaHeader);
}

Allocator make_linear_allocator(int64_t size) {
	Allocator result;
	result.arena.memory = static_cast<uint8_t*>(malloc(size));
	result.arena.size = size;
	result.allocFn = &allocate_from_linear_arena;
	result.freeFn = &deallocate_from_linear_arena;

	// Fill in header
	auto header = reinterpret_cast<LinearArenaHeader*>(result.arena.memory);
	header->end = result.arena.memory + sizeof(LinearArenaHeader);

	return result;
}

void destroy_linear_allocator(Allocator *allocator) {
	free(allocator->arena.memory);
	*allocator = {};
}

void init_temporary_allocator(int64_t size) {
	temporaryAllocator = static_cast<Allocator*>(malloc(sizeof(Allocator)));
	*temporaryAllocator = make_linear_allocator(size);
}

Allocator* temporaryAllocator;
