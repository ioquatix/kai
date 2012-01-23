//
//  Logic.h
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 16/12/11.
//  Copyright (c) 2011 Orion Transfer Ltd. All rights reserved.
//

#ifndef _KAI_LOGIC_H
#define _KAI_LOGIC_H

#include "Object.h"

namespace Kai {
	
	// Because some of these names are C++ keywords, they have an underscore appended.
	class Logic {			
	public:
		static Ref<Object> or_(Frame * frame);
		static Ref<Object> and_(Frame * frame);
		static Ref<Object> not_(Frame * frame);
		
		static Ref<Object> when(Frame * frame);
		static Ref<Object> if_(Frame * frame);
		
		static Ref<Object> block(Frame * frame);
		static Ref<Object> return_(Frame * frame);
		
		//static Ref<Object> anythingValue();
		//static Ref<Object> nothingValue();
		
		static void import(Frame * frame);
	};
	
}

#endif
