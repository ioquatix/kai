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

#include "Value.h"

namespace Kai {

	template <Value * (*FunctionT)(Frame *)>
	class BuiltinFunction : public Value {
		private:
			const char * m_name;

		public:
			BuiltinFunction (const char * name) : m_name(name) {
			
			}
			
			virtual Value* evaluate (Frame * frame) {
				return FunctionT(frame);
			}
			
			virtual void toCode (StringStreamT & buffer) {
				buffer << m_name;
			}
	};

	#define KFunctionWrapper(function) new BuiltinFunction<&function>(#function)

	namespace Builtins {
		// Builtin Logical Operations
		Value * logicalOr (Frame * frame);
		Value * logicalAnd (Frame * frame);
		Value * logicalNot (Frame * frame);
		
		// Processing
		Value * with (Frame * frame);
	};
	
}

#endif
