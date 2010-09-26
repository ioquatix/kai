/*
 *  Value.h
 *  This file is part of the "Kai" project, and is licensed under the GNU GPLv3.
 *
 *  Created by Samuel Williams on 10/04/10.
 *  Copyright 2010 Samuel Williams. All rights reserved.
 *
 */
 
#ifndef _KVALUE_H
#define _KVALUE_H

#include "Kai.h"
#include "Exception.h"

#include <llvm/Value.h>

#include <set>

namespace Kai {
	// Comparison Results
	typedef std::ptrdiff_t ComparisonResult;
	
	const ComparisonResult COMPARISON_ASCENDING = 1;
	const ComparisonResult COMPARISON_DESCENDING = -1;
	const ComparisonResult COMPARISON_EQUAL = 0;

	class InvalidComparison {};
	class InvalidInvocation {};
	
	typedef std::set<Value*> MarkedT;
	
	class Frame;
	class Table;
	class Symbol;

#pragma mark -
#pragma mark Value
	
	void debug (Value * value);
	
	template <typename ThisT>
	inline static int derivedCompare (ThisT * lhs, Value * rhs) {
		ThisT * other = dynamic_cast<ThisT *>(rhs);

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
	
	class Value : virtual public gc {
		public:
			Value ();
			virtual ~Value ();
		
			virtual int compare (Value * other);
			
			template <typename LeftT, typename RightT>
			static int compare (LeftT * lhs, RightT * rhs) {
				if (lhs == rhs) {
					return 0;
				}
				
				if (lhs == NULL || rhs == NULL) {
					throw InvalidComparison();
				} else {
					return lhs->compare(rhs);
				}
			}
			
			virtual void toCode(StringStreamT & buffer, MarkedT & marks, std::size_t indentation);
						
			inline void toCode(StringStreamT & buffer) {
				MarkedT marks;
				toCode(buffer, marks, 0); 
			}
			
			void debug ();

			// Lookup the given identifier. Defers to prototype.
			virtual Value * lookup (Symbol * identifier);
			
			// A prototype specifies the behaviour of the current value.
			virtual Value * prototype ();
			
			// Evaluate the current value in the given context.
			virtual Value * evaluate (Frame * frame);
			
			// Compile the value to an llvm Value
			virtual llvm::Value * compile (Frame * frame);
			
			// If the value contained is a compiled value, return it unevaluated.
			virtual llvm::Value * compiledValue (Frame * frame);

			static StringT toString (Value * value);
			static bool toBoolean (Value * value);
			static int compare (Value * lhs, Value * rhs);
			
			// Converts the argument to a string value
			static Value * toString (Frame * frame);
			
			// Converts the argument to a boolean symbol
			static Value * toBoolean (Frame * frame);
			
			// Compares the given arguments
			static Value * compare (Frame * frame);
			
			// Compares the given values and returns a true/false value
			static Value * equal (Frame * frame);
			
			// Returns a prototype for the given object.
			static Value * prototype (Frame * frame);
			
			// Returns the arguments unevaluated
			static Value * value (Frame * frame);
			
			// Evaluates arguments one at a time in result of the previous.
			static Value * lookup (Frame * frame);
			
			// Performs a method call with the given function.
			static Value * call (Frame * frame);
			
			// Builtins
			static Value * globalPrototype ();
			static void import (Table * context);
	};

#pragma mark -
#pragma mark Cell

	class Cell : public Value {
		protected:
			Value * m_head;
			Value * m_tail;
			
		public:
			Cell (Value * head = NULL, Value * tail = NULL);
			virtual ~Cell ();
			
			Value * head () { return m_head; }
			
			template <typename ValueT>
			ValueT * headAs () { return dynamic_cast<ValueT*>(m_head); }
			
			Value * tail () { return m_tail; }
			
			template <typename ValueT>
			ValueT * tailAs () { return dynamic_cast<ValueT*>(m_tail); }
			
			Cell * insert (Value * value);
			Cell * append (Value * value);
			
			static Cell * append (Cell * list, Value * item, Cell *& first) {
				if (list) {
					return list->append(item);
				} else {
					return (first = new Cell(item));
				}
			}
			
			unsigned count ();
			
			virtual int compare (Value * other);
			int compare (Cell * other);

			virtual Value * prototype ();
			
			virtual void toCode(StringStreamT & buffer, MarkedT & marks, std::size_t indentation);
			
			virtual Value * evaluate (Frame * frame);
			virtual llvm::Value * compile (Frame * frame);
			
			class ArgumentExtractor {
				Frame * m_frame;
				Cell * m_current;
				
				public:
					inline ArgumentExtractor(Frame * frame, Cell * current) : m_frame(frame), m_current(current) {
					
					}
				
					template <typename AnyT>
					ArgumentExtractor operator[] (AnyT *& t) {
						if (m_current == NULL) {
							throw Exception("Argument Error", m_frame);
						}
						
						t = m_current->headAs<AnyT>();
						
						return ArgumentExtractor(m_frame, m_current->tailAs<Cell>());
					}
					
					// Ensures argument is non-NULL
					template <typename AnyT>
					ArgumentExtractor operator() (AnyT *& t, bool required = true) {
						if (m_current == NULL) {
							throw Exception("Argument Error", m_frame);
						}
						
						t = m_current->headAs<AnyT>();
						
						if (required && !t) {
							throw Exception("Argument Required", m_current, m_frame);
						}
						
						return ArgumentExtractor(m_frame, m_current->tailAs<Cell>());
					}
					
					inline operator Cell* () {
						return m_current;
					}
			};
			
			inline ArgumentExtractor extract(Frame * frame) {
				return ArgumentExtractor(frame, this);
			}
			
			class ListBuilder {
				protected:
					Cell * m_start;
					Cell * m_current;
					
				public:
					ListBuilder () : m_start(NULL), m_current(NULL) {
					
					}
					
					ListBuilder & operator() (Value * value) {
						m_current = Cell::append(m_current, value, m_start);
						
						return *this;
					}
					
					operator Cell* () {
						return m_start;
					}
			};

			static inline ListBuilder create() {
				return ListBuilder();
			}
			
			// Shorthand
			static inline ListBuilder create(Value * value) {
				return ListBuilder()(value);
			}
			
			//% (prepend cell value) -> (cell value)
			static Value * _new (Frame * frame);
			static Value * head (Frame * frame);
			static Value * tail (Frame * frame);
			
			static Value * each (Frame * frame);
						
			static Value * globalPrototype ();
			static void import (Table * context);
	};

#pragma mark -
#pragma mark String

	class String : public Value, virtual public gc_cleanup {
		protected:
			StringT m_value;
			
		public:
			String (const StringT & value, bool unescape = false);
			virtual ~String ();
			
			StringT & value () { return m_value; }
			
			virtual int compare (Value * other);
			int compare (String * other);
			
			virtual void toCode(StringStreamT & buffer, MarkedT & marks, std::size_t indentation);
			
			static Value * join(Frame * frame);
			static Value * size(Frame * frame);
			static Value * at(Frame * frame);
			
			virtual Value * prototype ();
			static Value * globalPrototype ();
			static void import (Table * context);
	};

#pragma mark -
#pragma mark Symbol

	class Symbol : public Value, virtual public gc_cleanup {
		protected:
			const StringT m_value;
			const int m_hash;
			
		public:
			static int calculateHash (const StringT & string);
			
			Symbol (const StringT & string);
			virtual ~Symbol ();
			
			const StringT & value () { return m_value; }
			
			int hash () const { return m_hash; }
			
			virtual int compare (Value * other);
			int compare (Symbol * other);
			
			virtual void toCode(StringStreamT & buffer, MarkedT & marks, std::size_t indentation);
			
			virtual Value * evaluate (Frame * frame);
			virtual llvm::Value * compile (Frame * frame);
			
			static Symbol * nilSymbol ();
			static Symbol * falseSymbol ();
			static Symbol * trueSymbol ();
			
			static Value * hash (Frame * frame);
			
			virtual Value * prototype ();
			static Value * globalPrototype ();
			static void import (Table * context);
	};

#pragma mark -
#pragma mark Integer

	class Integer : public Value {
		protected:
			int m_value;
			
		public:
			Integer (int value);
			virtual ~Integer ();
			
			llvm::Value * compile (Frame * context);
			
			int & value () { return m_value; }
			
			virtual Value * prototype ();
			
			virtual int compare (Value * other);
			int compare (Integer * other);
			
			virtual void toCode(StringStreamT & buffer, MarkedT & marks, std::size_t indentation);
			
			static Value * sum (Frame * frame);
			static Value * product (Frame * frame);
			static Value * modulus (Frame * frame);
			
			static Value * globalPrototype ();
			static void import (Table * context);
	};

#pragma mark -
#pragma mark Table

	class Table : public Value {
		public:
			struct Bin : public gc {
				Symbol * key;
				Value * value;
				Bin * next;
			};
			
		public:
			Table (int size = 16);
			virtual ~Table ();
			
		protected:
			Table (int size, bool allocate);
			
		public:			
			// Inline table allocation
			static Table * allocate (int size = 16);
			
			Bin * find (Symbol * key);
			Value * update (Symbol * key, Value * value);
			Value * remove (Symbol * key);
		
			virtual int compare (Value * other);
			int compare (Table * other);
			
			virtual void toCode(StringStreamT & buffer, MarkedT & marks, std::size_t indentation);
		
			virtual Value * lookup (Symbol * key);
						
			void setPrototype (Value * prototype);
			virtual Value * prototype ();
			
			//% (table [key, value])
			static Value * table (Frame * frame);
			
			//% (update table key value) -> old_value
			static Value * update (Frame * frame);
			
			//% (lookup table key) -> value || nil
			static Value * lookup (Frame * frame);

			// Iteration over key/value pairs
			static Value * each (Frame * frame);
			
			// % (setPrototype table value)
			static Value * setPrototype (Frame * frame);
			
			static Value * globalPrototype ();
			static void import (Table *);
			
		protected:
			Value * m_prototype;
			
			unsigned m_size;
			Bin ** m_bins;
	};

#pragma mark -
#pragma mark Lambda

	class Lambda : public Value {
		protected:
			Frame * m_scope;
			Cell * m_arguments;
			Cell * m_code;
			
		public:
			Lambda (Frame * scope, Cell * arguments, Cell * code);
			virtual ~Lambda ();
			
			virtual Value * evaluate (Frame * frame);
			
			virtual void toCode(StringStreamT & buffer, MarkedT & marks, std::size_t indentation);
			
			static Value * lambda (Frame * frame);
			static void import (Table *);
	};
	
#pragma mark -
#pragma mark Logic

	class Logic {			
		public:
			static Value * or_ (Frame * frame);
			static Value * and_ (Frame * frame);
			static Value * not_ (Frame * frame);
		
			static Value * when (Frame * frame);
			static Value * if_ (Frame * frame);
			
			static Value * block (Frame * frame);
			static Value * return_ (Frame * frame);
			
			static Value * trueValue ();
			static Value * falseValue ();
			static Value * anythingValue ();
			static Value * nothingValue ();
			
			static void import (Table *);
	};

}
#endif
