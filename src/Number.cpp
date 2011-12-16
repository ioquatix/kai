//
//  Number.cpp
//  Kai
//
//  Created by Samuel Williams on 16/12/11.
//  Copyright (c) 2011 Orion Transfer Ltd. All rights reserved.
//

#include "Number.h"
#include "Frame.h"
#include "Table.h"
#include "Function.h"

namespace Kai {
	Integer::Integer (ValueT value) : m_value(value) {
	}
	
	Integer::~Integer () {
	}
	
	Ref<Value> Integer::sum (Frame * frame) {
		ValueT total = 0;
		
		// Evaluate the given arguments
		Cell * args = frame->unwrap();
		
		while (args != NULL) {
			// For each argument, extract it as an Integer value
			Integer * integer = args->headAs<Integer>();
			
			if (integer) {
				// If it was an integer, add its value to the total
				total += integer->value();
			} else {
				// If it wasn't an integer, throw an exception.
				throw Exception("Invalid Integer Value", frame);
			}
			
			// Move to the next argument.
			args = args->tailAs<Cell>();
		}
		
		// Return a new integer with the calculated sum.
		return new Integer(total);
	}
	
	Ref<Value> Integer::subtract (Frame * frame) {
		ValueT total = 0;
		Integer * first;
		
		Cell * args = frame->extract()(first);
		
		total = first->value();
		
		while (args != NULL) {
			Integer * integer = args->headAs<Integer>();
			
			if (integer) {
				total -= integer->value();
			} else {
				throw Exception("Invalid Integer Value", frame);
			}
			
			args = args->tailAs<Cell>();
		}
		
		return new Integer(total);
	}
	
	Ref<Value> Integer::product (Frame * frame) {
		ValueT total = 1;
		
		Cell * args = frame->unwrap();
		
		while (args != NULL) {
			Integer * integer = args->headAs<Integer>();
			
			if (integer) {
				total *= integer->value();
			} else {
				throw Exception("Invalid Integer Value", frame);
			}
			
			args = args->tailAs<Cell>();
		}
		
		return new Integer(total);
	}
	
	Ref<Value> Integer::modulus (Frame * frame) {
		Integer * number, * base;
		
		frame->extract()[number][base];
		
		return new Integer(number->value() % base->value());
	}
	
	Ref<Value> Integer::greatest_common_divisor(Frame * frame) {
		Integer * a = NULL, * b = NULL;
		
		frame->extract()(a)(b);
		
		Math::Integer result;
		result.calculate_greatest_common_divisor(a->value(), b->value());
		
		return new Integer(result);
	}
	
	Ref<Value> Integer::generate_prime(Frame * frame) {
		Value * self = NULL;
		Integer * length = NULL;
		
		frame->extract()(self)(length);
		
		Math::Integer prime;
		prime.generate_prime(length->value().to_int64());
		
		return new Integer(prime);
	}
	
	Ref<Value> Integer::to_string(Frame * frame) {
		Integer * self;
		Integer * radix;
		
		frame->extract()(self)[radix];
		
		Math::Integer base;
		
		if (radix) {
			base = radix->value();
		} else {
			base = 10;
		}
		
		return new String(self->value().to_string(base));
	}
	
	Ref<Value> Integer::globalPrototype () {
		static Ref<Table> g_prototype;
		
		if (!g_prototype) {
			g_prototype = new Table;
			
			g_prototype->update(sym("+"), KAI_BUILTIN_FUNCTION(Integer::sum));
			g_prototype->update(sym("-"), KAI_BUILTIN_FUNCTION(Integer::subtract));
			g_prototype->update(sym("*"), KAI_BUILTIN_FUNCTION(Integer::product));
			g_prototype->update(sym("%"), KAI_BUILTIN_FUNCTION(Integer::modulus));
			
			g_prototype->update(sym("generate-prime"), KAI_BUILTIN_FUNCTION(Integer::generate_prime));
			g_prototype->update(sym("greatest-common-divisor"), KAI_BUILTIN_FUNCTION(Integer::greatest_common_divisor));
			
			g_prototype->update(sym("to-string"), KAI_BUILTIN_FUNCTION(Integer::to_string));
		}
		
		return g_prototype;
	}
	
	Ref<Value> Integer::prototype () {		
		return globalPrototype();
	}
	
	int Integer::compare (const Value * other) const {
		return derivedCompare(this, other);
	}
	
	int Integer::compare (const Integer * other) const {
		return m_value.compare_with(other->m_value);
	}
	
	void Integer::toCode(StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const {
		buffer << m_value;
	}
	
	void Integer::import (Table * context) {
		context->update(sym("Integer"), globalPrototype());
	}
}
