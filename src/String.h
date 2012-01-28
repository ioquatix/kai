//
//  String.h
//  This file is part of the "Kai" project, and is released under the MIT license.
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

		/// Interpolates a string using the standard Kai syntax:
		/// Print a variable/code #{foo}
		/// Execute a loop <# [list each {|item| #>#{item}<# } #>
		Ref<Object> interpolation(Frame * frame) const;
		
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
		
		// A helper for the heredoc syntax
		static Ref<Object> heredoc(Frame * frame);
		
		static void import (Frame * frame);
	};

	class StringBuffer : public Object {
	protected:
		StringStreamT _value;
		
	public:
		static const char * const NAME;
		
		StringBuffer();
		virtual ~StringBuffer();
		
		virtual Ref<Symbol> identity(Frame * frame) const;
		
		void append(const StringT & string);
		StringT to_string() const;
		
		StringStreamT & value() { return _value; }
		const StringStreamT & value() const { return _value; }
		
		static Ref<Object> new_(Frame * frame);
		static Ref<Object> append(Frame * frame);
		static Ref<Object> to_string(Frame * frame);
		
		static void import(Frame * frame);
	};
	
}

#endif
