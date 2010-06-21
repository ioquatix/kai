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

namespace Kai {
	// Comparison Results
	typedef std::ptrdiff_t ComparisonResult;
	
	const ComparisonResult COMPARISON_ASCENDING = 1;
	const ComparisonResult COMPARISON_DESCENDING = -1;
	const ComparisonResult COMPARISON_EQUAL = 0;

	class InvalidComparison {};
	class InvalidInvocation {};

	struct Frame;
	class Table;
	class Symbol;

#pragma mark -
#pragma mark Value
	
	//typedef Value * (*EvaluateFunctionT)(Frame *);
	
	class Value : public gc {
		public:
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
			
			virtual void toCode (StringStreamT & buffer) = 0;
			
			void debug ();

			// Lookup the given identifier. Defers to prototype.
			virtual Value * lookup (Symbol * identifier);
			
			// A prototype specifies the behaviour of the current value.
			virtual Value * prototype ();
			
			// Evaluate the current value in the given context.
			virtual Value * evaluate (Frame * frame);
			
			static StringT toString (Value * value);
			static bool toBoolean (Value * value);
			static int compare (Value * lhs, Value * rhs);
			
			// Converts the argument to a string value
			static Value * toString (Frame * frame);
			
			// Converts the argument to a boolean symbol
			static Value * toBoolean (Frame * frame);
			
			// Compares the given arguments
			static Value * compare (Frame * frame);
			
			// Returns a prototype for the given object.
			static Value * prototype (Frame * frame);
			
			// Returns the arguments unevaluated
			static Value * value (Frame * frame);
			
			// Builtins
			static void import (Table * context);
	};

#pragma mark -
#pragma mark Cell

	class Cell : public Value {
		private:
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
			
			virtual void toCode (StringStreamT & buffer);
			
			virtual Value * evaluate (Frame * frame);
			
			class ArgumentExtractor {
				Cell * m_current;
				Frame * m_frame;
				
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
					
					inline operator Cell* () {
						return m_current;
					}
			};
			
			inline ArgumentExtractor extract(Frame * frame) {
				return ArgumentExtractor(frame, this);
			}
			
			//% (prepend cell value) -> (cell value)
			static Value * cell (Frame * frame);
			static Value * head (Frame * frame);
			static Value * tail (Frame * frame);
			
			static void import (Table * context);
	};

#pragma mark -
#pragma mark String

	class String : public Value {
		private:
			StringT m_value;
			
		public:
			String (const StringT & value);
			virtual ~String ();
			
			StringT & value () { return m_value; }
			
			virtual int compare (Value * other);
			int compare (String * other);
			
			virtual void toCode (StringStreamT & buffer);
	};

#pragma mark -
#pragma mark Symbol

	class Symbol : public Value {
		private:
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
			
			virtual void toCode (StringStreamT & buffer);
			
			virtual Value * evaluate (Frame * frame);
			
			static Symbol * nilSymbol ();
			static Symbol * falseSymbol ();
			static Symbol * trueSymbol ();
	};

#pragma mark -
#pragma mark Integer

	class Integer : public Value {
		private:
			int m_value;
			
		public:
			Integer (int value);
			virtual ~Integer ();
			
			int & value () { return m_value; }
			
			virtual int compare (Value * other);
			int compare (Integer * other);
			
			virtual void toCode (StringStreamT & buffer);
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
			
		private:
			Table (int size, bool allocate);
			
		public:
			// Inline table allocation
			static Table * allocate (int size = 16);
			
			Bin * find (Symbol * key);
			Value * update (Symbol * key, Value * value);
			Value * remove (Symbol * key);
		
			virtual int compare (Value * other);
			int compare (Table * other);
			
			virtual void toCode (StringStreamT & buffer);
		
			virtual Value * lookup (Symbol * key);
						
			void setPrototype (Value * prototype);
			virtual Value * prototype ();
			
			//% (table [key, value])
			static Value * table (Frame * frame);
			
			//% (update table key value) -> old_value
			static Value * update (Frame * frame);
			
			//% (lookup table key) -> value || nil
			static Value * lookup (Frame * frame);
			
			// % (setPrototype table value)
			static Value * setPrototype (Frame * frame);
			
			static void import (Table *);
			
		private:
			Value * m_prototype;
			
			unsigned m_size;
			Bin ** m_bins;
	};
}
#endif
