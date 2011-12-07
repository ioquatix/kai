/*
 *  Array.h
 *  Kai
 *
 *  Created by Samuel Williams on 21/09/10.
 *  Copyright 2010 Orion Transfer Ltd. All rights reserved.
 *
 */

#ifndef _KAI_ARRAY_H
#define _KAI_ARRAY_H

#include "Value.h"
#include <deque>

namespace Kai {
	
	//typedef gc_allocator<Value*> ValuePointerAllocatorT;
	
	class Array : public Value {
		public:
			typedef std::deque<Ref<Value>> ArrayT;
			typedef ArrayT::iterator IteratorT;
			typedef ArrayT::const_iterator ConstIteratorT;
			
		protected:
			ArrayT m_value;
			
		public:
			Array ();
			virtual ~Array ();
			
			ArrayT & value () { return m_value; }
						
			virtual int compare (const Value * other) const;
			int compare (const Array * other) const;
			
			virtual void toCode(StringStreamT & buffer, MarkedT & marks, std::size_t indentation);
			
			static Ref<Value> _new (Frame * frame);
			
			static Ref<Value> minimum (Frame * frame);
			static Ref<Value> maximum (Frame * frame);
			static Ref<Value> at (Frame * frame);
						
			static Ref<Value> push_back(Frame * frame);
			static Ref<Value> pop_back(Frame * frame);			
			static Ref<Value> push_front(Frame * frame);
			static Ref<Value> pop_front(Frame * frame);
			
			static Ref<Value> append(Frame * frame);
			static Ref<Value> prepend(Frame * frame);
			static Ref<Value> insert(Frame * frame);
			
			static Ref<Value> includes(Frame * frame);
			
			static Ref<Value> each(Frame * frame);
			static Ref<Value> collect(Frame * frame);
			static Ref<Value> select(Frame * frame);
			static Ref<Value> find(Frame * frame);
			
			virtual Ref<Value> prototype ();
			static Ref<Value> globalPrototype ();
			static void import (Table * context);
	};
	
}

#endif
