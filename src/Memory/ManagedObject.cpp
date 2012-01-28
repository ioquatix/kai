//
//  ManagedObject.cpp
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 29/12/11.
//  Copyright (c) 2011 Orion Transfer Ltd. All rights reserved.
//

#include "ManagedObject.h"
#include "Ensure.h"

namespace Kai {
	namespace Memory {
	
		ManagedObject::ManagedObject() {
		}
		
		ManagedObject::ManagedObject(ManagedObject & other) {
			// Don't copy memory management meta-data.			
		}
		
		ManagedObject & ManagedObject::operator=(ManagedObject & other) {
			// Don't copy memory management meta-data.
			
			return *this;
		}
		
		ManagedObject::~ManagedObject() {
		}
		
		void * ManagedObject::operator new(std::size_t size) {
			throw std::bad_alloc();
			//return ManagedObject::operator new(size, Memory::object_allocator());
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
}
