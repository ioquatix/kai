//
//  Collector.cpp
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 29/12/11.
//  Copyright (c) 2011 Orion Transfer Ltd. All rights reserved.
//

#include "Collector.h"
#include "Ensure.h"

namespace Kai {
	namespace Memory {
		
		Collector::Collector(PageAllocation * start) : _start(start) {
			
		}
		
		Collector::~Collector() {
			
		}
		
		void Collector::traverse(ObjectAllocation * object) {
			if (object) {
				if (!_start->includes(object)) {
					//std::cerr << "Couldn't traverse foreign memory: " << object << " (Potential memory leak)." << std::endl;
					
					return;
				}
				
				// We can't have objects in different parts of the memory graph pointing at each other.
				//KAI_ENSURE(_start->includes(object));
				
				// If the object has already been marked, don't visit it.
				if (object->_flags & MARKED) {
					return;
				}
				
				// Mark the object:
				object->_flags |= MARKED;
				
				// Traverse any children/edges:
				object->mark(this);
			}
		}
		
		void Collector::collect() {
			ObjectAllocation * allocation = _start;
			
			// Mark all root objects and their descendants:
			while (allocation) {
				// A ref_count of 1 or more marks a root object.
				if (allocation->_ref_count) {
					traverse(allocation);
				}
				
				allocation = allocation->_next;
			}
			
			// Remove any unmarked objects since they are no longer accessible:
			allocation = _start;
			
			// We try to free multiple blocks at once, so we keep track of how many allocations can be removed at once:
			ObjectAllocation * start = NULL, * finish = NULL;
			
			while (allocation) {
				// We need to move through the linked list of allocations...
				if (allocation->_flags & MARKED) {
					allocation->_flags &= ~MARKED;
					allocation = allocation->_next;
				} else if (allocation->_flags & USED) {
					// Deallocate the object:
					allocation->ObjectAllocation::~ObjectAllocation();
					
					// Free the memory:
					//allocation = allocation->remove();
					
					// Track the start and end of this sequence of allocations:
					if (start) {
						finish = allocation;
					} else {
						start = allocation;
						finish = allocation;
					}
					
					allocation = allocation->_next;
					
					// Jump back to the top...
					continue;
				} else {
					// Block was free memory
					allocation = allocation->_next;
				}
				
				// We have encountered a discontinuity in the memory heap, so now we should free all unneeded allocations:
				if (start) {
					_start->deallocate(start, finish);
					
					start = NULL;
				}
			}
		}
		
	}
}
