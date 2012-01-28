//
//  Object.cpp
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 28/12/11.
//  Copyright (c) 2011 Orion Transfer Ltd. All rights reserved.
//

#include "Object.h"
#include "Cell.h"
#include "String.h"
#include "Symbol.h"
#include "Frame.h"
#include "Function.h"
#include "Number.h"

namespace Kai
{
	const char * const Object::NAME = "Object";
	
	Object::~Object() {
		
	}
	
	Ref<Symbol> Object::identity(Frame * frame) const {
		return frame->sym(NAME);
	}
	
	Ref<Object> Object::prototype(Frame * frame) const {
		return frame->lookup(identity(frame));
	}
	
	Ref<Object> Object::lookup(Frame * frame, Symbol * identifier) {
		if (Object::equal(identifier, identity(frame))) {
			return NULL;
		}
		
		Ref<Object> _prototype = prototype(frame);
		
		if (_prototype)
			return _prototype->lookup(frame, identifier);
		else
			return NULL;		
	}
	
	Ref<Object> Object::evaluate(Frame * frame) {
		return this;
	}
	
	ComparisonResult Object::compare(const Object * other) const {
		throw InvalidComparison();
	}
	
	void Object::to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const {
		buffer << "(" << identity(frame)->value() << "@" << this << ")";
	}
	
	StringT Object::to_string(Frame * frame, Object * object) {
		if (object) {
			StringStreamT buffer;
			object->to_code(frame, buffer);
			return buffer.str();
		} else {
			return "nil";
		}
	}
	
	bool Object::to_boolean(Frame * frame, Object * object) {
		if (object) {
			return true;
		} else {
			return false;
		}
	}
	
	Ref<Object> Object::as_value(Frame * frame) {
		return Cell::create(frame)(frame->sym("value"))(this);
	}
	
#pragma mark -
	
	Ref<Object> Object::to_string(Frame * frame) {
		Object * value;
		
		frame->extract()[value];
		
		return new(frame) String(Object::to_string(frame, value));
	}
	
	Ref<Object> Object::to_boolean (Frame * frame) {
		Object * value;
		
		frame->extract()[value];
		
		if (Object::to_boolean(frame, value)) {
			return Symbol::true_symbol(frame);
		} else {
			return Symbol::false_symbol(frame);
		}
	}
	
	Ref<Object> Object::compare(Frame * frame) {
		Object * lhs, * rhs;
		
		frame->extract()[lhs][rhs];
		
		ComparisonResult c = EQUAL;
		
		try {
			c = Object::compare(lhs, rhs);
		} catch (InvalidComparison) {
			throw Exception("Invalid Comparison", frame);
		}
		
		return new(frame) Integer(c);
	}
	
	Ref<Object> Object::equal(Frame * frame) {
		Object * lhs, * rhs;
		
		frame->extract()[lhs][rhs];
		
		ComparisonResult c = EQUAL;
		
		try {
			c = Object::compare(lhs, rhs);
		} catch (InvalidComparison) {
			throw Exception("Invalid Comparison", frame);
		}
		
		if (c == EQUAL) {
			return frame->sym("true");
		} else {
			return NULL;
		}
	}
	
	Ref<Object> Object::prototype_(Frame * frame) {
		Object * value = NULL;
		
		frame->extract()(value, "object");
		
		return value->prototype(frame);
	}
	
	Ref<Object> Object::identity_(Frame * frame) {
		Object * value = NULL;
		
		frame->extract()(value, "object", false);
		
		if (value)
			return value->identity(frame);
		else
			return Symbol::nil_symbol(frame);
	}
	
	Ref<Object> Object::value(Frame * frame) {
		if (frame->operands()) {
			return frame->operands()->head();
		}
		
		return NULL;
	}
	
	Ref<Object> Object::lookup(Frame * frame) {		
		Cell * cur = frame->operands();
		Ref<Object> value = NULL;
		
		while (cur) {
			if (!cur->head()) {
				throw Exception("Invalid Name", cur, frame);
			}
			
			value = cur->head()->evaluate(frame);
			
			Cell * tail = cur->tail().as<Cell>();
			if (!tail) break;
			
			if (!value) {
				throw Exception("Null Scope", cur->head(), frame);
			}
			
			frame = new(frame) Frame(value, frame);
			cur = tail;
		}
		
		return value;
	}
	
	Ref<Object> Object::call(Frame * frame) {
		Object * self;
		Cell * body;
		
		frame->extract()(self, "self")(body, "body");
		
		// Wrap self so we can pass it to other functions
		self = self->as_value(frame);
		
		Symbol * function_name = body->head().as<Symbol>();		
		//std::cerr << "Calling " << Object::to_string(function_name) << " for " << Object::to_string(self) << std::endl;
		
		Cell * dispatch = Cell::create(frame)(frame->sym("lookup"))(self)(function_name);
		
		Cell * arguments = new(frame) Cell(self, body->tail());
		Cell * call = new(frame) Cell(dispatch, arguments);
		
		return call->evaluate(frame);
	}
	
	void Object::import(Frame * frame) {
		Ref<Table> prototype = new(frame) Table;
		
		prototype->update(frame->sym("identity"), KAI_BUILTIN_FUNCTION(Object::identity_));
		prototype->update(frame->sym("to-string"), KAI_BUILTIN_FUNCTION(Object::to_string));
		prototype->update(frame->sym("to-boolean"), KAI_BUILTIN_FUNCTION(Object::to_boolean));
		prototype->update(frame->sym("<=>"), KAI_BUILTIN_FUNCTION(Object::compare));
		prototype->update(frame->sym("=="), KAI_BUILTIN_FUNCTION(Object::equal));
		prototype->update(frame->sym("prototype"), KAI_BUILTIN_FUNCTION(Object::prototype_));
		prototype->update(frame->sym("value"), KAI_BUILTIN_FUNCTION(Object::value));
		
		prototype->update(frame->sym("lookup"), KAI_BUILTIN_FUNCTION(Object::lookup));
		prototype->update(frame->sym("call"), KAI_BUILTIN_FUNCTION(Object::call));
		
		frame->update(frame->sym("Object"), prototype);
	}	
}
