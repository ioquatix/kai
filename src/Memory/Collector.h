//
//  Collector.h
//  Kai
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
			
			virtual void traverse(ObjectAllocation * object);
			
			void collect();
		};
		
	}
}

#endif