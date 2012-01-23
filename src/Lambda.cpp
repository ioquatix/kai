//
//  Lambda.cpp
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 27/09/11.
//  Copyright 2011 Orion Transfer Ltd. All rights reserved.
//

#include "Lambda.h"
#include "Frame.h"
#include "Table.h"
#include "Symbol.h"
#include "Function.h"
#include "Array.h"
#include "Logic.h"

namespace Kai {

#pragma mark -

	const char * const Lambda::NAME = "Lambda";

	Lambda::Lambda(Frame * scope, Cell * arguments, Cell * code) : _scope(scope), _arguments(arguments), _code(code), _macro(false) {
	}
	
	Lambda::~Lambda () {
	}
	
	Ref<Symbol> Lambda::identity(Frame * frame) const {
		return frame->sym("Lambda");
	}
	
	void Lambda::mark(Memory::Traversal * traversal) const {
		traversal->traverse(_scope);
		traversal->traverse(_arguments);
		traversal->traverse(_code);
	}
	
	struct LambdaScope {
		Array * _chain;
		
		LambdaScope(Frame * frame, Frame * scope) {
			_chain = frame->lookup(frame->sym("dynamic-scope-chain")).as<Array>();
			
			_chain->value().push_back(scope);
		}
		
		~LambdaScope() {
			_chain->value().pop_back();
		}
	};
	
	Ref<Object> Lambda::evaluate(Frame * frame) {
		Table * locals = new(frame) Table;
		
		Cell * names = _arguments;
		Cell * values = NULL;
		
		if (is_macro()) {
			values = frame->operands();
		} else {
			values = frame->unwrap();
		}
		
		while (names != NULL) {
			if (values == NULL) {
				throw Exception("Lambda Arity Mismatch", frame);
			}
			
			locals->update(names->head().as<Symbol>(), values->head());
			
			names = names->tail().as<Cell>();
			values = values->tail().as<Cell>();
		}
				
		// Give the execution scope access to the executing lambda:
		locals->update(frame->sym("frame"), frame);
		locals->update(frame->sym("caller"), frame->scope());
		locals->update(frame->sym("callee"), this);
		
		{			
			Frame * next = new(frame) Frame(locals, _scope);
			LambdaScope lambda_scope(frame, next);
			
			if (_code)
				return _code->evaluate(next);
			else
				return NULL;
		}
	}
	
	void Lambda::to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const {
		buffer << "(";
		
		if (is_macro()) {
			buffer << "macro";
		} else {
			buffer << "lambda";
		}
		
		buffer << "@" << this;
		
		if (marks.find(this) != marks.end()) {
			buffer << ")";
		} else {
			buffer << " `";
			
			MarkedT recurse (marks);
			recurse.insert(this);
			
			if (_arguments)
				_arguments->to_code(frame, buffer, recurse, indentation + 1);
			else
				buffer << "()";
				
			buffer << " `";
			
			if (_code)
				_code->to_code(frame, buffer, recurse, indentation + 1);
			else
				buffer << "()";
			
			buffer << ")";
		}
	}
	
	Ref<Object> Lambda::to_macro(Frame * frame)
	{
		Lambda * self;
		
		frame->extract()(self, "self");
		
		Lambda * macro = new(frame) Lambda(*self);
		macro->set_macro(true);
		
		return macro;
	}
	
	Ref<Object> Lambda::is_macro(Frame * frame)
	{
		Lambda * self;
		
		frame->extract()(self, "self");
		
		if (self->is_macro()) {
			return Symbol::true_symbol(frame);
		} else {
			return Symbol::false_symbol(frame);
		}
	}
	
	Ref<Object> Lambda::is_function(Frame * frame)
	{
		Lambda * self;
		
		frame->extract()(self, "self");
		
		if (!self->is_macro()) {
			return Symbol::true_symbol(frame);
		} else {
			return Symbol::false_symbol(frame);
		}		
	}
	
	Ref<Object> Lambda::lambda(Frame * frame) {
		Cell * arguments, * code;
		
		frame->extract()[arguments][code];
		
		return new(frame) Lambda(frame, arguments, code);
	}
	
	Ref<Object> Lambda::macro(Frame * frame) {
		Cell * arguments, * code;
		
		frame->extract()[arguments][code];
		
		Lambda * lambda = new(frame) Lambda(frame, arguments, code);
		lambda->set_macro(true);
		
		return lambda;
	}
	
	Ref<Object> Lambda::dynamic_scope(Frame * frame) {
		Symbol * name;
		
		frame->extract()(name, "name");
		
		Array * chain = frame->lookup(frame->sym("dynamic-scope-chain")).as<Array>();
		
		for (Array::ArrayT::reverse_iterator i = chain->value().rbegin(); i != chain->value().rend(); ++i) {
			Object * result = (*i)->lookup(frame, name);
			
			if (result)
				return result;
		}
		
		return NULL;
	}
	
	Ref<Object> Lambda::prototype(Frame * frame) {
		return frame->lookup(frame->sym("Lambda"));
	}
	
	void Lambda::import(Frame * frame) {
		Table * prototype = new(frame) Table;
		
		prototype->update(frame->sym("is-macro?"), KAI_BUILTIN_FUNCTION(Lambda::is_macro));
		prototype->update(frame->sym("is-function?"), KAI_BUILTIN_FUNCTION(Lambda::is_function));
		prototype->update(frame->sym("to-macro"), KAI_BUILTIN_FUNCTION(Lambda::to_macro));
		prototype->update(frame->sym("new"), KAI_BUILTIN_FUNCTION(Lambda::lambda));
				
		frame->update(frame->sym("Lambda"), prototype);
		frame->update(frame->sym("lambda"), KAI_BUILTIN_FUNCTION(Lambda::lambda));
		frame->update(frame->sym("macro"), KAI_BUILTIN_FUNCTION(Lambda::macro));
		
		// Lexicographic stack based on nested lambda evaluations.
		frame->update(frame->sym("dynamic-scope-chain"), new(frame) Array);
		frame->update(frame->sym("dynamic-scope"), KAI_BUILTIN_FUNCTION(Lambda::dynamic_scope));
	}

}
