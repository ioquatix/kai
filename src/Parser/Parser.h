//
//  Parser.h
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 14/07/10.
//  Copyright 2010 Orion Transfer Ltd. All rights reserved.
//
//
 
#include "Token.h"

namespace Kai {
	
	namespace Parser {
		Token parse_integer(StringIteratorT begin, StringIteratorT end);
		Token parse_string(StringIteratorT begin, StringIteratorT end);
		Token parse_decimal(StringIteratorT begin, StringIteratorT end);
		Token parse_hexadecimal(StringIteratorT begin, StringIteratorT end);
		Token parse_whitespace(StringIteratorT begin, StringIteratorT end);
		Token parse_identifier(StringIteratorT begin, StringIteratorT end);
		Token parse_heredoc(StringIteratorT begin, StringIteratorT end);
		
		Token parse_interpolation(StringIteratorT begin, StringIteratorT end);
		
		/*
			[(this) set grammar [Grammar new]]
			
			[grammar define `eol `(fn (begin end)
				(var `token [Token new begin])
				
				
				
				(or (parse-string begin end ";") (parse-string "\n"))
			)]
		*/
		/*
		class Grammar
		{
			protected:
				Table * _rules;
			
			public:
				/// [Grammar define name rule]
				static Ref<Object> define(Frame * frame);
				
				static Ref<Object> parse(Frame * frame);
		};
		*/
	}
}
