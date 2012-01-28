//
//  Cell.h
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 28/12/11.
//  Copyright (c) 2011 Orion Transfer Ltd. All rights reserved.
//

#ifndef _KAI_CELL_H
#define _KAI_CELL_H

#include "Object.h"
#include "Table.h"
#include "Exception.h"
#include "Frame.h"

namespace Kai {
	
	class ArgumentExtractor;
	
	class Cell : public Object {
	protected:
		Object * _head;
		Object * _tail;
		
	public:
		static const char * const NAME;
		
		Cell (Object * head = NULL, Object * tail = NULL);
		virtual ~Cell ();
		
		virtual Ref<Symbol> identity(Frame * frame) const;
		
		virtual void mark(Memory::Traversal *) const;
		
		Ref<Object> head() { return _head; }
		const Ref<Object> head() const { return _head; }
		
		Ref<Object> tail() { return _tail; }
		const Ref<Object> tail() const { return _tail; }
		
		Cell * insert(Object * object);
		Cell * append(Object * object);
		
		template <typename FirstT>
		static Cell * append(Frame * frame, Cell * list, Object * item, FirstT & first) {
			if (list) {
				return list->append(item);
			} else {
				return (first = new(frame) Cell(item));
			}
		}
		
		std::size_t count();
		
		virtual ComparisonResult compare(const Object * other) const;
		ComparisonResult compare(const Cell * other) const;
		
		virtual void to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const;
		
		virtual Ref<Object> evaluate(Frame * frame);
		
		ArgumentExtractor extract(Frame * frame);
		
		class ListBuilder {
		protected:
			Cell * _start;
			Cell * _current;
			Frame * _frame;
			
		public:
			ListBuilder (Frame * frame) : _start(NULL), _current(NULL), _frame(frame) {
				
			}
			
			ListBuilder & operator() (Object * object) {
				_current = Cell::append(_frame, _current, object, _start);
				
				return *this;
			}
			
			operator Cell* () {
				return _start;
			}
			
			operator Ref<Object> () {
				return _start;
			}
		};
		
		static inline ListBuilder create(Frame * frame) {
			return ListBuilder(frame);
		}
		
		//% (prepend cell value) -> (cell value)
		static Ref<Object> new_(Frame * frame);
		static Ref<Object> head(Frame * frame);
		static Ref<Object> tail(Frame * frame);
		
		static Ref<Object> each(Frame * frame);
		
		static void import(Frame * frame);
	};
	
#pragma mark -
	
	class ArgumentExtractor {
		Frame * _frame;
		Cell * _current;
		
	public:
		inline ArgumentExtractor(Frame * frame, Cell * current) : _frame(frame), _current(current) {
			
		}
		
		// Optional argument - e.g. NULL.
		template <typename AnyT>
		ArgumentExtractor operator[](AnyT *& t) {
			// We have an empty argument list.
			if (_current == NULL) {
				t = NULL;
				return *this;
				//throw Exception("Argument Error", _frame);
			}
			
			t = _current->head().as<AnyT>();
			
			return ArgumentExtractor(_frame, _current->tail().as<Cell>());
		}
		
		// Ensures argument is non-NULL
		template <typename AnyT>
		ArgumentExtractor operator()(AnyT *& t, const StringT & name, bool required = true) {
			if (_current == NULL) {
				throw ArgumentError(name, AnyT::NAME, NULL, _frame);
			}
			
			t = _current->head().as<AnyT>();
			
			// If t is true or wasn't required, everything is okay.
			if (t || !required) {
				return ArgumentExtractor(_frame, _current->tail().as<Cell>());
			}
			
			// Else, something went wrong:
			if (_current->head()) {
				throw ArgumentError(name, AnyT::NAME, _current->head(), _frame);
			} else {
				throw ArgumentError(name, AnyT::NAME, NULL, _frame);
			}
		}
		
		// Ensures argument is non-NULL
		template <typename AnyT>
		ArgumentExtractor operator()(AnyT *& t) {
			return (*this)(t, "*unspecified*", true);
		}
		
		inline operator Cell *() {
			return _current;
		}
	};
	
}

#endif
