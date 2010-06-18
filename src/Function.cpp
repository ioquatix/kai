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
		
		Value * with (Frame * frame) {
			Cell * cur = frame->operands();
			Value * caller = frame->caller();
			
			while (cur != NULL) {
				Symbol * function = NULL;
				Cell * operands = NULL;
				
				if (function = cur->headAs<Symbol>()) {
					
				} else {
					function = cur->headAs<Cell>()->headAs<Symbol>();
					operands = cur->headAs<Cell>()->tailAs<Cell>();
				}
				
				frame = new Frame(caller, function, operands, frame);
				caller = caller->invoke(frame);
				
				cur = cur->tailAs<Cell>();
			}
			
			return caller;
		}
	}
}
