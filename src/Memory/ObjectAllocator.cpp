//
//  ObjectAllocator.cpp
//  Page Allocator
//
//  Created by Samuel Williams on 12/12/11.
//  Copyright (c) 2011 Orion Transfer Ltd. All rights reserved.
//

#include "ObjectAllocator.h"

#include "Ensure.h"

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
//#define KAI_MEMORY_USE_MALLOC

namespace Kai {
	namespace Memory {
		static const bool ENABLE_GC = true;
		
		struct AllocatonStatistics {
			std::size_t total;
			std::size_t used;
			std::size_t freed;
		} g_statistics = {0, 0, 0};
		
		ObjectAllocation::ObjectAllocation() : _ref_count(0) {
		}
		
		ObjectAllocation::~ObjectAllocation() {
		}
		
		ObjectAllocator * ObjectAllocation::allocator() {
#ifdef KAI_MEMORY_USE_MALLOC
			return NULL;
#endif
			
			ObjectAllocation * current = this;
			
			while (!(current->_flags & Memory::FRONT)) {
				KAI_ENSURE(current->_previous);
				
				current = current->_previous;
			}
			
			return (ObjectAllocator *)current;
		}
		
		bool ObjectAllocation::marked() {
			return _flags & MARKED;
		}
		
		void ObjectAllocation::mark() {
			_flags |= MARKED;
		}
		
		ObjectAllocation * ObjectAllocation::split(std::size_t size) {
			std::size_t remainder = this->memory_size() - size;
			std::size_t minimum = sizeof(PageAllocation) + 32;
			
			if (remainder >= minimum) {
				PageAllocation * middle = new((ByteT *)this + size) PageAllocation;
				
				middle->_previous = this;
				middle->_next = this->_next;
				middle->_flags = FREE;
				
				this->_next->_previous = middle;
				this->_next = middle;
				
				return middle;
			}
			
			return NULL;
		}
		
		std::size_t ObjectAllocation::memory_size() {
			if (_next)
				return (ByteT *)_next - (ByteT *)this;
			else
				return 0;
		}
		
		ObjectAllocation * ObjectAllocation::remove() {
			g_statistics.freed += this->memory_size();
			g_statistics.used -= this->memory_size();

			if (this == (ObjectAllocation *)0x100138c20)
				std::cerr << "Boo" << std::endl;
			
			this->_flags &= ~USED;
			
#ifdef KAI_MEMORY_DEBUG_FREE
			std::cerr << "Freeing " << this->memory_size() << " bytes at offset " << this << std::endl;
#endif
			
			// We need to find the next used block, which in this case may be one further step if the next block is free.
			ObjectAllocation * next_used = this->_next;
			if (next_used->_flags == FREE) {
				next_used = next_used->_next;
			}
			
			ObjectAllocation * first_free = this;
			if (first_free->_previous && first_free->_previous->_flags == FREE) {
				first_free = _previous;
			}
					
			first_free->_next = next_used;
			first_free->_next->_previous = first_free;
			
			return next_used;
		}
		
		void ObjectAllocation::retain() {
			_ref_count += 1;
		}
		
		void ObjectAllocation::release() {
			_ref_count -= 1;
		}
		
		PageAllocation::PageAllocation() {
			
		}
		
		PageAllocation::~PageAllocation() {
			
		}
			
