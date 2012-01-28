//
//  ObjectAllocator.cpp
//  Page Allocator
//
//  Created by Samuel Williams on 12/12/11.
//  Copyright (c) 2011 Orion Transfer Ltd. All rights reserved.
//

#include "ObjectAllocator.h"

#include <iostream>
#include "Ensure.h"

#include "Object.h"

// sysconf
#include <unistd.h>

// mmap, munmap
#include <sys/mman.h>

// Compatibilty between Darwin/BSD and Linux
#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

// These are only for debugging - behaviour of memory allocation may be substantially changed/broken.
//#define KAI_MEMORY_DEBUG_GC
//#define KAI_MEMORY_DEBUG_ALLOC
//#define KAI_MEMORY_DEBUG_FREE

namespace Kai {
	namespace Memory {

#ifdef KAI_MEMORY_DEBUG_ALLOC
		static std::size_t _allocation_id = 0;
#endif
		
		std::size_t page_size() {
			static std::size_t _page_size = 0;
			
			if (_page_size == 0) {
				_page_size = sysconf(_SC_PAGESIZE);
			}
			
			return _page_size;
		}
		
		Traversal::~Traversal() {
			
		}
		
#ifdef KAI_MEMORY_STATISTICS
		struct AllocatonStatistics {
			std::size_t total;
			std::size_t used;
			std::size_t freed;
		} g_statistics = {0, 0, 0};
#endif
		
		ObjectAllocation::ObjectAllocation() : _ref_count(0) {
		}
		
		ObjectAllocation::~ObjectAllocation() {
		}
		
		PageAllocation * ObjectAllocation::allocator() const {
			const ObjectAllocation * current = this;
			
			while (!(current->_flags & Memory::BACK)) {				
				current = current->_next;
			}
			
			PageBoundary * boundary = (PageBoundary *)current;
			
			return boundary->_front;
		}
		
		void ObjectAllocation::mark(Memory::Traversal * traversal) const {
		}
				
		std::size_t ObjectAllocation::memory_size() {
			if (_next)
				return (ByteT *)_next - (ByteT *)this;
			else
				return 0;
		}
		
		void ObjectAllocation::retain() const {
			_ref_count += 1;
		}
		
		void ObjectAllocation::release() const {
			_ref_count -= 1;
		}
		
#pragma mark -
		
		FreeAllocation::FreeAllocation(FreeAllocation * next_free) : _next_free(next_free) {
		}
		
		FreeAllocation::~FreeAllocation() {
		}
		
		// 32 bytes in addition to the size of the FreeAllocation header:
		const std::size_t MINIMUM_FREE_ALLOCATION_SIZE = sizeof(FreeAllocation) + 32;
		
		FreeAllocation * FreeAllocation::split(std::size_t size) {
			FreeAllocation * next_free = this->_next_free;
			
			KAI_ENSURE(size <= this->memory_size());
			
			std::size_t remainder = this->memory_size() - size;
			
			if (remainder >= MINIMUM_FREE_ALLOCATION_SIZE) {
				FreeAllocation * middle = new((ByteT *)this + size) FreeAllocation;
				
				middle->_next_free = next_free;
				middle->_next = this->_next;
				middle->_flags = FREE;
				
				this->_next = middle;
				
#ifdef KAI_MEMORY_DEBUG_ALLOC
				KAI_ENSURE(this->_next > this);
				KAI_ENSURE(middle->_next > middle);
				
				this->check();
				middle->check();
				
				if (this->_next)
					middle->_next->check();
#endif
				
				// We created a free block, return it.
				return middle;
			}
			
			// There wasn't enough space to create a free block, return the next free block.
			return next_free;
		}
		
#pragma mark -
		
		PageAllocation::PageAllocation() {
			
		}
		
		PageAllocation::~PageAllocation() {
			
		}
			
