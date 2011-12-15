/*
 *  Value.cpp
 *  This file is part of the "Kai" project, and is licensed under the GNU GPLv3.
 *
 *  Created by Samuel Williams on 10/04/10.
 *  Copyright 2010 Samuel Williams. All rights reserved.
 *
 */

#include "Value.h"
#include <assert.h>
#include <gc/gc_cpp.h>
#include <unistd.h>
#include <iostream>

#include "Frame.h"
#include "Exception.h"
#include "Function.h"
#include "Ensure.h"

#include "Unicode/Unicode.h"

// For String constructor
#include "Parser/Strings.h"

namespace Kai {

	void debug (Value * value) {
		std::cerr << Value::toString(value) << std::endl;
	}

#pragma mark -
#pragma mark Value

	Value::Value () {
	}

	Value::~Value () {
		
	}
	
	void Value::toCode(StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const {
		buffer << "(value{" << this << "})";
	}
	
	Ref<Value> Value::lookup (Symbol * identifier) {
		Ref<Value> proto = prototype();
		
		if (proto) {
			return proto->lookup(identifier);
		}
		
		return NULL;
	}
	
	Ref<Value> Value::prototype () {
		return globalPrototype();
	}

	int Value::compare (const Value * other) const {
		throw InvalidComparison();
	}

	void Value::debug () {
		std::cerr << toString(this) << std::endl;
	}

	Ref<Value> Value::evaluate (Frame * frame) {
		return this;
	}
	
	Ref<Value> Value::asValue () {
		return Cell::create(sym("value"))(this);
	}
	
	StringT Value::toString (const Value * value) {
		if (value) {
			StringStreamT buffer;
			value->toCode(buffer);
			return buffer.str();
		} else {
			return "nil";
		}
	}
	
	bool Value::toBoolean (const Value * value) {
		if (value) {
			return true;
		}
		
		return false;
	}
	
	int Value::compare (const Value * lhs, const Value * rhs) {
		if (lhs && rhs) {
			// Both values are non-NULL
			return lhs->compare(rhs);
		} else {
			// One or both values are NULL
			if (lhs == rhs) {
				return COMPARISON_EQUAL;
			} else if (lhs) {
				return COMPARISON_DESCENDING;
			} else if (rhs) {
				return COMPARISON_ASCENDING;
			}
		}
		
		// We should never get here
		throw InvalidComparison();
	}
	
	bool Value::equal(const Value * lhs, const Value * rhs)
	{
		try {
			return Value::compare(lhs, rhs) == COMPARISON_EQUAL;
		} catch (...) {
			return false;
		}
	}

#pragma mark Builtin Functions

	void Value::import (Table * context) {
		context->update(sym("Value"), globalPrototype());
	}
	
	Ref<Value> Value::globalPrototype () {
		static Ref<Table> g_prototype;
		
		if (!g_prototype) {
			g_prototype = new Table;
			
			g_prototype->update(sym("memory-address"), KAI_BUILTIN_FUNCTION(Value::memory_address));
			g_prototype->update(sym("to-string"), KAI_BUILTIN_FUNCTION(Value::toString));
			g_prototype->update(sym("to-boolean"), KAI_BUILTIN_FUNCTION(Value::toBoolean));
			g_prototype->update(sym("<=>"), KAI_BUILTIN_FUNCTION(Value::compare));
			g_prototype->update(sym("=="), KAI_BUILTIN_FUNCTION(Value::equal));
			g_prototype->update(sym("prototype"), KAI_BUILTIN_FUNCTION(Value::prototype));
			g_prototype->update(sym("value"), KAI_BUILTIN_FUNCTION(Value::value));
			
			g_prototype->update(sym("sleep"), KAI_BUILTIN_FUNCTION(Value::sleep));
			
			g_prototype->update(sym("lookup"), KAI_BUILTIN_FUNCTION(Value::lookup));
			g_prototype->update(sym("call"), KAI_BUILTIN_FUNCTION(Value::call));
		}
		
		return g_prototype;
	}
	
	Ref<Value> Value::call (Frame * frame) {
		Value * self;
		Cell * body;
		
		frame->extract()(self)(body);
		
		// Wrap self so we can pass it to other functions
		self = self->asValue();
		
		Symbol * functionName = body->headAs<Symbol>();
		
		//std::cerr << "Calling " << Value::toString(functionName) << " for " << Value::toString(self) << std::endl;
		
		Cell * dispatch = Cell::create()
			(sym("lookup"))
			(self)
			(functionName);
		
		Cell * arguments = new Cell(self, body->tail());
		
		Cell * call = new Cell(dispatch, arguments);
		
		return call->evaluate(frame);
	}
	
	Ref<Value> Value::lookup (Frame * frame) {		
		Cell * cur = frame->operands();
		Ref<Value> value = NULL;
		
		while (cur) {
			if (!cur->head()) {
				throw Exception("Invalid Name", cur, frame);
			}
			
			value = cur->head()->evaluate(frame);
			
			Cell * tail = cur->tailAs<Cell>();
			if (!tail) break;
			
			if (!value) {
				throw Exception("Null Scope", cur->head(), frame);
			}
			
			frame = new Frame(value, frame);
			cur = tail;
		}
		
		return value;
	}
	
	Ref<Value> Value::toString (Frame * frame) {
		Value * value;
		
		frame->extract()[value];
		
		return new String(Value::toString(value));
	}
	
	Ref<Value> Value::toBoolean (Frame * frame) {
		Value * value;
		
		frame->extract()[value];
		
		if (Value::toBoolean(value)) {
			return Symbol::trueSymbol();
		} else {
			return Symbol::falseSymbol();
		}
	}
	
	Ref<Value> Value::compare (Frame * frame) {
		Value * lhs, * rhs;
		
		frame->extract()[lhs][rhs];
		
		ComparisonResult c = COMPARISON_EQUAL;
		
		try {
			c = Value::compare(lhs, rhs);
		} catch (InvalidComparison) {
			throw Exception("Invalid Comparison", frame);
		}
		
		return new Integer(c);
	}
	
	Ref<Value> Value::equal (Frame * frame) {
		Value * lhs, * rhs;
		
		frame->extract()[lhs][rhs];
		
		ComparisonResult c = COMPARISON_EQUAL;
		
		try {
			c = Value::compare(lhs, rhs);
		} catch (InvalidComparison) {
			throw Exception("Invalid Comparison", frame);
		}
		
		if (c == COMPARISON_EQUAL) {
			return sym("true");
		} else {
			return NULL;
		}
	}
	
	Ref<Value> Value::sleep (Frame * frame)
	{
		Integer * duration;
		frame->extract()(duration);
		
		::sleep(duration->value());
		
		return NULL;
	}
	
	Ref<Value> Value::memory_address(Frame * frame) {
		Value * value;
		
		frame->extract()(value);
		
		return new Integer((intptr_t)value);
	}
	
	Ref<Value> Value::prototype (Frame * frame) {
		Value * value = NULL;
		
		frame->extract()(value);
		
		return value->prototype();
	}
	
	Ref<Value> Value::value (Frame * frame) {
		if (frame->operands()) {
			return frame->operands()->head();
		}
		
		return NULL;
	}

#pragma mark -
#pragma mark Cell

	Cell::Cell (Value * head, Value * tail) : m_head(head), m_tail(tail) {
		
	}

	Cell::~Cell () {

	}
	
	void Cell::mark() {
		if (marked()) return;
		
		Value::mark();
		
		if (m_head) m_head->mark();
		if (m_tail) m_tail->mark();
	}

	Cell * Cell::insert (Value * value) {
		Cell * next = new Cell(value, this->m_tail);
		
		m_tail = next;
		
		return next;
	}

	Cell * Cell::append (Value * value) {
		Cell * end = this;
		
		while (end != NULL) {
			Cell * next = end->m_tail.as<Cell>();
			
			if (next)
				end = next;
			else
				break;
		}
		
		assert(end != NULL);
		
		return end->insert(value);
	}

	unsigned Cell::count () {
		Cell * end = this;
		
		unsigned result = 0;
		
		while (end != NULL) {
			end = end->m_tail.as<Cell>();
			
			result += 1;
		}
		
		return result;
	}

	int Cell::compare (const Value * other) const {
		return derivedCompare(this, other);
	}

	int Cell::compare (const Cell * other) const {
		int result = Value::compare(m_head, other->m_head);
		
		if (result == 0) {
			return Value::compare(m_tail, other->m_tail);
		} else {
			return result;
		}
	}

	void Cell::toCode(StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const {
		if (marks.find(this) != marks.end()) {
			buffer << "(cell{" << this << "})";
		} else {
			marks.insert(this);
			const Cell * tail = m_tail.as<Cell>();
			
			if (tail && Value::equal(m_head, sym("value"))) {
				// Print out values using backtick if possible.
				buffer << '`';
				if (tail->head())
					tail->head()->toCode(buffer, marks, indentation + 1);
				else
					Symbol::nilSymbol()->toCode(buffer, marks, indentation + 1);
			} else {
				// Otherwise, print out as a bracketed list.
				buffer << '(';
				
				const Cell * cur = this;
				
				while (cur) {
					if (cur->m_head)
						cur->m_head->toCode(buffer, marks, indentation + 1);
					else
						buffer << "nil";
					
					if (cur->m_tail) {
						buffer << ' ';
						
						const Cell * next = cur->m_tail.as<Cell>();
						
						if (next) {
							cur = next;
						} else {
							cur->m_tail->toCode(buffer, marks, indentation + 1);
							cur = NULL;
						}
					} else {
						break;
					}
				}
				
				buffer << ')';
			
			}
		}
	}

	Ref<Value> Cell::evaluate (Frame * frame) {
		return frame->call(NULL, this);
	}
	
	Ref<Value> Cell::_new (Frame * frame) {
		Value * self, * head = NULL, * tail = NULL;
		
		frame->extract()[self][head][tail];
		
		return new Cell(head, tail);
	}
	
	Ref<Value> Cell::head (Frame * frame) {
		Cell * cell;
		
		frame->extract()[cell];
		
		if (!cell) {
			throw Exception("Invalid Argument", frame);
		}
		
		return cell->head();
	}
	
	Ref<Value> Cell::tail (Frame * frame) {
		Cell * cell;
		
		frame->extract()[cell];
		
		if (!cell) {
			throw Exception("Invalid Argument", frame);
		}
		
		return cell->tail();
	}
	
	Ref<Value> Cell::list (Frame * frame)
	{
		return frame->operands();
	}
	
	Ref<Value> Cell::prototype () {
		return globalPrototype();
	}
	
	Ref<Value> Cell::each (Frame * frame) {
		Cell * cell;
		Value * callback;
		
		frame->extract()[cell][callback];
		
		while (cell != NULL) {
			Cell * message = Cell::create()(callback)(cell);
			message->evaluate(frame);
			
			cell = cell->tailAs<Cell>();
		}
		
		return NULL;
	}
	
	Ref<Value> Cell::globalPrototype () {
		static Ref<Table> g_prototype;
		
		if (!g_prototype) {
			g_prototype = new Table;
			
			g_prototype->update(sym("new"), KAI_BUILTIN_FUNCTION(Cell::_new));
			g_prototype->update(sym("each"), KAI_BUILTIN_FUNCTION(Cell::each));
			g_prototype->update(sym("head"), KAI_BUILTIN_FUNCTION(Cell::head));
			g_prototype->update(sym("tail"), KAI_BUILTIN_FUNCTION(Cell::tail));
		}
		
		return g_prototype;
	}
	
	void Cell::import (Table * context) {
		context->update(sym("Cell"), globalPrototype());
		context->update(sym("list"), KAI_BUILTIN_FUNCTION(Cell::list));
	}

#pragma mark -
#pragma mark String

	String::String (const StringT & value, bool unescape) : m_value(value) {
		if (unescape) {
			m_value = Parser::unescapeString(m_value);
		}
	}

	String::~String () {

	}

	int String::compare (const Value * other) const {
		return derivedCompare(this, other);
	}

	int String::compare (const String * other) const {
		return m_value.compare(other->m_value);
	}

	void String::toCode(StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const {
		buffer << Parser::escapeString(m_value);
	}
	
	Ref<Value> String::join(Frame * frame)
	{
		StringStreamT buffer;
		String * self;
		
		Cell * next = frame->extract()(self);
		
		buffer << self->value();
		
		while (next != NULL) {
			String * head = next->headAs<String>();
			
			if (head) {
				buffer << head->value();
			} else {
				buffer << Value::toString(next->head());
			}
			
			next = next->tailAs<Cell>();
		}
		
		return new String(buffer.str());
	}
	
	Ref<Value> String::size (Frame * frame)
	{
		String * self;
		frame->extract()(self);
		
		return new Integer(self->value().size());
	}
	
	Ref<Value> String::at (Frame * frame)
	{
		String * self;
		Integer * offset;
		frame->extract()(self)(offset);
		
		// Bounds checking
		if (offset->value() < 0 || offset->value() > self->value().size()) {
			throw Exception("Invalid Offset!", offset, frame);
		}
		
		StringT character(1, self->value()[offset->value()]);
		
		return new String(character);
	}
	
	Ref<Value> String::length (Frame * frame)
	{
		String * self;
		
		frame->extract()(self);
		
		std::size_t result = utf8::distance(self->m_value.begin(), self->m_value.end());
		
		return new Integer(result);
	}
	
	Ref<Value> String::each (Frame * frame)
	{
		String * self;
		Value * function;
		
		frame->extract()(self)(function);
				
		StringT::iterator current = self->m_value.begin();
		StringT::iterator previous = current;
		
		Cell * last = NULL, * first = NULL;
		
		while (current != self->m_value.end()) {
			uint32_t value = utf8::next(current, self->m_value.end());
			
			// Create a buffer to contain the single character:			
			String * character = new String(StringT(previous, current));
			Cell * message = Cell::create(function)(character);
			Ref<Value> result = frame->call(message);

			last = Cell::append(last, result, first);
			
			previous = current;
		}
		
		return first;
	}
	
	Ref<Value> String::prototype()
	{
		return globalPrototype();
	}
	
	Ref<Value> String::globalPrototype()
	{
		static Ref<Table> g_prototype;
		
		if (!g_prototype) {
			g_prototype = new Table;
			
			g_prototype->update(sym("+"), KAI_BUILTIN_FUNCTION(String::join));
			g_prototype->update(sym("size"), KAI_BUILTIN_FUNCTION(String::size));
			g_prototype->update(sym("at"), KAI_BUILTIN_FUNCTION(String::at));
			
			g_prototype->update(sym("each"), KAI_BUILTIN_FUNCTION(String::each));
			g_prototype->update(sym("length"), KAI_BUILTIN_FUNCTION(String::length));
			
		}
		
		return g_prototype;
	}
	
	void String::import(Table * context)
	{
		context->update(sym("String"), globalPrototype());
	}

#pragma mark -
#pragma mark Symbol

	int Symbol::calculateHash (const StringT & string) {
		int sum = 0;
		
		StringT::const_iterator c = string.begin();
		const StringT::const_iterator end = string.end();
		
		while (c != end) {
			sum += *c;
			
			++c;
		}
		
		return sum;
	}

	Symbol::Symbol (const StringT & value) : m_value(value), m_hash(calculateHash(value)) {
	}

	Symbol::~Symbol () {

	}

	int Symbol::compare (const Value * other) const {
		return derivedCompare(this, other);
	}

	int Symbol::compare (const Symbol * other) const {
		int result = clampComparison(m_hash - other->m_hash);
		
		if (result == 0) {
			return m_value.compare(other->m_value);
		}		
		
		return result;
	}

	void Symbol::toCode(StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const {
		buffer << m_value;
	}

	Ref<Value> Symbol::evaluate (Frame * frame) {
		if (m_value[0] != ':') {
			return frame->lookup(this);
		} else {
			return this;
		}
	}

	Symbol * Symbol::nilSymbol () {
		return sym("nil");
	}
	
	Symbol * Symbol::falseSymbol () {
		return NULL;
	}
	
	Symbol * Symbol::trueSymbol () {
		return sym("true");
	}
	
	Ref<Value> Symbol::hash (Frame * frame) {
		Symbol * self;
		
		frame->extract()(self);
		
		return new Integer(self->m_hash);
	}
	
	Ref<Value> Symbol::assign (Frame * frame) {
		Symbol * self;
		Value * value;
		
		frame->extract()(self)[value];
		
		Table * scope = frame->scope().as<Table>();
		
		if (scope) {
			scope->update(self, value);
		} else {
			throw Exception("Invalid Scope", frame->scope(), frame);
		}
		
		return value;
	}
	
	Ref<Value> Symbol::prototype () {
		return globalPrototype();
	}
	
	Ref<Value> Symbol::globalPrototype () {
		static Ref<Table> g_prototype;
		
		if (!g_prototype) {
			g_prototype = new Table;
			
			g_prototype->update(sym("="), KAI_BUILTIN_FUNCTION(Symbol::assign));
			g_prototype->update(sym("hash"), KAI_BUILTIN_FUNCTION(Symbol::hash));
		}
		
		return g_prototype;
	}
	
	void Symbol::import (Table * context) {
		context->update(sym("Symbol"), globalPrototype());
	}

#pragma mark -
#pragma mark Integer

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
	
	Ref<Value> Integer::globalPrototype () {
		static Ref<Table> g_prototype;
		
		if (!g_prototype) {
			g_prototype = new Table;
			
			g_prototype->update(sym("+"), KAI_BUILTIN_FUNCTION(Integer::sum));
			g_prototype->update(sym("-"), KAI_BUILTIN_FUNCTION(Integer::subtract));
			g_prototype->update(sym("*"), KAI_BUILTIN_FUNCTION(Integer::product));
			g_prototype->update(sym("%"), KAI_BUILTIN_FUNCTION(Integer::modulus));
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
		return clampComparison(other->m_value - m_value);
	}

	void Integer::toCode(StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const {
		buffer << m_value;
	}
	
	void Integer::import (Table * context) {
		context->update(sym("Integer"), globalPrototype());
	}

#pragma mark -
#pragma mark Table

	Table::Table (int size) : m_prototype(NULL) {
		assert(size >= 1);
		
		m_bins.resize(size);
	}

	Table::~Table () {
		for (Bin * bin : m_bins) {
			while (bin) {
				Bin * next = bin->next;
				
				delete bin;
				
				bin = next;
			}
		}
	}

	void Table::mark() {
		if (marked()) return;
		
		Value::mark();
		
		for (Bin * bin : m_bins) {
			while (bin) {
				bin->key->mark();
				bin->value->mark();
				
				bin = bin->next;
			}
		}
	}
	
	Table::Bin * Table::find (Symbol * key) {
		assert(key != NULL);
				
		Bin * bin = m_bins[key->hash() % m_bins.size()];
		
		while (bin != NULL) {
			if (key->compare(bin->key) == 0) {
				return bin;
			}
			
			bin = bin->next;
		}

		return NULL;
	}

	Ref<Value> Table::update (Symbol * key, Value * value) {		
		KAI_ENSURE(key != NULL);
			
		unsigned index = key->hash() % m_bins.size();
		
		Bin * bin = m_bins[index];
		
		if (bin) {
			while (1) {
				KAI_ENSURE(bin);
				KAI_ENSURE(bin->key);
				
				if (key->compare(bin->key) == 0) {
					Ref<Value> old = bin->value;
					
					bin->value = value;
					
					return old;
				}
				
				if (bin->next == NULL)
					break;
				
				bin = bin->next;
			}
		}
		
		Bin * next = new Bin;
		
		next->next = NULL;
		next->key = key;
		next->value = value;
		
		if (bin) {
			bin->next = next;
		} else {
			m_bins[index] = next;
		}
		
		return NULL;
	}

	Ref<Value> Table::remove (Symbol * key) {
		assert(key != NULL);
		
		// The place where we should write the pointer to the next value
		Bin ** next = &m_bins[key->hash() % m_bins.size()];
		Bin * bin = *next;
		
		while (bin != NULL) {
			if (key->compare(bin->key) == 0) {
				// If key is the same, remove the bin, but skipping over it.
				*next = bin->next;
			}
			
			next = &bin->next;
			bin = bin->next;
		}

		return NULL;
	}

	int Table::compare (const Value * other) const {
		return derivedCompare(this, other);
	}

	int Table::compare (const Table * other) const {
		throw InvalidComparison();
	}

	void Table::toCode(StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const {
		if (marks.find(this) != marks.end()) {
			buffer << "(table@" << this << " ...)" << std::endl;
		} else {
			marks.insert(this);
			
			buffer << "(table@" << this;
			
			for (unsigned i = 0; i < m_bins.size(); i += 1) {
				Bin * bin = m_bins[i];
				
				while (bin != NULL) {
					buffer << std::endl << StringT(indentation, '\t') << "`";
					bin->key->toCode(buffer, marks, indentation + 1);
					buffer << " ";
					bin->value->toCode(buffer, marks, indentation + 1);
					
					bin = bin->next;
				}
			}
			
			buffer << ")";
		}
	}

	Ref<Value> Table::lookup (Symbol * key) {
		Bin * bin = find(key);
		
		if (bin) {
			return bin->value;
		}
		
		if (m_prototype)
			return m_prototype->lookup(key);

		return NULL;
	}
	
	void Table::setPrototype (Value * prototype) {
		m_prototype = prototype;
	}
	
	Ref<Value> Table::prototype () {
		return m_prototype;
	}
	
#pragma mark Builtins
	
	Ref<Value> Table::table (Frame * frame) {
		Table * table = new Table;
		Cell * args = frame->unwrap();

		// Bump self
		args = args->tailAs<Cell>();
		
		while (args) {
			Symbol * key = NULL;
			Value * value = NULL;
			
			args = args->extract(frame)[key][value];
			
			if (key == NULL) {
				throw Exception("Invalid Key", frame);
			}
			
			table->update(key, value);
		}
		
		return table;
	}

	Ref<Value> Table::update (Frame * frame) {
		Table * table = NULL;
		Symbol * key = NULL;
		Value * value = NULL;
		
		frame->extract()(table)(key)(value);
		
		if (value == NULL)
			return table->update(key, value);
		else
			return table->remove(key);
	}
	
	Ref<Value> Table::set (Frame * frame) {
		Table * table = NULL;
		Symbol * key = NULL;
		Value * value = NULL;
		
		frame->extract()(table)(key)[value];
		
		if (value)
			table->update(key, value);
		else
			table->remove(key);
		
		return value;
	}
	
	Ref<Value> Table::lookup (Frame * frame) {
		Table * table = NULL;
		Symbol * key = NULL;
		
		frame->extract()[table][key];
		
		if (table == NULL) {
			throw Exception("Invalid Target", frame);
		}
		
		if (key == NULL) {
			throw Exception("Invalid Key", frame);
		}
		
		return table->lookup(key);
	}
	
	Ref<Value> Table::each (Frame * frame) {
		Table * table;
		Value * callback;
		
		frame->extract()[table][callback];
		
		for (unsigned i = 0; i < table->m_bins.size(); i += 1) {
			Bin * cur = table->m_bins[i];
			
			while (cur != NULL) {
				Cell * message = Cell::create()(callback)(cur->key)(cur->value);
				message->evaluate(frame);
				
				cur = cur->next;
			}
		}
		
		return NULL;
	}
	
	Ref<Value> Table::setPrototype (Frame * frame) {
		Table * table = NULL;
		Value * prototype = NULL;
		
		frame->extract()(table)(prototype);
		
		table->setPrototype(prototype);
		
		return prototype;
	}
	
	Ref<Value> Table::globalPrototype () {
		static Ref<Table> g_prototype;
		
		if (!g_prototype) {
			g_prototype = new Table;
			g_prototype->setPrototype(Value::globalPrototype());
			
			g_prototype->update(sym("new"), KAI_BUILTIN_FUNCTION(Table::table));
			//g_prototype->update(sym("update"), KAI_BUILTIN_FUNCTION(Table::update));
			g_prototype->update(sym("set"), KAI_BUILTIN_FUNCTION(Table::set));
			g_prototype->update(sym("get"), KAI_BUILTIN_FUNCTION(Table::lookup));
			g_prototype->update(sym("each"), KAI_BUILTIN_FUNCTION(Table::each));
			g_prototype->update(sym("prototype="), KAI_BUILTIN_FUNCTION(Table::setPrototype));
		}
		
		return g_prototype;
	}
	
	void Table::import (Table * context) {
		context->update(sym("Table"), Table::globalPrototype());
	}	
}
