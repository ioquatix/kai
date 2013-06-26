//
//  ManagedObject.h
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 29/12/11.
//  Copyright (c) 2011 Orion Transfer Ltd. All rights reserved.
//

#ifndef _KAI_MEMORY_MANAGEDOBJECT_H
#define _KAI_MEMORY_MANAGEDOBJECT_H

#include "ObjectAllocator.h"

namespace Kai {
	namespace Memory {
		
		typedef PageAllocation ObjectAllocator;
		
		class ManagedObject : public ObjectAllocation {
		protected:
			mutable unsigned _reference_count;
			
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
			
			void retain() const;
			void release() const;
			
			unsigned reference_count() const { return _reference_count; }
		};
		
	}
}

#endif
