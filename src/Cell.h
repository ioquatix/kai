/*
 *  Cell.h
 *  This file is part of the "Kai" project, and is licensed under the GNU GPLv3.
 *
 *  Created by Samuel Williams on 10/04/10.
 *  Copyright 2010 Samuel Williams. All rights reserved.
 *
 */
 
#ifndef _KCELL_H
#define _KCELL_H

#include "Kai.h"

namespace Kai {
	class InvalidComparison {};
	class InvalidInvocation {};

	struct Frame;

	#pragma mark -
	#pragma mark Value
	
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
			
			// Invoke a named function.
			virtual Value * invoke (Frame * frame);
			
			// Evaluate the current value in the given context.
			virtual Value * evaluate (Frame * frame);
			
			static StringT toString (Value * value);
			static bool toBoolean (Value * value);
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
		
			virtual Value * invoke (Frame * frame);
			virtual Value * lookup (Symbol * key);
			
			static Value * metaclass ();
			
		private:		
			Table * m_prototype;
			
			unsigned m_size;
			Bin ** m_bins;
	};
}
#endif
