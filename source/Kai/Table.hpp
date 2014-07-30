//
//  Table.h
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 16/12/11.
//  Copyright (c) 2011 Orion Transfer Ltd. All rights reserved.
//

#ifndef _KAI_TABLE_H
#define _KAI_TABLE_H

#include "Object.hpp"

namespace Kai {
	
	class Table : public Object {
	public:
		struct Bin {
			Symbol * key;
			Object * value;
			Bin * next;
		};
		
	public:
		static const char * const NAME;
		
		Table(int size = 16);
		virtual ~Table();
		
		virtual Ref<Symbol> identity(Frame * frame) const;
		
		virtual void mark(Memory::Traversal * traversal) const;
		
		Bin * find(Symbol * key);
		Ref<Object> update(Symbol * key, Object * value);
		Ref<Object> remove(Symbol * key);
		
		virtual ComparisonResult compare(const Object * other) const;
		ComparisonResult compare(const Table * other) const;
		
		virtual void to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const;
		
		virtual Ref<Object> lookup(Frame * frame, Symbol * key);
		
		void set_prototype(Object * prototype);
		virtual Ref<Object> prototype(Frame * frame) const;
		
		//% (table [key, value])
		static Ref<Object> new_(Frame * frame);
		
		//% (update table key value) -> old_value
		static Ref<Object> update(Frame * frame);
		
		static Ref<Object> set(Frame * frame);
		
		//% (lookup table key) -> value || nil
		static Ref<Object> lookup(Frame * frame);
		
		// Iteration over key/value pairs
		static Ref<Object> each(Frame * frame);
		
		// % (set_prototype table value)
		static Ref<Object> set_prototype(Frame * frame);
		
		static void import(Frame *);
		
	protected:
		Object * _prototype;
		
		std::vector<Bin*> _bins;
	};
}

#endif
