//
//  ObjectAllocator.cpp
//  Page Allocator
//
//  Created by Samuel Williams on 12/12/11.
//  Copyright (c) 2011 Orion Transfer Ltd. All rights reserved.
//

#include "ObjectAllocator.hpp"

#include <iostream>
#include "../Ensure.hpp"

#include "../Object.hpp"

// sysconf
#include <unistd.h>

// mmap, munmap
#include <sys/mman.h>

// Compatibilty between Darwin/BSD and Linux
#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

#define KAI_MEMORY_STATISTICS

namespace Kai {
	namespace Memory {
		static const bool MEMORY_DEBUG = false;
		static const bool MEMORY_DEBUG_ALLOCATE = MEMORY_DEBUG & false;
		static const bool MEMORY_DEBUG_DEALLOCATE = MEMORY_DEBUG & false;

		static std::size_t _allocation_id = 0;

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
		
		ObjectAllocation::ObjectAllocation() {
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
		
		std::size_t ObjectAllocation::memory_size() const {
			if (_next)
				return (ByteT *)_next - (ByteT *)this;
			else
				return 0;
		}
		
// MARK: -
		
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
				
				if (MEMORY_DEBUG_ALLOCATE)
				{
					KAI_ENSURE(this->_next > this);
					KAI_ENSURE(middle->_next > middle);
				}

				// We created a free block, return it.
				return middle;
			}
			
			// There wasn't enough space to create a free block, return the next free block.
			return next_free;
		}
		
// MARK: -
		
		PageAllocation::PageAllocation() {
			
		}
		
		PageAllocation::~PageAllocation() {
			
		}
		
		void PageAllocation::prepend(FreeAllocation * free_allocation)
		{
			KAI_ENSURE(free_allocation->_next_free == nullptr);

			free_allocation->_next_free = this->_next_free;
			this->_next_free = free_allocation;
		}
		
		void PageAllocation::check() const
		{
			debug();
			
			std::cerr << "-- Free List Check --" << std::endl;
			
			std::set<const ObjectAllocation *> actual_free_list;
			
			{
				const FreeAllocation * current = this->_next_free;
				
				while (current)
				{
					if (current->_flags != 0)
					{
						std::cerr << "!! Invalid free block @ " << current << std::endl;
					}
					else
					{
						std::cerr << "Free block @ " << current << std::endl;
						actual_free_list.insert(current);
					}

					current = current->_next_free;
				}
			}
			
			{
				const ObjectAllocation * current = this;
				
				while (current)
				{
					//std::cerr << "Current: " << current << " Next: " << current->_next << std::endl;
					
					if (current->_flags == FREE)
					{
						if (actual_free_list.find(current) == actual_free_list.end())
						{
							std::cerr << "!! Missing entry for " << current << std::endl;
						}
					}

					current = current->_next;
				}
			}
			
			std::cerr << "Free blocks = " << actual_free_list.size() << std::endl;
		}
		
