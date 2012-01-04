/*
 *  Parser.h
 *  Kai
 *
 *  Created by Samuel Williams on 14/07/10.
 *  Copyright 2010 Orion Transfer Ltd. All rights reserved.
 *
 */
 
#include "Token.h"

namespace Kai {
	
	namespace Parser {
		Token parseInteger(StringIteratorT begin, StringIteratorT end);
		Token parseString(StringIteratorT begin, StringIteratorT end);
		Token parseDecimal(StringIteratorT begin, StringIteratorT end);
		Token parseHexadecimal(StringIteratorT begin, StringIteratorT end);
		Token parseWhitespace(StringIteratorT begin, StringIteratorT end);
		Token parseIdentifier(StringIteratorT begin, StringIteratorT end);
		
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
