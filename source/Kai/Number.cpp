//
//  Number.cpp
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 16/12/11.
//  Copyright (c) 2011 Orion Transfer Ltd. All rights reserved.
//

#include "Number.h"
#include "Cell.h"
#include "Ensure.h"
#include "Frame.h"
#include "Table.h"
#include "String.h"
#include "Symbol.h"
#include "Function.h"

namespace Kai {
	
	const char * const Integral::NAME = "Integral";
	
	Integral::~Integral() {
		
	}
	
// MARK: -
	
	const char * const Integer::NAME = "Integer";
	
	Integer::Integer (ValueT value) : _value(value) {
	}
	
	Integer::~Integer () {
	}
	
	Ref<Symbol> Integer::identity(Frame * frame) const {
		return frame->sym("Integer");
	}
	
	Math::Integer Integer::to_integer() const {
		return _value;
	}
	
	Ref<Object> Integer::sum (Frame * frame) {
		ValueT total = 0;
		
		// Evaluate the given arguments
		Cell * args = frame->unwrap();
		
		while (args != NULL) {
			// For each argument, extract it as an Integer value
			Integer * integer = args->head().as<Integer>();
			
			if (integer) {
				// If it was an integer, add its value to the total
				total += integer->value();
			} else {
				// If it wasn't an integer, throw an exception.
				throw Exception("Invalid Integer Value", frame);
			}
			
			// Move to the next argument.
			args = args->tail().as<Cell>();
		}
		
		// Return a new integer with the calculated sum.
		return new(frame) Integer(total);
	}
	
	Ref<Object> Integer::subtract (Frame * frame) {
		ValueT total = 0;
		Integer * first;
		
		ArgumentExtractor arguments = frame->extract();
		arguments = arguments(first, "left-value");
		
		total = first->value();
		
		while (arguments) {
			Integer * integer;
			
			arguments = arguments(integer, "right-value");
			
			total -= integer->value();
		}
		
		return new(frame) Integer(total);
	}
	
	Ref<Object> Integer::product (Frame * frame) {
		ValueT total = 1;
		
		ArgumentExtractor arguments = frame->extract();
		
		while (arguments) {
			Integer * integer;
			
			arguments = arguments(integer, "right-value");
			
			total *= integer->value();
		}
		
		return new(frame) Integer(total);
	}
	
	Ref<Object> Integer::modulus (Frame * frame) {
		Integer * number, * base;
		
		frame->extract()(number, "self")(base, "base");
		
		return new(frame) Integer(number->value() % base->value());
	}
	
	Ref<Object> Integer::power (Frame * frame) {
		Integer * base, * exponent;
		
		frame->extract()(base, "base")(exponent, "exponent");
		
		Math::Integer result;
		result.set_power(base->value(), exponent->value());
		
		return new(frame) Integer(result);
	}
	
	Ref<Object> Integer::fractional_part(Frame * frame) {
		Integer * self;
		Integral * scale, * base;
		
		frame->extract()(self, "self")(scale, "scale")[base];
		
		Math::Integer result;
		
		if (base) {
			result = self->value().fractional_part(scale->to_integer().to_intermediate(), base->to_integer());
		} else {
			result = self->value().fractional_part(scale->to_integer().to_intermediate());
		}
		
		return new(frame) Integer(result);
	}
	
	Ref<Object> Integer::greatest_common_divisor(Frame * frame) {
		Integer * a = NULL, * b = NULL;
		
		frame->extract()(a, "a")(b, "b");
		
		Math::Integer result;
		result.calculate_greatest_common_divisor(a->value(), b->value());
		
		return new(frame) Integer(result);
	}
	
	Ref<Object> Integer::generate_prime(Frame * frame) {
		Object * self = NULL;
		Integral * length = NULL;
		
		frame->extract()(self, "self")(length, "word-length");
		
		Math::Integer prime;
		prime.generate_prime(length->to_integer().to_intermediate());
		
		return new(frame) Integer(prime);
	}
	
	Ref<Object> Integer::from_string(Frame * frame) {
		Object * self;
		String * string;
		Integral * radix;
		
		frame->extract()(self, "self")(string, "text")[radix];
		
		Math::BaseT base;
		
		if (radix) {
			base = radix->to_integer().to_digit();
		} else {
			base = 10;
		}
		
		Math::Integer value(string->value(), base);
		
		return new(frame) Integer(value);
	}
	
