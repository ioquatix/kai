//
//  Table.h
//  Kai
//
//  Created by Samuel Williams on 16/12/11.
//  Copyright (c) 2011 Orion Transfer Ltd. All rights reserved.
//

#ifndef Kai_Table_h
#define Kai_Table_h

#include "Value.h"

namespace Kai {
	class Table : public Value {
	public:
		struct Bin {
			Symbol * key;
			Value * value;
			Bin * next;
		};
		
	public:
		Table(int size = 16);
		virtual ~Table();
		
		virtual void mark();
		
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
}

#endif
