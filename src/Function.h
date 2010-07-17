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
				buffer << "(builtin-function " << m_name << ")";
			}
	};

	#define KFunctionWrapper(function) new BuiltinFunction<&function>(#function)
	
	class CompiledFunction : public Value {
		private:
			EvaluateFunctionT m_function;
			
		public:
			CompiledFunction (EvaluateFunctionT function);
			virtual ~CompiledFunction ();
			
			// Returns a compiled function corresponding to the given arguments.
			virtual Value * evaluate (Frame * frame);
			
			virtual void toCode (StringStreamT & buffer);
	};
	
	class CompiledType : public Value {
		private:
			const llvm::Type * m_type;
		
		public:
			CompiledType (const llvm::Type *);
			virtual ~CompiledType ();
			
			const llvm::Type * value () const;
			
			virtual void toCode (StringStreamT & buffer);
			
			static Value * voidType (Frame * frame);
			static Value * intType (Frame * frame);
			static Value * floatType (Frame * frame);
			static Value * pointerType (Frame * frame);
			static Value * arrayType (Frame * frame);
			
			static void import (Table * context);
	};
}

#endif
