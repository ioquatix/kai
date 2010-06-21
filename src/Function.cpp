/*
 *  Function.cpp
 *  This file is part of the "Kai" project, and is licensed under the GNU GPLv3.
 *
 *  Created by Samuel Williams on 13/04/10.
 *  Copyright 2010 Samuel Williams. All rights reserved.
 *
 */

#include "Function.h"
#include "Frame.h"
#include "Exception.h"
#include "Ensure.h"

#include <iostream>

namespace Kai {
	namespace Builtins {

		// Builtin Logical Operations
		Value * logicalOr (Frame * frame) {
			Cell * cur = frame->operands();
			
			while (cur != NULL) {
				Value * value = cur->head()->evaluate(frame);
				
				if (Value::toBoolean(value)) {
					return value;
				}
				
				cur = cur->tailAs<Cell>();
			}
			
			return Symbol::falseSymbol();
		}
		
		Value * logicalAnd (Frame * frame) {
			Cell * cur = frame->operands();
			
			while (cur != NULL) {
				Value * value = cur->head()->evaluate(frame);
				
				if (!Value::toBoolean(value)) {
					return Symbol::falseSymbol();					
				}
				
				cur = cur->tailAs<Cell>();
			}
			
			return Symbol::trueSymbol();		
		}
		
		Value * logicalNot (Frame * frame) {
			Cell * cur = frame->unwrap();
			
			if (cur == NULL)
				throw Exception("Invalid Argument", frame);
			
			if (Value::toBoolean(cur->head())) {
				return Symbol::falseSymbol();
			}
			
			return Symbol::trueSymbol();
		}
	}
}
