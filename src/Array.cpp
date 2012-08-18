//
//  Array.cpp
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 21/09/10.
//  Copyright 2010 Orion Transfer Ltd. All rights reserved.
//
//

#include "Array.h"
#include "Cell.h"
#include "Frame.h"
#include "Number.h"
#include "Table.h"
#include "Symbol.h"
#include "Function.h"

namespace Kai {
	
	const char * const Array::NAME = "Array";
	
	Array::Array() {
	}
	
	Array::~Array() {
	}
	
	Ref<Symbol> Array::identity(Frame * frame) const {
		return frame->sym(NAME);
	}
	
	void Array::mark(Memory::Traversal * traversal) const {
		for (ConstIteratorT a = _value.begin(); a != _value.end(); a++) {
			traversal->traverse(*a);
		}
	}
	
	ComparisonResult Array::compare(const Object * other) const {
		return derived_compare(this, other);
	}
	
	ComparisonResult Array::compare(const Array * other) const {
		std::size_t lhs = _value.size(), rhs = other->_value.size();
		
		if (lhs < rhs)
			return ASCENDING;
		else if (lhs > rhs)
			return DESCENDING;
		
		ConstIteratorT a = _value.begin();
		ConstIteratorT b = other->_value.begin();
		
		for (; a != _value.end(); a++, b++) {
			ComparisonResult result = (*a)->compare(*b);
			
			if (result != 0)
				return result;
		}
		
		return EQUAL;
	}
	
	void Array::to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const
	{
		if (marks.find(this) != marks.end()) {
			buffer << "(Array@" << this << " ...)";
		} else {
			marks.insert(this);
			
			buffer << "(Array@" << this;
			
			for (ConstIteratorT a = _value.begin(); a != _value.end(); a++) {
				if (*a) {
					buffer << " ";
					(*a)->to_code(frame, buffer, marks, indentation);
				} else {
					buffer << " nil";
				}
			}
			
			buffer << ")";
		}
	}
	
	Ref<Object> Array::new_ (Frame * frame)
	{
		Array * array = new(frame) Array();
		Cell * items = frame->unwrap()->tail().as<Cell>();
		
		while (items) {
			array->_value.push_back(items->head());
			
			items = items->tail().as<Cell>();
		}
		
		return array;
	}
	
	Ref<Object> Array::minimum(Frame * frame) {
		return NULL;
	}
	
	Ref<Object> Array::maximum(Frame * frame) {
		return NULL;
	}
	
	Ref<Object> Array::at(Frame * frame) {
		Array * self = NULL;
		Integer * _offset = NULL;
		
		frame->extract()(self, "self")(_offset, "offset");
		
		std::size_t offset = _offset->value().to_size();
		
		if (offset < self->_value.size()) {
			return self->_value[offset];
		} else {
			throw RangeError("Index out of bounds", _offset, frame);
		}
	}
	
	Ref<Object> Array::push_back(Frame * frame) {
		Array * self = NULL;
		
		ArgumentExtractor arguments = frame->extract()(self, "self");
		
		while (arguments) {
			Object * item = NULL;
			
			arguments = arguments(item, "item", false);
			
			self->_value.push_back(item);
		}
		
		return self;
	}
	
	Ref<Object> Array::pop_back(Frame * frame) {
		Array * self = NULL;
		
		frame->extract()(self);
		
		if (self->_value.size() == 0)
			return NULL;
		
		Object * object = self->_value.back();
		
		self->_value.pop_back();
		
		return object;
	}
	
	Ref<Object> Array::push_front(Frame * frame) {
		Array * self = NULL;
		Object * value = NULL;
		ArgumentExtractor arguments = frame->extract()(self);
		
		self->_value.push_front(value);
		
		return self;
	}
	
	Ref<Object> Array::pop_front(Frame * frame) {
		Array * self = NULL;
		
		frame->extract()(self);
		
		if (self->_value.size() == 0)
			return NULL;
		
		Object * object = self->_value.front();
		
		self->_value.pop_front();
		
		return object;
	}	
	
	Ref<Object> Array::append(Frame * frame) {
		return NULL;
	}
	
	Ref<Object> Array::prepend(Frame * frame) {
		return NULL;
	}
	
	Ref<Object> Array::insert(Frame * frame) {
		return NULL;
	}
	
	
	Ref<Object> Array::includes(Frame * frame) {
		return NULL;
	}
	
	
	Ref<Object> Array::each(Frame * frame) {
		Array * self;
		Object * callback;
		
		frame->extract()(self, "self")(callback, "callback");
		
		for (IteratorT a = self->_value.begin(); a != self->_value.end(); a++) {
			Cell * message = Cell::create(frame)(callback)(*a);
			frame->call(message);
		}
		
		return self;
	}
	
	Ref<Object> Array::collect(Frame * frame) {
		Array * self = NULL;
		Object * function = NULL;
		
		frame->extract()(self)(function);
		
		Array * result = new(frame) Array();
		
		for (IteratorT a = self->_value.begin(); a != self->_value.end(); a++) {
			Cell * message = Cell::create(frame)(function)(*a);
			Ref<Object> v = frame->call(message);
			
			result->_value.push_back(v);
		}
		
		return result;
	}
	
	Ref<Object> Array::select(Frame * frame) {
		return NULL;
	}
	
	Ref<Object> Array::find(Frame * frame) {
		return NULL;
	}
	
	void Array::import(Frame * frame) {
		Table * prototype = new(frame) Table;
		
		prototype->update(frame->sym("push-back!"), KAI_BUILTIN_FUNCTION(Array::push_back));
		prototype->update(frame->sym("pop-back!"), KAI_BUILTIN_FUNCTION(Array::pop_back));
		prototype->update(frame->sym("push-front!"), KAI_BUILTIN_FUNCTION(Array::push_front));
		prototype->update(frame->sym("pop-front!"), KAI_BUILTIN_FUNCTION(Array::pop_front));
		
		prototype->update(frame->sym("at"), KAI_BUILTIN_FUNCTION(Array::at));
		
		prototype->update(frame->sym("each"), KAI_BUILTIN_FUNCTION(Array::each));
		
		prototype->update(frame->sym("new"), KAI_BUILTIN_FUNCTION(Array::new_));
		
		frame->update(frame->sym("Array"), prototype);
	}
	
}
