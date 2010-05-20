/*
 *  Function.h
 *  This file is part of the "Kai" project, and is licensed under the GNU GPLv3.
 *
 *  Created by Samuel Williams on 13/04/10.
 *  Copyright 2010 Samuel Williams. All rights reserved.
 *
 */

#ifndef _KFUNCTION_H
#define _KFUNCTION_H

#include "Cell.h"

namespace Kai {

	template <typename ResultT, ResultT (*FunctionT)(Frame *)>
	class Function : public Value {
		private:
			const char * m_name;

		public:
			Function (const char * name) : m_name(name) {
			
			}
			
			virtual Value* evaluate (Frame * frame) {
				return FunctionT(frame);
			}
			
			virtual void toCode (StringStreamT & buffer) {
				buffer << m_name;
			}
	};

	#define KFunctionWrapper(function) new Function<typeof(function((Frame*)0)), &function>(#function)

	namespace Builtins {

		Value * trace (Frame * frame);
		
		// Returns the arguments unevaluated
		Value * value (Frame * frame);
		
		// Returns a function such that when evaluated, returns the arguments unevaluated.
		Value * wrap (Frame * frame);
		
		// Returns the arguments evaluated in the caller's context.
		Cell * unwrap (Frame * frame);
		
		// Updates the value in the Table caller.
		Value * update (Frame * frame);
		
		// Compares two given values
		Symbol * compare (Frame * frame);
		
		// Returns the caller of the current frame
		Value * caller (Frame * frame);
		
		// Extract values from a given Cell
		Value * head (Frame * frame);
		Value * tail (Frame * frame);

		// Builtin Logical Operations
		Value * logicalOr (Frame * frame);
		Value * logicalAnd (Frame * frame);
		Value * logicalNot (Frame * frame);
		
		// Processing
		Value * with (Frame * frame);
	};
	
}

#endif
