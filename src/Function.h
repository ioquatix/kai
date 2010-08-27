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
#include <llvm/Type.h>

namespace Kai {

	typedef Value * (*EvaluateFunctionT)(Frame *);
	
	llvm::Value * buildTrampoline (std::string name, EvaluateFunctionT function, Frame * frame);
	
	template <Value * (*FunctionT)(Frame *)>
	class BuiltinFunction : public Value {
		protected:
			const char * m_name;

		public:
			BuiltinFunction (const char * name) : m_name(name) {
			
			}
			
			virtual Value* evaluate (Frame * frame) {
				return FunctionT(frame);
			}
			
			virtual llvm::Value * compile (Frame * frame) {
				return buildTrampoline(m_name, FunctionT, frame);
			}
			
			virtual void toCode (StringStreamT & buffer) {
				buffer << "(builtin-function " << m_name << ")";
			}
	};

	#define KFunctionWrapper(function) new BuiltinFunction<&function>(#function)
	
	class DynamicFunction : public Value {
		protected:
			EvaluateFunctionT m_function;
			
		public:
			DynamicFunction (EvaluateFunctionT function);
			virtual ~DynamicFunction ();
			
			// Executes the given function
			virtual Value * evaluate (Frame * frame);
			
			virtual void toCode (StringStreamT & buffer);
	};	
}

#endif
