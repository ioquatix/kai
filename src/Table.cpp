//
//  Table.cpp
//  Kai
//
//  Created by Samuel Williams on 16/12/11.
//  Copyright (c) 2011 Orion Transfer Ltd. All rights reserved.
//

#include "Table.h"
#include "Frame.h"
#include "Function.h"

namespace Kai {
	
	Table::Table (int size) : m_prototype(NULL) {
		KAI_ENSURE(size >= 1);
		
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
		KAI_ENSURE(key != NULL);
		
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
		KAI_ENSURE(key != NULL);
		
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
