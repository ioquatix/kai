//
//  ManagedObject.h
//  Kai
//
//  Created by Samuel Williams on 29/12/11.
//  Copyright (c) 2011 Orion Transfer Ltd. All rights reserved.
//

#ifndef _KAI_MEMORY_MANAGEDOBJECT_H
#define _KAI_MEMORY_MANAGEDOBJECT_H

#include "ObjectAllocator.h"

namespace Kai {
	namespace Memory {
		
		typedef Memory::PageAllocation ObjectAllocator;
		
		class ManagedObject : public Memory::ObjectAllocation {
		public:
			ManagedObject();
			ManagedObject(ManagedObject & other);
			virtual ~ManagedObject();

			ManagedObject & operator=(ManagedObject & other);
			
			// Global Memory Pool.
			void * operator new(std::size_t);
			
			// Per-context Memory Pool
			void * operator new(std::size_t, ObjectAllocation *);
			void * operator new(std::size_t, ObjectAllocator *);
			
			void operator delete(void *);
		};
		
	}
}

#endif