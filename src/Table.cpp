//
//  Table.cpp
//  Kai
//
//  Created by Samuel Williams on 16/12/11.
//  Copyright (c) 2011 Orion Transfer Ltd. All rights reserved.
//

#include "Table.h"
#include "Cell.h"
#include "Frame.h"
#include "Symbol.h"
#include "Function.h"

namespace Kai {
	
	Table::Table(int size) : _prototype(NULL) {
		KAI_ENSURE(size >= 1);
		
		_bins.resize(size);
	}
	
	Table::~Table() {
		for (Bin * bin : _bins) {
			while (bin) {
				Bin * next = bin->next;
				
				delete bin;
				
				bin = next;
			}
		}
	}
	
	Ref<Symbol> Table::identity(Frame * frame) const {
		return frame->sym("Table");
	}
	
	void Table::mark(Memory::Traversal * traversal) const {
		Object::mark(traversal);
		
		traversal->traverse(_prototype);
		
		for (Bin * bin : _bins) {
			while (bin) {
				traversal->traverse(bin->key);
				traversal->traverse(bin->value);
				
				bin = bin->next;
			}
		}
	}
	
	Table::Bin * Table::find(Symbol * key) {
		KAI_ENSURE(key != NULL);
		
		Bin * bin = _bins[key->hash() % _bins.size()];
		
		while (bin != NULL) {
			if (key->compare(bin->key) == 0) {
				return bin;
			}
			
			bin = bin->next;
		}
		
		return NULL;
	}
	
	Ref<Object> Table::update(Symbol * key, Object * value) {		
		KAI_ENSURE(key != NULL);
		
		//Memory::PageAllocation * allocator = this->allocator();
		//KAI_ENSURE(allocator->includes(key));
		//KAI_ENSURE(allocator->includes(value));		
		
		unsigned index = key->hash() % _bins.size();
		
		Bin * bin = _bins[index];
		
		if (bin) {
			while (1) {
				KAI_ENSURE(bin);
				KAI_ENSURE(bin->key);
				
				if (key->compare(bin->key) == 0) {
					Ref<Object> old = bin->value;
					
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
			_bins[index] = next;
		}
		
		return NULL;
	}
	
	Ref<Object> Table::remove(Symbol * key) {
		KAI_ENSURE(key != NULL);
		
		// The place where we should write the pointer to the next value
		Bin ** next = &_bins[key->hash() % _bins.size()];
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
	
	ComparisonResult Table::compare(const Object * other) const {
		return derived_compare(this, other);
	}
	
	ComparisonResult Table::compare(const Table * other) const {
		throw InvalidComparison();
	}
	
	void Table::to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const {
		if (marks.find(this) != marks.end()) {
			buffer << "(table@" << this << " ...)" << std::endl;
		} else {
			marks.insert(this);
			
			buffer << "(table@" << this;
			
			for (unsigned i = 0; i < _bins.size(); i += 1) {
				Bin * bin = _bins[i];
				
				while (bin != NULL) {
					buffer << std::endl << StringT(indentation, '\t') << "`";
					bin->key->to_code(frame, buffer, marks, indentation + 1);
					buffer << " ";
					bin->value->to_code(frame, buffer, marks, indentation + 1);
					
					bin = bin->next;
				}
			}
			
			buffer << ")";
		}
	}
	
	Ref<Object> Table::lookup(Frame * frame, Symbol * key) {
		Bin * bin = find(key);
		
		if (bin) {
			return bin->value;
		}
		
		if (_prototype)
			return _prototype->lookup(frame, key);
		
		return NULL;
	}
	
	void Table::set_prototype(Object * prototype) {
		_prototype = prototype;
	}
	
	Ref<Object> Table::prototype(Frame * frame) const {
		return _prototype;
	}
	
#pragma mark Builtins
	
	Ref<Object> Table::new_(Frame * frame) {
		Table * table = new(frame) Table;
		Cell * args = frame->unwrap();
		
		// Bump self
		args = args->tail().as<Cell>();
		
		while (args) {
			Symbol * key = NULL;
			Object * value = NULL;
			
			args = args->extract(frame)[key][value];
			
			if (key == NULL) {
				throw Exception("Invalid Key", frame);
			}
			
			table->update(key, value);
		}
		
		return table;
	}
	
	Ref<Object> Table::update(Frame * frame) {
		Table * table = NULL;
		Symbol * key = NULL;
		Object * value = NULL;
		
		frame->extract()(table)(key)(value);
		
		if (value == NULL)
			return table->update(key, value);
		else
			return table->remove(key);
	}
	
	Ref<Object> Table::set(Frame * frame) {
		Table * table = NULL;
		Symbol * key = NULL;
		Object * value = NULL;
		
		frame->extract()(table)(key)[value];
		
		if (value)
			table->update(key, value);
		else
			table->remove(key);
		
		return value;
	}
	
	Ref<Object> Table::lookup(Frame * frame) {
		Table * table = NULL;
		Symbol * key = NULL;
		
		frame->extract()(table, "self")(key, "key");
				
		return table->lookup(frame, key);
	}
	
	Ref<Object> Table::each(Frame * frame) {
		Table * table;
		Object * callback;
		
		frame->extract()(table, "self")(callback, "callback");
		callback = callback->as_value(frame);
		
		std::cerr << "Callback: " << Object::to_string(frame, callback) << std::endl;
		
		for (unsigned i = 0; i < table->_bins.size(); i += 1) {
			Bin * cur = table->_bins[i];
			
			while (cur != NULL) {
				Cell * message = Cell::create(frame)(callback)(cur->key)(cur->value);
				frame->call(message);
				
				cur = cur->next;
			}
		}
		
		return NULL;
	}
	
	Ref<Object> Table::set_prototype(Frame * frame) {
		Table * table = NULL;
		Object * prototype = NULL;
		
		frame->extract()(table, "self")(prototype, "prototype");
		
		table->set_prototype(prototype);
		
		return prototype;
	}
	
	void Table::import (Frame * frame) {
		Ref<Table> prototype = new(frame) Table;
		
		// Ensure that the prototype chain is complete.
		prototype->set_prototype(frame->lookup(frame->sym("Object")));
		
		prototype->update(frame->sym("new"), KAI_BUILTIN_FUNCTION(Table::new_));
		prototype->update(frame->sym("set"), KAI_BUILTIN_FUNCTION(Table::set));
		prototype->update(frame->sym("get"), KAI_BUILTIN_FUNCTION(Table::lookup));
		prototype->update(frame->sym("each"), KAI_BUILTIN_FUNCTION(Table::each));
		prototype->update(frame->sym("set-prototype"), KAI_BUILTIN_FUNCTION(Table::set_prototype));
		
		frame->update(frame->sym("Table"), prototype);
	}
}
