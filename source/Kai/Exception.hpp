//
//  Exception.h
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 13/04/10.
//  Copyright 2010 Samuel Williams. All rights reserved.
//
//

#ifndef _KAI_EXCEPTION_H
#define _KAI_EXCEPTION_H

#include "Kai.hpp"
#include "Reference.hpp"

namespace Kai {
	
	class Object;
	class Cell;
	class Frame;
	
	class Exception : std::exception {
	protected:
		StringT _what;
		Ref<Object> _object;
		Ref<Frame> _frame;
		
	public:
		Exception(StringT what, Frame * frame);
		Exception(StringT what, Object * object, Frame * frame);
		
		virtual ~Exception();
		
		Frame * top();
		virtual StringT message() noexcept;

		// For std::exception
		virtual const char* what() const noexcept;
		
		virtual StringT name() noexcept;
	};
	
	class ArgumentError : public Exception {
	protected:
		StringT _name, _type;
		
	public:
		ArgumentError(StringT name, StringT type, Object * value, Frame * frame);
		
		virtual StringT message() noexcept;
		virtual StringT name() noexcept;
	};
	
	class RangeError : public Exception {
	public:
		RangeError(StringT what, Object * value, Frame * frame);
		
		virtual StringT name() noexcept;
	};
	
}

#endif
