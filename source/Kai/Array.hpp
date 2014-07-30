//
//  Array.h
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 21/09/10.
//  Copyright 2010 Orion Transfer Ltd. All rights reserved.
//
//

#ifndef _KAI_ARRAY_H
#define _KAI_ARRAY_H

#include "Object.hpp"
#include <deque>

namespace Kai {
	
	class Array : public Object {
	public:
		typedef std::deque<Object *> ArrayT;
		typedef ArrayT::iterator IteratorT;
		typedef ArrayT::const_iterator ConstIteratorT;
		
	protected:
		ArrayT _value;
		
	public:
		static const char * const NAME;
		
		Array();
		virtual ~Array();
		
		virtual Ref<Symbol> identity(Frame * frame) const;
		virtual void mark(Memory::Traversal * traversal) const;
		
		ArrayT & value() { return _value; }
		const ArrayT & value() const { return _value; }
		
		virtual ComparisonResult compare(const Object * other) const;
		ComparisonResult compare(const Array * other) const;
		
		virtual void to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const;
		
		static Ref<Object> new_(Frame * frame);
		
		static Ref<Object> minimum(Frame * frame);
		static Ref<Object> maximum(Frame * frame);
		static Ref<Object> at(Frame * frame);
		
		static Ref<Object> push_back(Frame * frame);
		static Ref<Object> pop_back(Frame * frame);			
		static Ref<Object> push_front(Frame * frame);
		static Ref<Object> pop_front(Frame * frame);
		
		static Ref<Object> append(Frame * frame);
		static Ref<Object> prepend(Frame * frame);
		static Ref<Object> insert(Frame * frame);
		
		static Ref<Object> includes(Frame * frame);
		
		static Ref<Object> each(Frame * frame);
		static Ref<Object> collect(Frame * frame);
		static Ref<Object> select(Frame * frame);
		static Ref<Object> find(Frame * frame);
		
		static void import(Frame * frame);
	};
	
}

#endif