	Ref<Object> Integer::to_string(Frame * frame) {
		Integer * self;
		Integral * radix;
		
		frame->extract()(self, "self")[radix];
		
		Math::Integer base;
		
		if (radix) {
			base = radix->to_integer();
		} else {
			base = 10;
		}
		
		return new(frame) String(self->value().to_string(base.to_digit()));
	}
	
	Ref<Object> Integer::to_number(Frame * frame) {
		Integer * self;
		
		frame->extract()(self, "self");
		
		Math::Number value(self->value(), 0);
		
		return new(frame) Number(value);
	}
	
	ComparisonResult Integer::compare(const Object * other) const {
		return derived_compare(this, other);
	}
	
	ComparisonResult Integer::compare(const Integer * other) const {
		return (ComparisonResult)(_value.compare_with(other->_value));
	}
	
	void Integer::to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const {
		buffer << _value.to_hexadecimal();
	}
	
	void Integer::import(Frame * frame) {
		Table * prototype = new(frame) Table;
		
		prototype->update(frame->sym("+"), KAI_BUILTIN_FUNCTION(Integer::sum));
		prototype->update(frame->sym("-"), KAI_BUILTIN_FUNCTION(Integer::subtract));
		prototype->update(frame->sym("*"), KAI_BUILTIN_FUNCTION(Integer::product));
		prototype->update(frame->sym("%"), KAI_BUILTIN_FUNCTION(Integer::modulus));
		prototype->update(frame->sym("**"), KAI_BUILTIN_FUNCTION(Integer::power));
		
		prototype->update(frame->sym("fractional-part"), KAI_BUILTIN_FUNCTION(Integer::fractional_part));
		
		prototype->update(frame->sym("generate-prime"), KAI_BUILTIN_FUNCTION(Integer::generate_prime));
		prototype->update(frame->sym("greatest-common-divisor"), KAI_BUILTIN_FUNCTION(Integer::greatest_common_divisor));
		
		prototype->update(frame->sym("from-string"), KAI_BUILTIN_FUNCTION(Integer::from_string));
		prototype->update(frame->sym("to-string"), KAI_BUILTIN_FUNCTION(Integer::to_string));
		prototype->update(frame->sym("to-number"), KAI_BUILTIN_FUNCTION(Integer::to_number));
		
		prototype->update(frame->sym("radix"), new(frame) Integer(DEFAULT_RADIX));
		
		frame->update(frame->sym("Integer"), prototype);
	}
	
// MARK: -
	
	const char * const Number::NAME = "Number";
	
	Number::Number(ValueT value) : _value(value)
	{
		
	}
	
	Number::~Number()
	{
		
	}
	
	Ref<Symbol> Number::identity(Frame * frame) const {
		return frame->sym("Number");
	}
	
	Math::Integer Number::to_integer() const {
		Math::Number copy = _value;
		
		return copy.whole_part();
	}
	
	ComparisonResult Number::compare(const Object * other) const
	{
		return derived_compare(this, other);
	}
	
	ComparisonResult Number::compare(const Number * other) const
	{
		return (ComparisonResult)(_value.compare_with(other->_value));
	}
	
	void Number::to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const
	{
		buffer << _value.to_string();
	}
	
	Ref<Object> Number::product (Frame * frame)
	{
		ValueT total = 1;
		
		ArgumentExtractor arguments = frame->extract();
		
		while (arguments) {
			Number * number;
			
			arguments = arguments(number, "right-value");
			
			total *= number->value();
		}
		
		return new(frame) Number(total);
	}

	Ref<Object> Number::sum (Frame * frame)
	{
		ValueT total = 0;

		ArgumentExtractor arguments = frame->extract();

		while (arguments) {
			Number * number;

			arguments = arguments(number, "right-value");

			//total = total + number->value();
		}

		return new(frame) Number(total);
	}

	Ref<Object> Number::fraction (Frame * frame)
	{
		Number * numerator, * denominator;
		frame->extract()(numerator, "numerator")(denominator, "denominator");

		//return new(frame) Number(numerator->value() / denominator->value());
		return nullptr;
	}
	
	void Number::import (Frame * frame) {
		Table * prototype = new(frame) Table;
		
		prototype->update(frame->sym("*"), KAI_BUILTIN_FUNCTION(Number::product));
		prototype->update(frame->sym("+"), KAI_BUILTIN_FUNCTION(Number::sum));
		prototype->update(frame->sym("/"), KAI_BUILTIN_FUNCTION(Number::fraction));
		
		frame->update(frame->sym("Number"), prototype);
	}
}
