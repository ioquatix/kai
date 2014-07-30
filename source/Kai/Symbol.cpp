//
//  Symbol.cpp
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 28/12/11.
//  Copyright (c) 2011 Orion Transfer Ltd. All rights reserved.
//

#include "Symbol.hpp"
#include "Frame.hpp"
#include "Cell.hpp"
#include "Function.hpp"
#include "Number.hpp"

namespace Kai {
	
	const char * const Symbol::NAME = "Symbol";
	
	HashT Symbol::calculate_hash(const char * value) {
		HashT sum = 0;
		
		while (*value) {
			sum += *value;
			
			++value;
		}
		
		return sum;
	}
	
	Symbol::Symbol(const StringT & value) : _value(value), _hash(calculate_hash(value.c_str())) {
		
	}
	
	Symbol::~Symbol() {
		
	}
	
	Ref<Symbol> Symbol::identity(Frame * frame) const {
		return frame->sym(NAME);
	}
	
	ComparisonResult Symbol::compare(const Object * other) const {
		return derived_compare(this, other);
	}
	
	ComparisonResult Symbol::compare(const Symbol * other) const {
		if (_hash < other->_hash) {
			return ASCENDING;
		} else if (_hash > other->_hash) {
			return DESCENDING;
		} else {
			return (ComparisonResult)(_value.compare(other->_value));
		}		
	}
	
	void Symbol::to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const {
		buffer << _value;
	}
	
	Ref<Object> Symbol::evaluate(Frame * frame) {
		if (_value[0] != ':') {
			return frame->lookup(this);
		} else {
			return ptr(this);
		}
	}
	
	Symbol * Symbol::nil_symbol(Frame * frame) {
		return frame->sym("nil");
	}
	
	Symbol * Symbol::false_symbol(Frame * frame) {
		return frame->sym("false");
	}
	
	Symbol * Symbol::true_symbol(Frame * frame) {
		return frame->sym("true");
	}
	
	Ref<Object> Symbol::hash(Frame * frame) {
		Symbol * self;
		
		frame->extract()(self, "self");
		
		return new(frame) Integer(self->_hash);
	}
	
	Ref<Object> Symbol::assign(Frame * frame) {
		Symbol * self;
		Object * value;
		
		frame->extract()(self, "self")[value];
		
		Table * scope = ptr(frame->scope()).as<Table>();
		
		if (scope) {
			scope->update(self, value);
		} else {
			throw Exception("Invalid Scope", frame->scope(), frame);
		}
		
		return value;
	}
	
	void Symbol::import(Frame * frame) {
		Table * prototype = new(frame) Table;
		
		prototype->update(frame->sym("="), KAI_BUILTIN_FUNCTION(Symbol::assign));
		prototype->update(frame->sym("hash"), KAI_BUILTIN_FUNCTION(Symbol::hash));
		
		frame->update(frame->sym("Symbol"), prototype);
	}
	
// MARK: -
	
	// This implementation is incomplete.
	Symbol * SymbolTable::fetch(Frame * frame, const char * name) {
		//HashT hash = Symbol::calculate_hash(name);
		
		return new Symbol(name);
	}
	
}
