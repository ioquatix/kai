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
	
	typedef gc_allocator<Value*> ValuePointerAllocatorT;
	
	class Array : public Value {
		public:
			typedef std::deque<Value*, ValuePointerAllocatorT> ArrayT;
			typedef ArrayT::iterator IteratorT;
			typedef ArrayT::const_iterator ConstIteratorT;
			
		protected:
			ArrayT m_value;
			
		public:
			Array ();
			virtual ~Array ();
			
			ArrayT & value () { return m_value; }
						
			virtual int compare (Value * other);
			int compare (Array * other);
			
			virtual void toCode(StringStreamT & buffer, MarkedT & marks, std::size_t indentation);
			
			static Value * _new (Frame * frame);
			
			static Value * minimum (Frame * frame);
			static Value * maximum (Frame * frame);
			static Value * at (Frame * frame);
						
			static Value * push_back(Frame * frame);
			static Value * pop_back(Frame * frame);			
			static Value * push_front(Frame * frame);
			static Value * pop_front(Frame * frame);
			
			static Value * append(Frame * frame);
			static Value * prepend(Frame * frame);
			static Value * insert(Frame * frame);
			
			static Value * includes(Frame * frame);
			
			static Value * each(Frame * frame);
			static Value * collect(Frame * frame);
			static Value * select(Frame * frame);
			static Value * find(Frame * frame);
			
			virtual Value * prototype ();
			static Value * globalPrototype ();
			static void import (Table * context);
	};
	
}

#endif
