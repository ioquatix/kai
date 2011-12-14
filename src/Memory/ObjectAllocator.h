//
//  ObjectAllocator.h
//  Page Allocator
//
//  Created by Samuel Williams on 12/12/11.
//  Copyright (c) 2011 Orion Transfer Ltd. All rights reserved.
//

#ifndef _Kai_Memory_ObjectAllocator_h
#define _Kai_Memory_ObjectAllocator_h

#include <iostream>

namespace Kai {
	class ManagedObject;
	
	namespace Memory {
	
		// Align to the size of pointers, naturally.
		static const std::size_t ALIGNMENT = sizeof(void*);
		typedef unsigned char ByteT;
		
		// Return a size such that it is a multiple of alignment, and at least as big as the original size.
		inline std::size_t calculate_alignment(std::size_t size, std::size_t alignment) {
			// alignment needs to be a power of 2 for this to work correctly.
			return (size + (alignment - 1)) & ~(alignment - 1);
		}
		
		class PageAllocation;
		
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
		
		class ObjectAllocation {
		private:			
			friend class PageAllocation;
			friend class ManagedObject;
			
			ObjectAllocation * split(std::size_t size);

			ObjectAllocation * _previous, * _next;
			unsigned _flags, _ref_count;
			
			std::size_t memory_size();
			
			// This function will essentially delete the object from the object graph.
			ObjectAllocation * remove();
		protected:
			bool marked();
			virtual void mark();
			
		public:
			ObjectAllocation();
			virtual ~ObjectAllocation();
			
			PageAllocation * allocator();
			
			void retain();
			void release();
		};
		
		class PageAllocation : public ObjectAllocation {
		protected:
			//ObjectAllocation * _next_free;
			
		public:
			static PageAllocation * create(std::size_t size);
			
			PageAllocation();
			virtual ~PageAllocation();

			ObjectAllocation * allocate(std::size_t size);
			
			void debug();
			
			void collect();
		};
	}
	
	typedef Memory::PageAllocation ObjectAllocator;
	
	class ManagedObject : public Memory::ObjectAllocation {
	public:
		virtual ~ManagedObject();
		
		// Global Memory Pool
		void * operator new(std::size_t);
		
		// Per-context Memory Pool
		void * operator new(std::size_t, ObjectAllocation *);
		void * operator new(std::size_t, ObjectAllocator *);
		
		void operator delete(void *);
	};
}

#endif
