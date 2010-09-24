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
	
	class Value;
	
	namespace Parser {
		Token parseNumber (StringIteratorT begin, StringIteratorT end);
		Token parseString (StringIteratorT begin, StringIteratorT end);
		Token parseDecimal (StringIteratorT begin, StringIteratorT end);
		Token parseWhitespace (StringIteratorT begin, StringIteratorT end);
		Token parseIdentifier (StringIteratorT begin, StringIteratorT end);
	}
}