//
//  Cell.cpp
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 28/12/11.
//  Copyright (c) 2011 Orion Transfer Ltd. All rights reserved.
//

#include "Cell.h"
#include "Frame.h"
#include "Function.h"
#include "Symbol.h"

namespace Kai {
	
	const char * const Cell::NAME = "Cell";
	
	Cell::Cell(Object * head, Object * tail) : _head(head), _tail(tail) {
		
	}
	
	Cell::~Cell() {
		
	}
	
	Ref<Symbol> Cell::identity(Frame * frame) const {
		return frame->sym("Cell");
	}
	
	void Cell::mark(Memory::Traversal * traversal) const {
		traversal->traverse(_head);
		traversal->traverse(_tail);
	}
	
	Cell * Cell::insert(Object * object) {
		Cell * next = new(this) Cell(object, this->_tail);
		
		_tail = next;
		
		return next;
	}
	
	Cell * Cell::append(Object * object) {
		Cell * end = this;
		
		while (end != NULL) {
			Cell * next = ptr(end->_tail).as<Cell>();
			
			if (next)
				end = next;
			else
				break;
		}
		
		KAI_ENSURE(end != NULL);
		
		return end->insert(object);
	}
	
	std::size_t Cell::count() {
		Cell * end = this;
		
		unsigned result = 0;
		
		while (end != NULL) {
			end = ptr(end->_tail).as<Cell>();
			
			result += 1;
		}
		
		return result;
	}
	
	ComparisonResult Cell::compare(const Object * other) const {
		return derived_compare(this, other);
	}
	
	ComparisonResult Cell::compare(const Cell * other) const {
		ComparisonResult result = Object::compare(_head, other->_head);
		
		if (result == 0) {
			return Object::compare(_tail, other->_tail);
		} else {
			return result;
		}
	}
	
	void Cell::to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const {
		if (marks.find(this) != marks.end()) {
			buffer << "(" << Object::to_string(frame, identity(frame)) << "@" << this << ")";
		} else {
			marks.insert(this);
			const Cell * tail = ptr(_tail).as<Cell>();
			
			if (tail && Object::equal(_head, frame->sym("value"))) {
				// Print out objects using backtick if possible.
				buffer << '`';
				if (tail->head())
					tail->head()->to_code(frame, buffer, marks, indentation + 1);
				else
					Symbol::nil_symbol(frame)->to_code(frame, buffer, marks, indentation + 1);
			} else {
				// Otherwise, print out as a bracketed list.
				buffer << '(';
				
				const Cell * cur = this;
				
				while (cur) {
					if (cur->_head)
						cur->_head->to_code(frame, buffer, marks, indentation + 1);
					else
						buffer << "nil";
					
					if (cur->_tail) {
						buffer << ' ';
						
						const Cell * next = ptr(cur->_tail).as<Cell>();
						
						if (next) {
							cur = next;
						} else {
							cur->_tail->to_code(frame, buffer, marks, indentation + 1);
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
	
	ArgumentExtractor Cell::extract(Frame * frame) {
		return ArgumentExtractor(frame, this);
	}
	
	Ref<Object> Cell::evaluate(Frame * frame) {
		return frame->call(NULL, this);
	}
	
	Ref<Object> Cell::new_(Frame * frame) {
		Object * self, * head = NULL, * tail = NULL;
		
		frame->extract()(self, "class")[head][tail];
		
		return new(frame) Cell(head, tail);
	}
	
	Ref<Object> Cell::head(Frame * frame) {
		Cell * cell;
		
		frame->extract()(cell, "self");
		
		return cell->head();
	}
	
	Ref<Object> Cell::tail(Frame * frame) {
		Cell * cell;
		
		frame->extract()(cell, "self");
		
		return cell->tail();
	}
	
	Ref<Object> Cell::each(Frame * frame) {
		Cell * cell;
		Object * callback;
		
		frame->extract()[cell](callback, "callback");
		
		while (cell != NULL) {
			Cell * message = Cell::create(frame)(callback)(cell);
			message->evaluate(frame);
			
			cell = cell->tail().as<Cell>();
		}
		
		return NULL;
	}
		
	void Cell::import(Frame * frame) {
		Table * prototype = new(frame) Table;
		
		prototype->update(frame->sym("new"), KAI_BUILTIN_FUNCTION(Cell::new_));
		prototype->update(frame->sym("each"), KAI_BUILTIN_FUNCTION(Cell::each));
		prototype->update(frame->sym("head"), KAI_BUILTIN_FUNCTION(Cell::head));
		prototype->update(frame->sym("tail"), KAI_BUILTIN_FUNCTION(Cell::tail));
		
		frame->update(frame->sym("Cell"), prototype);
	}
	
}