		PageAllocation * PageAllocation::create(std::size_t size) {
			void * base = mmap(0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
#ifdef KAI_MEMORY_STATISTICS
			g_statistics.total += size;
#endif
			
			if (MEMORY_DEBUG)
				std::cerr << "** Allocating " << size << " bytes, mapped memory at offset " << base << std::endl;
			
			PageAllocation * front = new(base) PageAllocation;
			front->_next_page_allocation = NULL;
			front->_flags = FRONT | USED | PINNED;
			
			void * top = (ByteT *)base + size - sizeof(PageBoundary);
			PageBoundary * back = new(top) PageBoundary;
			back->_next = NULL;
			back->_flags = BACK | USED | PINNED;
			back->_front = front;
			front->_back = back;
			
			FreeAllocation * free = new((ByteT *)base + sizeof(PageAllocation)) FreeAllocation;
			free->_next = back;
			front->_next = free;
			front->_next_free = free;
			
			if (MEMORY_DEBUG_ALLOCATE)
			{
				front->check();
			}

			return front;
		}

		ObjectAllocation * PageAllocation::allocate(std::size_t size) {
			if (MEMORY_DEBUG)
				std::cerr << "** Allocate " << size << std::endl;
			
			size = calculate_alignment(size, ALIGNMENT);
			
			PageAllocation * base = this;
			FreeAllocation * previous_free = base, * next_free = NULL;

			while (base) {
				// Does this page allocation have a free list?
				next_free = base->_next_free;
				
				// Traverse the free-list looking for the right sized block:
				while (next_free && next_free->memory_size() < size) {
					previous_free = next_free;
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
					
					previous_free = base;
					next_free = base->_next_free;
				}
			}
			
			// At this point we are guaranteed that the allocation is the right size:
			ObjectAllocation * allocation = next_free;
			
			// Split the chunk if required:
			next_free = next_free->split(size);
			
			// Update the free list:
			previous_free->_next_free = next_free;
			
			// Mark the chunk as being used:
			allocation->_flags |= USED;
			
#ifdef KAI_MEMORY_STATISTICS
			g_statistics.used += allocation->memory_size();
#endif

			if (MEMORY_DEBUG)
			{
				_allocation_id += 1;
				
				std::cerr << "Allocated (" << _allocation_id << ") " << size << " bytes at " << allocation << std::endl;
				
				if (MEMORY_DEBUG_DEALLOCATE)
				{
					this->check();
				}
			}
			
			return allocation;
		}
		
		void PageAllocation::deallocate(ObjectAllocation * start, ObjectAllocation * end) {
			if (MEMORY_DEBUG)
				std::cerr << "** Deallocate " << start << " -> " << end << std::endl;
			
			// Initialize a new free block in this segment:
			FreeAllocation * free_allocation = new(start) FreeAllocation;
			free_allocation->_flags &= ~USED;
			
			// The next block is the block past the end:
			free_allocation->_next = end->_next;
			
			if (MEMORY_DEBUG_DEALLOCATE)
				std::cerr << "Deallocating range from " << start << " -> " << end->_next << "(" << free_allocation->memory_size() << " bytes)" << std::endl;
			
			// Insert the free block into the free list:
			prepend(free_allocation);
			
#ifdef KAI_MEMORY_STATISTICS
			g_statistics.freed += free_allocation->memory_size();
			g_statistics.used -= free_allocation->memory_size();
#endif

			//this->check();
		}
		
		bool PageAllocation::includes(const ObjectAllocation * allocation) {
			KAI_ENSURE(this->_flags & FRONT);
			
			return allocation >= this && allocation <= this->_back;
		}
		
		PageAllocation * PageAllocation::base_of(const ObjectAllocation * allocation) {
			PageAllocation * base = this;
			
			while (base) {
				if (base->includes(allocation)) return base;
				
				base = base->_next_page_allocation;
			}
			
			return NULL;
		}
		
		std::size_t PageAllocation::allocation_count() const
		{
			std::size_t count = 0;
			
			const ObjectAllocation * current = this;
			
			while (current)
			{
				count += 1;
				
				current = current->_next;
			}
			
			return count;
		}
		
		void PageAllocation::debug() const {
#ifdef KAI_MEMORY_STATISTICS
			std::cerr << "Total: " << g_statistics.total << " Used: " << g_statistics.used << " Freed: " << g_statistics.freed << std::endl;
#endif

			{
				std::cerr << "-- Page @ " << this << " --" << std::endl;
				const ObjectAllocation * current = this;
				while (current) {
					std::cerr << "[" << current << "(" << current->_flags << ")" << " + " << current->memory_size() << "] -> " << current->_next << std::endl;
					
					current = current->_next;
				}
			}

			{
				std::cerr << "-- Free List --" << std::endl;
				const FreeAllocation * current = _next_free;
				
				while (current) {
					std::cerr << "[" << current << "(" << current->_flags << ")" << " + " << current->memory_size() << "] -> " << current->_next << std::endl;
					
					current = current->_next_free;
				}
			}
			
		}

// MARK: -

		PageBoundary::PageBoundary() {
			
		}
		
		PageBoundary::~PageBoundary() {
			
		}
		
	}	
}
