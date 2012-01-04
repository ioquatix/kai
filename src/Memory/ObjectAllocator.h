//
//  ObjectAllocator.h
//  Page Allocator
//
//  Created by Samuel Williams on 12/12/11.
//  Copyright (c) 2011 Orion Transfer Ltd. All rights reserved.
//

#ifndef _KAI_MEMORY_OBJECTALLOCATOR_H
#define _KAI_MEMORY_OBJECTALLOCATOR_H

#include <iostream>

namespace Kai {
	namespace Memory {
		class Collector;
		class ManagedObject;
		
		std::size_t page_size();
	
		// Align to the size of pointers, naturally.
		static const std::size_t ALIGNMENT = sizeof(void*);
		typedef unsigned char ByteT;
		
		// Return a size such that it is a multiple of alignment, and at least as big as the original size.
		inline std::size_t calculate_alignment(std::size_t size, std::size_t alignment) {
			// alignment needs to be a power of 2 for this to work correctly.
			return (size + (alignment - 1)) & ~(alignment - 1);
		}
		
		class PageAllocation;
		class FreeAllocation;
		
		enum Flags {
			// The memory has been freed.
			FREE = 0,
			
			// The memory is in use.
			USED = 1,
			
			// The object has been marked during the tracing phase of garbage collection.
			MARKED = 4,
			
			// The object has been deleted using operator delete
			DELETED = 8,
			PINNED = 16,
			
			// Mark the start and end of a page.
			FRONT = 32,
			BACK = 64
		};
		
		class Traversal;
		
		class ObjectAllocation {
		protected:
			friend class ManagedObject;
			friend class PageAllocation;
			friend class FreeAllocation;
			friend class Collector;
			
			ObjectAllocation * _next;
			mutable unsigned _flags, _ref_count;
			
			/// Return the distance in bytes from the strt of this allocation to the start of the next.
			std::size_t memory_size();
			
		protected:
			virtual void mark(Traversal *) const;
			
		public:
			ObjectAllocation();
			virtual ~ObjectAllocation();
			
			virtual PageAllocation * allocator() const;
			
			void retain();
			void release();
		};
		
		class FreeAllocation : public ObjectAllocation {			
		protected:
			friend class PageAllocation;
			
			FreeAllocation * _next_free;
			
		public:
			FreeAllocation(FreeAllocation * next_free = NULL);
			virtual ~FreeAllocation();
			
			FreeAllocation * split(std::size_t size);
			
			FreeAllocation * next() const {
				return _next_free;
			}
		};
		
		class PageAllocation : public FreeAllocation {
		protected:
			ObjectAllocation * _back;
			PageAllocation * _next_page_allocation;
			
		public:
			static PageAllocation * create(std::size_t size);
			
			PageAllocation();
			virtual ~PageAllocation();
			
			ObjectAllocation * allocate(std::size_t size);
			void deallocate(ObjectAllocation * start, ObjectAllocation * end);
			
			bool includes(ObjectAllocation * allocation);
			PageAllocation * base_of(ObjectAllocation * allocation);
			
			void debug();
		};
		
		class PageBoundary : public FreeAllocation {
		protected:
			friend class ObjectAllocation;
			friend class PageAllocation;
			
			PageAllocation * _front;
		
		public:
			PageBoundary();
			virtual ~PageBoundary();
		};
		
		// Used for implementing various mark and sweep algorithms.
		class Traversal {
		public:
			virtual ~Traversal();
			
			virtual void traverse(ObjectAllocation *) = 0;
		};
	}
}

#endif
