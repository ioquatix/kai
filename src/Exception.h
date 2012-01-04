/*
 *  Exception.h
 *  This file is part of the "Kai" project, and is licensed under the GNU GPLv3.
 *
 *  Created by Samuel Williams on 13/04/10.
 *  Copyright 2010 Samuel Williams. All rights reserved.
 *
 */

#ifndef _KAI_EXCEPTION_H
#define _KAI_EXCEPTION_H

#include "Kai.h"
#include "Reference.h"

namespace Kai {

	class Object;
	class Cell;
	class Frame;
	
	class Exception {
	protected:
		StringT _what;
		Ref<Object> _object;
		Ref<Frame> _frame;
		
	public:
		Exception(StringT what, Frame * frame);
		Exception(StringT what, Object * object, Frame * frame);

		virtual ~Exception();
		
		Frame * top();
		StringT what();
	
		virtual StringT name();
	};

	class ArgumentError : public Exception {
	public:
		ArgumentError(StringT what, Object * value, Frame * frame);
		
		virtual StringT name();
	};
	
	class RangeError : public Exception {
	public:
		RangeError(StringT what, Object * value, Frame * frame);
		
		virtual StringT name();
	};
	
}

#endif
