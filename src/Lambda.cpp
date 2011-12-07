//
//  Lambda.cpp
//  Kai
//
//  Created by Samuel Williams on 27/09/11.
//  Copyright 2011 Orion Transfer Ltd. All rights reserved.
//

#include "Lambda.h"
#include "Frame.h"
#include "Function.h"

namespace Kai {

#pragma mark -
#pragma mark Lambda

	Lambda::Lambda (Frame * scope, Cell * arguments, Cell * code)
		: m_scope(scope), m_arguments(arguments), m_code(code) {
		// Required if we inherit from gc_cleanup, as this will often create a circular finalisation reference
		//GC_register_disappearing_link((void**)&m_scope);
	}
	
	Lambda::~Lambda () {
		
	}
	
	Ref<Value> Lambda::evaluate (Frame * frame) {
		Table * locals = new Table;
		
		Cell * names = m_arguments;
		Cell * values = frame->unwrap();
		while (names != NULL) {
			if (values == NULL) {
				throw Exception("Lambda Arity Mismatch", frame);
			}
			
			locals->update(names->headAs<Symbol>(), values->head());
			
			names = names->tailAs<Cell>();
			values = values->tailAs<Cell>();
		}
		
		// Give the execution scope access to the executing lambda:
		locals->update(sym("caller"), this);
		
		Frame * next = new Frame(locals, m_scope);
		
		if (m_code)
			return m_code->evaluate(next);
		else
			return NULL;
	}
	
	void Lambda::toCode(StringStreamT & buffer, MarkedT & marks, std::size_t indentation) {
		buffer << "(lambda{" << this << "}";
		if (marks.find(this) != marks.end()) {
			buffer << ")";
		} else {
			buffer << " `";
			
			MarkedT recurse (marks);
			recurse.insert(this);
			
			if (m_arguments)
				m_arguments->toCode(buffer, recurse, indentation + 1);
			else
				buffer << "()";
				
			buffer << " `";
			
			if (m_code)
				m_code->toCode(buffer, recurse, indentation + 1);
			else
				buffer << "()";
			
			buffer << ")";
		}
	}
	
	Ref<Value> Lambda::lambda (Frame * frame) {
		Cell * arguments, * code;
		
		frame->extract()[arguments][code];
		
		return new Lambda(frame, arguments, code);
	}
	
	void Lambda::import (Table * context) {
		context->update(sym("lambda"), KFunctionWrapper(Lambda::lambda));
	}

}
