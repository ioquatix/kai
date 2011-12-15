//
//  Logic.h
//  Kai
//
//  Created by Samuel Williams on 16/12/11.
//  Copyright (c) 2011 Orion Transfer Ltd. All rights reserved.
//

#ifndef _KAI_LOGIC_H
#define _KAI_LOGIC_H

#include "Value.h"

namespace Kai {
	
	// Because some of these names are C++ keywords, they have an underscore appended.
	class Logic {			
	public:
		static Ref<Value> or_ (Frame * frame);
		static Ref<Value> and_ (Frame * frame);
		static Ref<Value> not_ (Frame * frame);
		
		static Ref<Value> when (Frame * frame);
		static Ref<Value> if_ (Frame * frame);
		
		static Ref<Value> block (Frame * frame);
		static Ref<Value> return_ (Frame * frame);
		
		static Ref<Value> trueValue ();
		static Ref<Value> falseValue ();
		static Ref<Value> anythingValue ();
		static Ref<Value> nothingValue ();
		
		static void import (Table *);
	};
	
}

#endif
