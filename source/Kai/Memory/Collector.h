//
//  Collector.h
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 29/12/11.
//  Copyright (c) 2011 Orion Transfer Ltd. All rights reserved.
//

#ifndef _KAI_MEMORY_COLLECTOR_H
#define _KAI_MEMORY_COLLECTOR_H

#include "ObjectAllocator.h"

namespace Kai {
	namespace Memory {
	
		class Collector : public Traversal {
		protected:
			PageAllocation * _start;
			
		public:
			Collector(PageAllocation * start);
			virtual ~Collector();
			
			virtual void traverse(const ObjectAllocation * object);
			
			std::size_t collect();
		};
		
	}
}

#endif
