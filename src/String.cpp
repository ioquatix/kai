//
//  String.cpp
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 29/12/11.
//  Copyright (c) 2011 Orion Transfer Ltd. All rights reserved.
//

#include "String.h"

#include "Unicode/Unicode.h"

// For String constructor
#include "Parser/Strings.h"

#include "Cell.h"
#include "Frame.h"
#include "Function.h"
#include "Number.h"
#include "Symbol.h"

namespace Kai {
	
	const char * const String::NAME = "String";
	
	String::String (const StringT & value, bool unescape) : _value(value) {
		if (unescape) {
			_value = Parser::unescape_string(_value);
		}
	}
	
	String::~String () {
	}
	
	Ref<Symbol> String::identity(Frame * frame) const {
		return frame->sym(NAME);
	}
	
	// Returns a function that takes a buffer.
	Ref<Object> String::interpolation(Frame * frame) const {
		Ref<Cell> result, current;
		
		Cell::append(frame, current, frame->sym("block"), result);
		
		return result;
	}
	
	ComparisonResult String::compare(const Object * other) const {
		return derived_compare(this, other);
	}
	
	ComparisonResult String::compare(const String * other) const {
		return (ComparisonResult)(_value.compare(other->_value));
	}
	
	void String::to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const {
		buffer << Parser::escape_string(_value);
	}
	
	Ref<Object> String::join(Frame * frame) {
		StringStreamT buffer;
		String * self;
		
		Cell * next = frame->extract()(self, "self");
		
		buffer << self->value();
		
		while (next != NULL) {
			String * head = next->head().as<String>();
			
			if (head) {
				buffer << head->value();
			} else {
				buffer << Object::to_string(frame, next->head());
			}
			
			next = next->tail().as<Cell>();
		}
		
		return new(frame) String(buffer.str());
	}
	
	Ref<Object> String::size (Frame * frame) {
		String * self;
		frame->extract()(self, "self");
		
		return new(frame) Integer(self->value().size());
	}
	
	Ref<Object> String::at (Frame * frame) {
		String * self;
		Integer * offset;
		frame->extract()(self, "self")(offset, "offset");
		
		// Bounds checking
		if (offset->value() < 0 || offset->value() > self->value().size()) {
			throw Exception("Invalid Offset!", offset, frame);
		}
		
		StringT character(1, self->value()[offset->value().to_intermediate()]);
		
		return new(frame) String(character);
	}
	
	Ref<Object> String::length (Frame * frame) {
		String * self;
		
		frame->extract()(self, "self");
		
		std::size_t result = utf8::distance(self->_value.begin(), self->_value.end());
		
		return new(frame) Integer(result);
	}
	
	Ref<Object> String::each (Frame * frame) {
		String * self;
		Object * function;
		
		frame->extract()(self)(function);
		
		StringT::iterator current = self->_value.begin();
		StringT::iterator previous = current;
		
		Cell * last = NULL, * first = NULL;
		
		while (current != self->_value.end()) {
			/* Unicode::CodePointT value = */ Unicode::next(current, self->_value.end());
			
			// Create a buffer to contain the single character:			
			String * character = new(frame) String(StringT(previous, current));
			Cell * message = Cell::create(frame)(function)(character);
			Ref<Object> result = frame->call(message);
			
			last = Cell::append(frame, last, result, first);
			
			previous = current;
		}
		
		return first;
	}

	Ref<Object> String::fixed_width(Frame * frame) {
		String * self;

		frame->extract()(self, "self");

		std::size_t width = Unicode::fixed_width(self->value());

		return new(frame) Integer(width);
	}
	
	Ref<Object> String::heredoc(Frame * frame) {
		Symbol * identifier;
		String * indentation;
		String * body;
		
		frame->extract(false)(identifier)(indentation)(body);
		
		return body;
	}
	
	void String::import(Frame * frame) {
		Table * prototype = new(frame) Table;
		
		prototype->update(frame->sym("+"), KAI_BUILTIN_FUNCTION(String::join));
		prototype->update(frame->sym("size"), KAI_BUILTIN_FUNCTION(String::size));
		prototype->update(frame->sym("at"), KAI_BUILTIN_FUNCTION(String::at));
		
		prototype->update(frame->sym("each"), KAI_BUILTIN_FUNCTION(String::each));
		prototype->update(frame->sym("length"), KAI_BUILTIN_FUNCTION(String::length));
		prototype->update(frame->sym("fixed-width"), KAI_BUILTIN_FUNCTION(String::fixed_width));
		
		frame->update(frame->sym("String"), prototype);
		frame->update(frame->sym("heredoc"), KAI_BUILTIN_FUNCTION(String::heredoc));
	}

// MARK: -

	const char * const StringBuffer::NAME = "StringBuffer";
	
	StringBuffer::StringBuffer() {
	}
	
	StringBuffer::~StringBuffer () {
	}
	
	Ref<Symbol> StringBuffer::identity(Frame * frame) const {
		return frame->sym(NAME);
	}
	
	void StringBuffer::append(const StringT & string) {
		_value << string;
	}
	
	StringT StringBuffer::to_string() const {
		return _value.str();
	}
	
	Ref<Object> StringBuffer::new_(Frame * frame) {
		return new(frame) StringBuffer;
	}
	
	Ref<Object> StringBuffer::append(Frame * frame) {
		StringBuffer * self;
		String * string;
		
		frame->extract()(self)(string);
		
		self->append(string->value());
		
		return self;
	}
	
	Ref<Object> StringBuffer::to_string(Frame * frame) {
		StringBuffer * self;
		
		frame->extract()(self);
		
		return new(frame) String(self->to_string());
	}
	
	void StringBuffer::import(Frame * frame) {
		Table * prototype = new(frame) Table;
		
		prototype->update(frame->sym("append"), KAI_BUILTIN_FUNCTION(StringBuffer::append));
		prototype->update(frame->sym("to-string"), KAI_BUILTIN_FUNCTION(StringBuffer::to_string));
		prototype->update(frame->sym("new"), KAI_BUILTIN_FUNCTION(StringBuffer::new_));
		
		frame->update(frame->sym("StringBuffer"), prototype);
	}

	
}
