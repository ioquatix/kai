//
//  Collector.cpp
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 29/12/11.
//  Copyright (c) 2011 Orion Transfer Ltd. All rights reserved.
//

#include "Collector.h"
#include "../Ensure.h"

namespace Kai {
	namespace Memory {
		
		Collector::Collector(PageAllocation * start) : _start(start) {
			
		}
		
		Collector::~Collector() {
			
		}
		
		void Collector::traverse(const ObjectAllocation * object) {
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
				
				//std::cerr << ">> Marking " << object << std::endl;
				
				// Mark the object:
				object->_flags |= MARKED;
				
				// Traverse any children/edges:
				object->mark(this);
				
				//std::cerr << "<< Marking " << object << std::endl;
			}
		}
		
		std::size_t Collector::collect() {
			std::size_t deallocation_count = 0;
			
			ObjectAllocation * allocation = _start;
			
			// Mark all root objects and their descendants:
			while (allocation) {
				// Root objects are pinned:
				if (allocation->_flags & PINNED) {
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
					// It is impossible to take this approach with the current memory allocator as it can't recombine free blocks.
					
					// If we've already started to free a block of memory, include any free blocks too. This ensures we deallocate the maximum possible range of memory which needs to be freed and possibly interleaved free blocks.
					//if (start) {
					//	finish = allocation;
					//}
					
					// Block was free memory
					allocation = allocation->_next;
					
					// Jump back to the top...
					// continue;
				}
				
				// We have encountered a discontinuity (i.e. non-free and non-unused) in the memory heap, so now we should free all unneeded allocations if there were any previously recorded:
				if (start) {
					_start->deallocate(start, finish);
					
					start = NULL;
					
					deallocation_count += 1;
				}
			}
			
			return deallocation_count;
		}
		
	}
}
