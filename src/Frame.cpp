/*
 *  Frame.cpp
 *  This file is part of the "Kai" project, and is licensed under the GNU GPLv3.
 *
 *  Created by Samuel Williams on 12/04/10.
 *  Copyright 2010 Samuel Williams. All rights reserved.
 *
 */

#include "Frame.h"
#include <iostream>

namespace Kai {

	Frame::Frame (Value * caller)
		: m_previous(this), m_caller(caller), m_function(NULL), m_operands(NULL), m_arguments(NULL)
	{

	}

	Frame::Frame (Value * caller, Symbol * function, Cell * operands, Frame * previous)
		: m_previous(previous), m_caller(caller), m_function(function), m_operands(operands), m_arguments(NULL)
	{

	}

	Value * Frame::call (Cell * functionAndOperands)
	{
		Symbol * function = dynamic_cast<Symbol*>(functionAndOperands->head());
		Cell * operands = dynamic_cast<Cell*>(functionAndOperands->tail());

		return call(function, operands);
	}

	Value * Frame::call (Symbol * function, Cell * operands)
	{
		return call(m_caller, m_caller, function, operands);
	}

	Value * Frame::call (Value * caller, Value * target, Symbol * function, Cell * operands)
	{
		Frame * frame = new Frame(caller, function, operands, this);
		
		//std::cerr << "Calling " << Value::toString(frame->function()) << " with operands " << Value::toString(operands) << std::endl;
		
		return target->invoke(frame);
	}

	Frame * Frame::previous () {
		return m_previous;
	}

	Value * Frame::caller () {
		return m_caller;
	}

	Symbol * Frame::function () {
		return m_function;
	}

	Cell * Frame::operands () {
		return m_operands;
	}

	Cell * Frame::unwrap () {
		if (m_arguments != NULL) return m_arguments;
		
		Cell * last = NULL;
		Cell * cur = m_operands;
		
		while (cur) {
			Value * value = cur->head()->evaluate(this);
			
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

	void Frame::debug () {
		Frame * cur = this;
		
		do {
			Cell cell(cur->function(), cur->operands());
			
			std::cerr << "Frame " << cur << ":" << std::endl;
			
			if (cur->caller())
				std::cerr << "\t Caller: " << Value::toString(cur->caller()) << std::endl;
			
			std::cerr << "\t Function: " << Value::toString(&cell) << std::endl;
			
			if (cur->arguments())
				std::cerr << "\t Arguments: " << Value::toString(cur->arguments()) << std::endl;
			
			cur = cur->previous();
		} while (!cur->top());
	}
}
