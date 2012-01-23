//
//  String.h
//  Kai
//
//  Created by Samuel Williams on 29/12/11.
//  Copyright (c) 2011 Orion Transfer Ltd. All rights reserved.
//

#ifndef _KAI_STRING_H
#define _KAI_STRING_H

#include "Object.h"

namespace Kai {
	
	class String : public Object {
	protected:
		StringT _value;
		
	public:
		static const char * const NAME;
		
		String(const StringT & value, bool unescape = false);
		virtual ~String();
		
		virtual Ref<Symbol> identity(Frame * frame) const;
		
		StringT & value() { return _value; }
		const StringT & value() const { return _value; }
		
		virtual ComparisonResult compare(const Object * other) const;
		ComparisonResult compare(const String * other) const;
		
		virtual void to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const;
		
		static Ref<Object> join(Frame * frame);
		static Ref<Object> size(Frame * frame);
		static Ref<Object> at(Frame * frame);
		
		// These two functions are unicode aware
		static Ref<Object> length(Frame * frame);
		static Ref<Object> each(Frame * frame);
		
		static void import (Frame * frame);
	};
	
}

#endif
