//
//  Function.h
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 13/04/10.
//  Copyright 2010 Samuel Williams. All rights reserved.
//
//

#ifndef _KFUNCTION_H
#define _KFUNCTION_H

#include "Object.hpp"

#define KAI_BUILTIN_FUNCTION(function) (builtin_function<&function>(#function))

namespace Kai {
	
	typedef Object * (*EvaluateFunctionT)(Frame *);
	
	template <Ref<Object> (*FunctionT)(Frame *)>
	class BuiltinFunction : public Object {
	protected:
		const char * _name;
		
	public:
		static const char * const NAME;
		
		BuiltinFunction(const char * name) : _name(name) {
		}
		
		virtual Ref<Object> evaluate (Frame * frame) {
			return FunctionT(frame);
		}
		
		virtual void to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const {
			buffer << "(builtin-function " << _name << ")";
		}
	};
	
	template <Ref<Object> (*FunctionT)(Frame *)>
	const char * const BuiltinFunction<FunctionT>::NAME = "BuiltinFunction";
	
	template <Ref<Object> (*FunctionT)(Frame *)>
	BuiltinFunction<FunctionT> * builtin_function(const char * name) {
		static BuiltinFunction<FunctionT> instance(name);
		
		return &instance;
	}
	
	class DynamicFunction : public Object {
	protected:
		EvaluateFunctionT _evaluate_function;
		
	public:
		static const char * const NAME;
		
		DynamicFunction(EvaluateFunctionT evaluate_function);
		virtual ~DynamicFunction();
		
		virtual Ref<Object> evaluate(Frame * frame);
		
		virtual void to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const;
	};	
}

#endif