		PageAllocation * PageAllocation::create(std::size_t size) {
			void * base = mmap(0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
			g_statistics.total += size;
			
#ifdef KAI_MEMORY_DEBUG
			std::cerr << "Allocating " << size << " bytes, mapped memory at offset " << base << std::endl;
#endif
			
			PageAllocation * front = new(base) PageAllocation();
			front->_previous = NULL;
			front->_flags = FRONT;
			
			void * top = (ByteT *)base + size - sizeof(PageAllocation);
			PageAllocation * back = new(top) PageAllocation();
			back->_previous = front;
			back->_next = NULL;
			back->_flags = BACK | USED;
			back->_ref_count = 1;
			
			front->_next = back;
			
			return front;
		}
			
		ObjectAllocation * PageAllocation::allocate(std::size_t size) {
			size = calculate_alignment(size, ALIGNMENT);
			ObjectAllocation * allocation = this;
			
			while (allocation && ((allocation->_flags & USED) || allocation->memory_size() < size)) {
				allocation = allocation->_next;
			}
			
			if (allocation) {
				allocation->split(size);
				g_statistics.used += allocation->memory_size();
				//std::cerr << "used += " << allocation << "->" << allocation->memory_size() << std::endl;
				
				allocation->_flags |= USED;

#ifdef KAI_MEMORY_DEBUG_ALLOC
				std::cerr << "Allocated " << size << " bytes at " << allocation << std::endl;
#endif
				
				//bzero((ByteT *)allocation + sizeof(ObjectAllocation), allocation->memory_size() - sizeof(ObjectAllocation));
				
				return allocation;
			}
			
			return NULL;
		}
		
		void PageAllocation::debug() {
			std::cerr << "Total: " << g_statistics.total << " Used: " << g_statistics.used << " Freed: " << g_statistics.freed << std::endl;
			
			ObjectAllocation * current = this;
			while (current) {
				std::cerr << "[" << current << "(" << current->_flags << ")" << " + " << current->memory_size() << "] -> " << current->_next << std::endl;
				current = current->_next;
			}
		}
		
		void PageAllocation::collect() {
			ObjectAllocation * allocation = this;
			
			// Mark all root objects and their descendants:
			while (allocation) {
				// A ref_count of 1 or more marks a root object.
				if (allocation->_ref_count && !allocation->marked()) {
					allocation->mark();
				}
				
				allocation = allocation->_next;
			}
			
			// Remove any unmarked objects since they are no longer accessible:
			allocation = this;
			while (allocation) {
				//std::cerr << "Sweeping " << allocation << std::endl;
				
				// We need to move through the linked list of allocations...
				if (allocation->_flags & MARKED) {
					allocation->_flags &= ~MARKED;
					allocation = allocation->_next;
				} else if (allocation->_flags & USED) {
					// ... however if we remove an allocation, it might be joined with the free segment before or after, so we can't guarantee the state of memory, thus the remove() function returns a pointer to the next used block of memory, or NULL, if we are at the end.
					
					// Deallocate the object:
					allocation->ObjectAllocation::~ObjectAllocation();
					
					// Free the memory:
					allocation = allocation->remove();
				} else {
					// Block was free memory
					allocation = allocation->_next;
				}
			}
		}
		
		PageAllocation * object_allocator() {
			static PageAllocation * _object_allocator = NULL;
			
			if (_object_allocator == NULL) {
				std::size_t page_size = sysconf(_SC_PAGESIZE);
				_object_allocator = PageAllocation::create(32 * page_size);
			}
			
			return _object_allocator;
		}
	}
	
	ManagedObject::~ManagedObject() {
	}
	
	void * ManagedObject::operator new(std::size_t size) {
		return ManagedObject::operator new(size, Memory::object_allocator());
	}
	
	void * ManagedObject::operator new(std::size_t size, ObjectAllocator * allocator) {
		KAI_ENSURE(allocator);
		
		void * result = allocator->allocate(size);
#ifdef KAI_MEMORY_USE_MALLOC
		void * result = malloc(size);
#endif
		
		if (result == NULL)
			throw std::bad_alloc();
		
		return result;
	}
	
	void * ManagedObject::operator new(std::size_t size, ObjectAllocation * object) {
		KAI_ENSURE(object);
		
		ObjectAllocator * allocator = object->allocator();
		
		return ManagedObject::operator new(size, allocator);
	}
	
	void ManagedObject::operator delete(void * pointer) {
		ObjectAllocation * object = (ObjectAllocation *)pointer;
		
		object->_flags |= Memory::DELETED;
	}
}
