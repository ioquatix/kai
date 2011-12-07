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
#include "Reference.h"

#include <set>

namespace Kai {
	// Comparison Results
	typedef std::ptrdiff_t ComparisonResult;
	
	const ComparisonResult COMPARISON_ASCENDING = 1;
	const ComparisonResult COMPARISON_DESCENDING = -1;
	const ComparisonResult COMPARISON_EQUAL = 0;

	class InvalidComparison {};
	class InvalidInvocation {};
	
	typedef std::set<const Value *> MarkedT;
	
	class Frame;
	class Table;
	class Symbol;

#pragma mark -
#pragma mark Value
	
	void debug (Value * value);
	
	template <typename ThisT>
	inline static int derivedCompare (const ThisT * lhs, const Value * rhs) {
		const ThisT * other = dynamic_cast<const ThisT *>(rhs);

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
	
	class Value : virtual public SharedObject {
		public:
			Value ();
			virtual ~Value ();

			/// Compare the value with another value. Returns -1, 0, 1 depending on comparison result.
			/// If objects cannot be compared, throws InvalidComparison exception.
			virtual int compare (const Value * other) const;
			
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
			
			/// Write the value to the given buffer.
			virtual void toCode(StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const;
						
			inline void toCode(StringStreamT & buffer) const {
				MarkedT marks;
				toCode(buffer, marks, 0); 
			}
			
			void debug ();

			/// Lookup the given identifier. Defers to prototype by default.
			virtual Ref<Value> lookup (Symbol * identifier);
			
			/// A prototype specifies the behaviour of the current value.
			virtual Ref<Value> prototype ();
			
			/// Evaluate the current value in the given context.
			virtual Ref<Value> evaluate (Frame * frame);

			static StringT toString (const Value * value);
			static bool toBoolean (const Value * value);
			static int compare(const Value * lhs, const Value * rhs);
			static bool equal(const Value * lhs, const Value * rhs);
			
			// Returns (value {this})
			Ref<Value> asValue ();
			
			// Converts the argument to a string value
			static Ref<Value> toString (Frame * frame);
			
			// Converts the argument to a boolean symbol
			static Ref<Value> toBoolean (Frame * frame);
			
			// Compares the given arguments
			static Ref<Value> compare (Frame * frame);
			
			// Compares the given values and returns a true/false value
			static Ref<Value> equal (Frame * frame);
			
			/// Returns a prototype for the given object.
			static Ref<Value> prototype (Frame * frame);
			
			// Returns the arguments unevaluated
			static Ref<Value> value (Frame * frame);
			
			// Evaluates arguments one at a time in result of the previous.
			static Ref<Value> lookup (Frame * frame);
			
			// Performs a method call with the given function.
			static Ref<Value> call (Frame * frame);
			
			static Ref<Value> sleep (Frame * frame);
			
			/// The global value prototype.
			static Ref<Value> globalPrototype ();
			/// Import the global prototype and associated functions into an execution context.
			static void import (Table * context);
	};

#pragma mark -
#pragma mark Cell

	class Cell : public Value {
		protected:
			Ref<Value> m_head;
			Ref<Value> m_tail;
			
		public:
			Cell (Value * head = NULL, Value * tail = NULL);
			virtual ~Cell ();
			
			Ref<Value> head () { return m_head; }
			const Ref<Value> head () const { return m_head; }
			Ref<Value> tail () { return m_tail; }
			const Ref<Value> tail () const { return m_tail; }
		
			template <typename AnyT>
			AnyT * headAs () { return m_head.as<AnyT>(); }

			template <typename AnyT>
			AnyT * tailAs () { return m_tail.as<AnyT>(); }
		
			Cell * insert (Value * value);
			Cell * append (Value * value);
			
			template <typename FirstT>
			static Cell * append (Cell * list, Value * item, FirstT & first) {
				if (list) {
					return list->append(item);
				} else {
					return (first = new Cell(item));
				}
			}
			
			unsigned count ();
			
			virtual int compare (const Value * other) const;
			int compare (const Cell * other) const;

			virtual Ref<Value> prototype ();
			
			virtual void toCode(StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const;
			
			virtual Ref<Value> evaluate (Frame * frame);
			
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
				
					operator Ref<Value> () {
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
			static Ref<Value> _new (Frame * frame);
			static Ref<Value> head (Frame * frame);
			static Ref<Value> tail (Frame * frame);
			static Ref<Value> list (Frame * frame);
			
			static Ref<Value> each (Frame * frame);
						
			static Ref<Value> globalPrototype ();
			static void import (Table * context);
	};

#pragma mark -
#pragma mark String

	class String : public Value {
		protected:
			StringT m_value;
			
		public:
			String (const StringT & value, bool unescape = false);
			virtual ~String ();
			
			StringT & value () { return m_value; }
			const StringT & value () const { return m_value; }
			
			virtual int compare (const Value * other) const;
			int compare (const String * other) const;
			
			virtual void toCode(StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const;
			
			static Ref<Value> join(Frame * frame);
			static Ref<Value> size(Frame * frame);
			static Ref<Value> at(Frame * frame);
		
			// These two functions are unicode aware
			static Ref<Value> length(Frame * frame);
			static Ref<Value> each(Frame * frame);
			
			virtual Ref<Value> prototype ();
			static Ref<Value> globalPrototype ();
			static void import (Table * context);
	};

#pragma mark -
#pragma mark Symbol
	
	class Symbol : public Value {
		protected:
			const StringT m_value;
			const int m_hash;
			
		public:
			static int calculateHash (const StringT & string);
			
			Symbol (const StringT & string);
			virtual ~Symbol ();
			
			const StringT & value () { return m_value; }
			
			int hash () const { return m_hash; }
			
			virtual int compare (const Value * other) const;
			int compare (const Symbol * other) const;
			
			virtual void toCode(StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const;
			
			virtual Ref<Value> evaluate (Frame * frame);
			
			static Symbol * nilSymbol ();
			static Symbol * falseSymbol ();
			static Symbol * trueSymbol ();
			
			static Ref<Value> hash (Frame * frame);
			static Ref<Value> assign (Frame * frame);
			
			virtual Ref<Value> prototype ();
			static Ref<Value> globalPrototype ();
			static void import (Table * context);
	};
	
	template <typename SymbolNameT>
	inline Symbol * sym(const SymbolNameT & name)
	{
		return new Symbol(name);
	}

#pragma mark -
#pragma mark Integer

	class Integer : public Value {
		public:
			typedef int32_t ValueT;
			
		protected:
			ValueT m_value;
			
		public:
			Integer (ValueT value);
			virtual ~Integer ();
			
			ValueT & value () { return m_value; }
			
			virtual Ref<Value> prototype ();
			
			virtual int compare (const Value * other) const;
			int compare (const Integer * other) const;
			
			virtual void toCode(StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const;
			
			static Ref<Value> sum (Frame * frame);
			static Ref<Value> product (Frame * frame);
			static Ref<Value> subtract (Frame * frame);
			static Ref<Value> modulus (Frame * frame);
			
			static Ref<Value> globalPrototype ();
			static void import (Table * context);
	};

#pragma mark -
#pragma mark Table

	class Table : public Value {
		public:
			struct Bin {
				Ref<Symbol> key;
				Ref<Value> value;
				Bin * next;
			};
			
		public:
			Table(int size = 16);
			virtual ~Table();
			
			Bin * find (Symbol * key);
			Ref<Value> update (Symbol * key, Value * value);
			Ref<Value> remove (Symbol * key);
		
			virtual int compare (const Value * other) const;
			int compare (const Table * other) const;
						
			virtual void toCode(StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const;
		
			virtual Ref<Value> lookup (Symbol * key);
						
			void setPrototype (Value * prototype);
			virtual Ref<Value> prototype ();
			
			//% (table [key, value])
			static Ref<Value> table (Frame * frame);
			
			//% (update table key value) -> old_value
			static Ref<Value> update (Frame * frame);

			static Ref<Value> set (Frame * frame);
			
			//% (lookup table key) -> value || nil
			static Ref<Value> lookup (Frame * frame);

			// Iteration over key/value pairs
			static Ref<Value> each (Frame * frame);
			
			// % (setPrototype table value)
			static Ref<Value> setPrototype (Frame * frame);
			
			static Ref<Value> globalPrototype ();
			static void import (Table *);
			
		protected:
			Ref<Value> m_prototype;
			
			std::vector<Bin*> m_bins;
	};
	
#pragma mark -
#pragma mark Logic

	class Logic {			
		public:
			static Ref<Value> or_ (Frame * frame);
			static Ref<Value> and_ (Frame * frame);
			static Ref<Value> not_ (Frame * frame);
		
			static Ref<Value> when (Frame * frame);
			static Ref<Value> if_ (Frame * frame);
			
			static Ref<Value> block (Frame * frame);
			static Ref<Value> return_ (Frame * frame);
			
			static Ref<Value> trueValue ();
			static Ref<Value> falseValue ();
			static Ref<Value> anythingValue ();
			static Ref<Value> nothingValue ();
			
			static void import (Table *);
	};

}
#endif
