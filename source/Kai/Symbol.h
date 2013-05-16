//
//  Symbol.h
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 28/12/11.
//  Copyright (c) 2011 Orion Transfer Ltd. All rights reserved.
//

#ifndef _KAI_SYMBOL_H
#define _KAI_SYMBOL_H

#include "Object.h"

namespace Kai {
	
	typedef uint64_t HashT;
	
	class Symbol : public Object {
	protected:
		const StringT _value;
		const HashT _hash;
		
	public:
		static const char * const NAME;
		
		static HashT calculate_hash (const char * value);
		
		Symbol(const StringT & string);
		virtual ~Symbol();
		
		virtual Ref<Symbol> identity(Frame * frame) const;
		
		const StringT & value() { return _value; }
		
		HashT hash() const { return _hash; }
		
		static Symbol * nil_symbol(Frame * frame);
		static Symbol * false_symbol(Frame * frame);
		static Symbol * true_symbol(Frame * frame);
		
		virtual ComparisonResult compare (const Object * other) const;
		ComparisonResult compare (const Symbol * other) const;
		
		virtual void to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const;
		
		virtual Ref<Object> evaluate(Frame * frame);
		
		static Ref<Object> hash(Frame * frame);
		static Ref<Object> assign(Frame * frame);
		
		static void import(Frame * frame);
	};
	
	class Cell;
	
	class SymbolTable : public Object {
	protected:
		enum { ENTRIES = 1024 };
		Cell * _entries[ENTRIES];
		
	public:
		SymbolTable();
		virtual ~SymbolTable();
		
		Symbol * fetch(Frame * frame, const char * name);
	};
}

#endif
