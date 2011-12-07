/*
 *  Array.cpp
 *  Kai
 *
 *  Created by Samuel Williams on 21/09/10.
 *  Copyright 2010 Orion Transfer Ltd. All rights reserved.
 *
 */

#include "Array.h"
#include "Frame.h"
#include "Function.h"

namespace Kai {

	Array::Array ()
	{
	
	}
	
	Array::~Array ()
	{
	
	}
	
	int Array::compare (const Value * other) const
	{
		return derivedCompare(this, other);
	}
	
	int Array::compare (const Array * other) const
	{
		ComparisonResult result = m_value.size() - other->m_value.size();
		
		if (result != 0) {
			return clampComparison(result);
		}
		
		ConstIteratorT a = m_value.begin();
		ConstIteratorT b = other->m_value.begin();
		
		for (; a != m_value.end(); a++, b++) {
			
			result = (*a)->compare(*b);
			
			if (result != 0)
				return result;
		}
		
		return 0;
	}
	
	void Array::toCode(StringStreamT & buffer, MarkedT & marks, std::size_t indentation)
	{
		if (marks.find(this) != marks.end()) {
			buffer << "(Array@" << this << " ...)";
		} else {
			marks.insert(this);
			
			buffer << "(Array@" << this;
			
			for (ConstIteratorT a = m_value.begin(); a != m_value.end(); a++) {
				if (*a) {
					buffer << " ";
					(*a)->toCode(buffer, marks, indentation);
				} else {
					buffer << " nil";
				}
			}
			
			buffer << ")";
		}
	}
	
	Ref<Value> Array::_new (Frame * frame)
	{
		Array * array = new Array();
		Cell * items = frame->unwrap()->tailAs<Cell>();
		
		while (items) {
			array->m_value.push_back(items->head());
			
			items = items->tailAs<Cell>();
		}
		
		return array;
	}
	
	Ref<Value> Array::minimum (Frame * frame)
	{
		return NULL;
	}
	
	Ref<Value> Array::maximum (Frame * frame)
	{
		return NULL;
	}
	
	Ref<Value> Array::at (Frame * frame)
	{
		return NULL;
	}
	
	Ref<Value> Array::push_back(Frame * frame)
	{
		Array * self = NULL;
		Value * value = NULL;
		frame->extract()(self)[value];
		
		self->m_value.push_back(value);
		
		return self;
	}
	
	Ref<Value> Array::pop_back(Frame * frame)
	{
		return NULL;
	}
				
	Ref<Value> Array::push_front(Frame * frame)
	{
		Array * self = NULL;
		Value * value = NULL;
		frame->extract()(self)[value];
		
		self->m_value.push_front(value);
		
		return self;
	}
	
	Ref<Value> Array::pop_front(Frame * frame)
	{
		return NULL;
	}
	
	
	Ref<Value> Array::append(Frame * frame)
	{
		return NULL;
	}
	
	Ref<Value> Array::prepend(Frame * frame)
	{
		return NULL;
	}
	
	Ref<Value> Array::insert(Frame * frame)
	{
		return NULL;
	}
	
	
	Ref<Value> Array::includes(Frame * frame)
	{
		return NULL;
	}
	
	
	Ref<Value> Array::each(Frame * frame)
	{
		return NULL;
	}
	
	Ref<Value> Array::collect(Frame * frame)
	{
		Array * self = NULL;
		Value * function = NULL;
		
		frame->extract()(self)(function);
		
		Array * result = new Array();
		
		for (IteratorT a = self->m_value.begin(); a != self->m_value.end(); a++) {
			Cell * message = Cell::create(function)(*a);
			Ref<Value> v = frame->call(message);
			
			result->m_value.push_back(v);
		}
		
		return result;
	}
	
	Ref<Value> Array::select(Frame * frame)
	{
		return NULL;
	}
	
	Ref<Value> Array::find(Frame * frame)
	{
		return NULL;
	}
	
	Ref<Value> Array::prototype ()
	{
		return globalPrototype();
	}
	
	Ref<Value> Array::Array::globalPrototype ()
	{
		static Table * g_prototype = NULL;
		
		if (g_prototype == NULL) {
			g_prototype = new Table();
			
			g_prototype->update(sym("push-back!"), KFunctionWrapper(Array::push_back));
			g_prototype->update(sym("pop-back!"), KFunctionWrapper(Array::pop_back));
			g_prototype->update(sym("push-front!"), KFunctionWrapper(Array::push_front));
			g_prototype->update(sym("pop-front!"), KFunctionWrapper(Array::pop_front));
			g_prototype->update(sym("at"), KFunctionWrapper(Array::push_back));
			
			g_prototype->update(sym("new"), KFunctionWrapper(Array::_new));
		}
		
		return g_prototype;
	}

	void Array::import (Table * context)
	{
		context->update(sym("Array"), globalPrototype());
	}

}