		PageAllocation * PageAllocation::create(std::size_t size) {
			void * base = mmap(0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
#ifdef KAI_MEMORY_STATISTICS
			g_statistics.total += size;
#endif
			
//#ifdef KAI_MEMORY_DEBUG
			std::cerr << "Allocating " << size << " bytes, mapped memory at offset " << base << std::endl;
//#endif
			
			PageAllocation * front = new(base) PageAllocation;
			front->_next_page_allocation = NULL;			
			front->_flags = FRONT | USED;
			front->_ref_count = 1;
			
			void * top = (ByteT *)base + size - sizeof(PageBoundary);
			PageBoundary * back = new(top) PageBoundary;
			back->_next = NULL;
			back->_flags = BACK | USED;
			back->_ref_count = 1;
			back->_front = front;
			front->_back = back;
			
			FreeAllocation * free = new((ByteT *)base + sizeof(PageAllocation)) FreeAllocation;
			free->_next = back;
			front->_next = free;
			front->_next_free = free;
			
#ifdef KAI_MEMORY_DEBUG_ALLOC
			front->check();
			back->check();
			free->check();
#endif
			
			return front;
		}
			
		ObjectAllocation * PageAllocation::allocate(std::size_t size) {
			size = calculate_alignment(size, ALIGNMENT);
			
			PageAllocation * base = this;
			FreeAllocation * next_free = NULL;

			while (base) {
				// Does this page allocation have a free list?
				next_free = base->_next_free;
				
				// Traverse the free-list looking for the right sized block:
				while (next_free && next_free->memory_size() < size) {
					next_free = next_free->_next_free;
				}
				
				if (next_free)
					break;
				
				// We got to the end of the free list and didn't find anything, we need to try the next page allocation, or possibly allocate a new one.
				while (!next_free) {
					if (!base->_next_page_allocation) {
						PageAllocation * page_allocation = PageAllocation::create(64 * page_size());
						
						base->_next_page_allocation = page_allocation;
						base->_back->_next = page_allocation;
						base->_next_free = page_allocation->_next_free;
					} else {
						base = base->_next_page_allocation;
					}
					
					next_free = base->_next_free;
				}
			}
			
			ObjectAllocation * allocation = next_free;
			next_free = next_free->split(size);
			
#ifdef KAI_MEMORY_STATISTICS
			g_statistics.used += allocation->memory_size();
#endif
			
			allocation->_flags |= USED;

#ifdef KAI_MEMORY_DEBUG_ALLOC
			_allocation_id += 1;
			
			std::cerr << "Allocated (" << _allocation_id << ") " << size << " bytes at " << allocation << std::endl;
#endif
			base->_next_free = next_free;
			
			return allocation;
		}
		
		void PageAllocation::deallocate(ObjectAllocation * start, ObjectAllocation * end) {			
			// Initialize a new free block in this segment:
			FreeAllocation * free_allocation = new(start) FreeAllocation;
			free_allocation->_flags ^= USED;
			
			// The next block is the block past the end:
			free_allocation->_next = end->_next;
			
#ifdef KAI_MEMORY_DEBUG_DEALLOC
			std::cerr << "Deallocating range from " << start << " -> " << end->_next << "(" << free_allocation->memory_size() << " bytes)" << std::endl;
#endif
			
			// Insert the free block into the free list:
			free_allocation->_next_free = this->_next_free;
			this->_next_free = free_allocation;
			
#ifdef KAI_MEMORY_STATISTICS
			g_statistics.freed += free_allocation->memory_size();
			g_statistics.used -= free_allocation->memory_size();
#endif
		}
		
		bool PageAllocation::includes(const ObjectAllocation * allocation) {
			KAI_ENSURE(this->_flags & FRONT);
			
			if (allocation >= this && allocation <= this->_back) {
				return true;
			}
			
			return false;
		}
		
		PageAllocation * PageAllocation::base_of(const ObjectAllocation * allocation) {
			PageAllocation * base = this;
			
			while (base) {
				if (base->includes(allocation)) return base;
				
				base = base->_next_page_allocation;
			}
			
			return NULL;
		}
		
		void PageAllocation::debug() {
#ifdef KAI_MEMORY_STATISTICS
			std::cerr << "Total: " << g_statistics.total << " Used: " << g_statistics.used << " Freed: " << g_statistics.freed << std::endl;
#endif
			
			ObjectAllocation * current = this;
			while (current) {
				std::cerr << "[" << current << "(" << current->_flags << ")" << " + " << current->memory_size() << "] -> " << current->_next << std::endl;
								
				current = current->_next;
			}
		}
				
#pragma mark -
		
		PageBoundary::PageBoundary() {
			
		}
		
		PageBoundary::~PageBoundary() {
			
		}
		
	}	
}
