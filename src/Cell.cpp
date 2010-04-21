/*
 *  Cell.c
 *  This file is part of the "Kai" project, and is licensed under the GNU GPLv3.
 *
 *  Created by Samuel Williams on 10/04/10.
 *  Copyright 2010 Samuel Williams. All rights reserved.
 *
 */

#include "Cell.h"
#include <assert.h>
#include <gc/gc_cpp.h>
#include <iostream>

#include "Frame.h"
#include "Exception.h"

namespace Kai {

	#pragma mark -
	#pragma mark Value

	template <typename ThisT>
	inline static int derivedCompare (ThisT * lhs, Value * rhs) {
		ThisT * other = dynamic_cast<ThisT *>(other);

		if (other) {
			return lhs->compare(other);
		} else {
			throw InvalidComparison();
		}
	}

	inline int clampComparison (int result) {
		if (result < 0) {
			return -1;
		} else if (result > 0) {
			return 1;
		} else {
			return 0;
		}
	}

	Value::~Value () {

	}

	int Value::compare (Value * other) {
		throw InvalidComparison();
	}

	void Value::debug () {
		std::cerr << toString(this) << std::endl;
	}

	Value * Value::invoke (Frame * frame) {
		throw Exception("Invalid Invocation", frame);
	}

	Value * Value::evaluate (Frame * frame) {
		return this;
	}

	StringT Value::toString (Value * value) {
		if (value) {
			StringStreamT buffer;
			value->toCode(buffer);
			return buffer.str();
		} else {
			return "nil";
		}
	}

	#pragma mark -
	#pragma mark Cell

	Cell::Cell (Value * head, Value * tail) : m_head(head), m_tail(tail) {
		
	}

	Cell::~Cell () {

	}

	Cell * Cell::insert (Value * value) {
		Cell * next = new Cell(value, this->m_tail);
		
		m_tail = next;
		
		return next;
	}

	Cell * Cell::append (Value * value) {
		Cell * end = this;
		
		while (end != NULL) {
			Cell * next = dynamic_cast<Cell*>(end->m_tail);
			
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
			end = dynamic_cast<Cell*>(end->m_tail);
			
			result += 1;
		}
		
		return result;
	}

	int Cell::compare (Value * other) {
		return derivedCompare(this, other);
	}

	int Cell::compare (Cell * other) {
		int result = Value::compare(m_head, other->m_head);
		
		if (result == 0) {
			return Value::compare(m_tail, other->m_tail);
		} else {
			return result;
		}
	}

	void Cell::toCode (StringStreamT & buffer) {
		buffer << '(';
		
		Cell * cur = this;
		
		while (cur) {
			if (cur->m_head)
				cur->m_head->toCode(buffer);
			else
				buffer << "nil";
			
			if (cur->m_tail) {
				buffer << ' ';
				
				Cell * next = dynamic_cast<Cell*>(cur->m_tail);
				
				if (next) {
					cur = next;
				} else {
					cur->m_tail->toCode(buffer);
				}
			} else {
				break;
			}
		}
		
		buffer << ')';
	}

	Value * Cell::evaluate (Frame * frame) {
		return frame->call(this);
	}

	#pragma mark -
	#pragma mark String

	String::String (const StringT & value) : m_value(value) {
		
	}

	String::~String () {

	}

	int String::compare (Value * other) {
		return derivedCompare(this, other);
	}

	int String::compare (String * other) {
		return m_value.compare(other->m_value);
	}

	void String::toCode (StringStreamT & buffer) {
		buffer << '"';
		
		buffer << m_value;
		
		buffer << '"';
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

	int Symbol::compare (Value * other) {
		return derivedCompare(this, other);
	}

	int Symbol::compare (Symbol * other) {
		return clampComparison(m_hash - other->m_hash);
	}

	void Symbol::toCode (StringStreamT & buffer) {
		buffer << m_value;
	}

	Value * Symbol::evaluate (Frame * frame) {
		if (m_value[0] != ':') {
			return frame->call(this);
		} else {
			return this;
		}
	}

	#pragma mark -
	#pragma mark Integer

	Integer::Integer (int value) : m_value(value) {
	}

	Integer::~Integer () {
	}

	int Integer::compare (Value * other) {
		return derivedCompare(this, other);
	}

	int Integer::compare (Integer * other) {
		return clampComparison(other->m_value - m_value);
	}

	void Integer::toCode (StringStreamT & buffer) {
		buffer << m_value;
	}

	#pragma mark -
	#pragma mark Table

	Table::Table (int size) : m_size(size) {
		assert(size >= 1);
		
		m_bins = (Bin**)GC_malloc(sizeof(Bin*) * size);
	}

	Table::~Table () {

	}

	Table::Table (int size, bool allocate) {
		assert(size >= 1);
		
		m_bins = (Bin**)(this + sizeof(Table));
	}

	Table * Table::allocate (int size) {	
		Table * table = (Table*)GC_malloc(sizeof(Table) + sizeof(Bin*) * size);
		
		return new(table) Table(size, false);
	}

	Table::Bin * Table::find (Symbol * key) {
		assert(key != NULL);
		
		Bin * bin = m_bins[key->hash() % m_size];
		
		while (bin != NULL) {
			if (key->compare(bin->key) == 0) {
				return bin;
			}
			
			bin = bin->next;
		}

		return NULL;
	}

	Value * Table::update (Symbol * key, Value * value) {
		assert(key != NULL);
		assert(value != NULL);
			
		unsigned index = key->hash() % m_size;
		
		Bin * bin = m_bins[index];
		
		if (bin) {
			while (1) {
				if (key->compare(bin->key) == 0) {
					Value * old = bin->value;
					
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

	Value * Table::remove (Symbol * key) {
		assert(key != NULL);
		
		// The place where we should write the pointer to the next value
		Bin ** next = &m_bins[key->hash() % m_size];
		Bin * bin = *next;
		
		while (bin != NULL) {
			if (key->compare(bin->key) == 0) {
				*next = bin->next;
			}
			
			next = &bin->next;
			bin = bin->next;
		}

		return NULL;
	}

	int Table::compare (Value * other) {
		return derivedCompare(this, other);
	}

	int Table::compare (Table * other) {
		throw InvalidComparison();
	}

	void Table::toCode (StringStreamT & buffer) {
		buffer << "{";
		
		bool first = true;
		
		for (unsigned i = 0; i < m_size; i += 1) {
			Bin * bin = m_bins[i];
			
			while (bin != NULL) {
				if (!first) {
					buffer << ' ';
				} else {
					first = false;
				}
			
				bin->key->toCode(buffer);
				buffer << ": ";
				bin->value->toCode(buffer);
				
				bin = bin->next;
			}
		}
		
		buffer << "}";
	}

	Value * Table::lookup (Symbol * key) {
		Bin * bin = find(key);
		
		if (bin) {
			return bin->value;
		}
		
		if (m_prototype)
			return m_prototype->lookup(key);

		return NULL;
	}

	Value * Table::invoke (Frame * frame) {
		Symbol * key = frame->function();

		if (!key) {
			throw Exception("Null Key", frame);
		}
		
		Value * value = this->lookup(key);
		
		if (value) {
			return value->evaluate(frame);
		}
		
		throw Exception("Invalid Invocation", frame);
	}
}
