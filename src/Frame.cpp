/*
 *  Frame.cpp
 *  This file is part of the "Kai" project, and is licensed under the GNU GPLv3.
 *
 *  Created by Samuel Williams on 12/04/10.
 *  Copyright 2010 Samuel Williams. All rights reserved.
 *
 */

#include "Frame.h"
#include "Function.h"
#include <iostream>

namespace Kai {

	Frame::Frame (Value * scope)
		: m_previous(this), m_scope(scope), m_message(NULL), m_arguments(NULL), m_function(NULL)
	{

	}
	
	Frame::Frame (Value * scope, Frame * previous)
		: m_previous(previous), m_scope(scope), m_message(previous->m_message), m_arguments(previous->m_arguments), 
		m_function(m_previous->m_function)
	{
	
	}

	Frame::Frame (Value * scope, Cell * message, Frame * previous)
		: m_previous(previous), m_scope(scope), m_message(message), m_arguments(NULL), m_function(NULL)
	{
		
	}
	
	Value * Frame::lookup (Symbol * identifier) {
		return m_scope->lookup(identifier);
	}

	Value * Frame::apply () {
		std::cerr << "-- " << Value::toString(m_message) << " <= " << Value::toString(m_scope) << std::endl;
		
		m_function = m_message->head()->evaluate(this);
		
		if (!m_function) {
			throw Exception("Invalid Function", m_message->head(), this);
		}
		
		return m_function->evaluate(this);
	}

	Value * Frame::call (Value * scope, Cell * message)
	{
		if (scope == NULL) {
			throw Exception("Invalid Scope", this);
		}
		
		if (message == NULL) {
			throw Exception("Invalid Message", this);
		}
	
		Frame * frame = new Frame(scope, message, this);
		
		return frame->apply();
	}

	Frame * Frame::previous () {
		return m_previous;
	}

	Value * Frame::scope () {
		return m_scope;
	}

	Value * Frame::function () {
		return m_function;
	}

	Cell * Frame::operands () {
		return m_message->tailAs<Cell>();
	}

	Cell * Frame::unwrap () {
		if (m_arguments != NULL) return m_arguments;
		
		Cell * last = NULL;
		Cell * cur = operands();
		
		while (cur) {
			Value * value = NULL;
			
			// If cur->head() == NULL, the result is also NULL.
			if (cur->head())
				value = cur->head()->evaluate(this);
			
			last = Cell::append(last, value, m_arguments);
			
			cur = dynamic_cast<Cell*>(cur->tail());
		}
		
		return m_arguments;
	}

	Cell * Frame::arguments () {
		return m_arguments;
	}

	bool Frame::top () {
		return this == m_previous;
	}
	
	Cell::ArgumentExtractor Frame::extract() {
		Cell * args = unwrap();
		
		if (args == NULL) {
			throw Exception("No arguments provided!", this);
		}
		
		return args->extract(this);
	}

	void Frame::debug () {
		Frame * cur = this;
		
		do {
			Cell cell(cur->function(), cur->operands());
			
			std::cerr << "Frame " << cur << ":" << std::endl;
			
			if (cur->scope())
				std::cerr << "\t Scope: " << Value::toString(cur->scope()) << std::endl;
			
			std::cerr << "\t Function: " << Value::toString(&cell) << std::endl;
			
			if (cur->arguments())
				std::cerr << "\t Arguments: " << Value::toString(cur->arguments()) << std::endl;
			
			cur = cur->previous();
		} while (!cur->top());
	}
	
	void Frame::import (Table * context) {
		context->update(new Symbol("this"), KFunctionWrapper(Frame::scope));
		context->update(new Symbol("trace"), KFunctionWrapper(Frame::trace));
		context->update(new Symbol("unwrap"), KFunctionWrapper(Frame::unwrap));
		context->update(new Symbol("wrap"), KFunctionWrapper(Frame::wrap));
		context->update(new Symbol("with"), KFunctionWrapper(Frame::with));
	}
	
	Value * Frame::scope (Frame * frame) {
		return frame->scope();
	}
	
	Value * Frame::trace (Frame * frame) {
		Cell * arguments = frame->unwrap();
		
		std::cerr << Value::toString(arguments) << std::endl;
		
		return NULL;
	}
	
	Value * Frame::unwrap (Frame * frame) {
		return frame->unwrap();
	}
	
	class Wrapper : public Value {
		private:
			Value * m_value;
		
		public:
			Wrapper (Value * value) : m_value(value)
			{
			
			}
			
			virtual Value * evaluate (Frame * frame) {
				return m_value;
			}
			
			virtual void toCode (StringStreamT & buffer) {
				buffer << "(wrap ";
				m_value->toCode(buffer);
				buffer << ')';
			}
	};
	
	Value * Frame::wrap (Frame * frame) {
		Value * value;
		
		frame->extract()[value];
		
		return new Wrapper(value);
	}
	
	Value * Frame::with (Frame * frame) {
		Cell * cur = frame->operands();
		Value * scope = frame->scope();
		
		while (cur != NULL) {
			scope = frame->call(scope, cur->headAs<Cell>());
			
			cur = cur->tailAs<Cell>();
		}
		
		return scope;
	}

}
