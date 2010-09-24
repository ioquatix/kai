/*
 *  Parser.cpp
 *  Kai
 *
 *  Created by Samuel Williams on 17/06/10.
 *  Copyright 2010 Samuel Williams. All rights reserved.
 *
 */

#include "Parser.h"
#include "../SourceCode.h"
#include "../Value.h"

#define auto_type(name,value) typeof((value)) name = value

/*
	operator(:binary, ':', "High Precidence Member")
	operator(:bin ary, "=", "
	B-OP: 
		B-OP ':' High Precidence Member
	B-OP: '=' 
	B-OP: '==' '<=' '>=' '!='
	B-OP: 'in', 'is'
	B-OP: '
*/

namespace Kai {
	namespace Parser {
		Token parseSingleLineComment (StringIteratorT begin, StringIteratorT end) {
			static const StringT SINGLE_LINE_COMMENT = "#";
			
			Token t(begin, COMMENTS);
			
			if (t &= parseConstant(t.end(), end, SINGLE_LINE_COMMENT)) {
				t += parseCharacters(t.end(), end, &isNotNewline);
			}
			
			return t;
		}
		
		Token parseMultiLineComment (StringIteratorT begin, StringIteratorT end) {
			static const StringT OPEN_COMMENT = "/*";
			static const StringT CLOSE_COMMENT = "*/";
			
			Token t(begin, COMMENTS);
			
			if (t += parseConstant(t.end(), end, OPEN_COMMENT)) {				
				while (t.end() != end) {
					Token u;
					
					if (u = parseConstant(t.end(), end, CLOSE_COMMENT)) {
						t += u;
						
						return t;
					} else {
						++t;
					}				
				}
				
				throw FatalParseFailure(t, "Unterminated comment");
			}
			
			return t;
		}
		
		Token parseComment (StringIteratorT begin, StringIteratorT end) {
			Token t;
			
			if (t = parseSingleLineComment(begin, end)) {
				return t;
			}
			
			if (t = parseMultiLineComment(begin, end)) {
				return t;
			}
			
			return t;
		}
		
		Token parseWhitespace (StringIteratorT begin, StringIteratorT end) {
			Token t(begin);
			
			t += parseCharacters(t.end(), end, isWhitespace);
			t += parseComment(t.end(), end);
			t += parseCharacters(t.end(), end, isWhitespace);
			
			return t;
		}
		
		Token parseNumber (StringIteratorT begin, StringIteratorT end) {
			Token t(begin, NUMBER);
			
			t += parseConstant(t.end(), end, "-");
			t &= parseCharacters(t.end(), end, isNumeric);
			
			return t;
		}

		Token parseDecimal (StringIteratorT begin, StringIteratorT end) {
			static const StringT DECIMAL_POINT = ".";
			
			Token t(begin, DECIMAL), u;
			
			if (t &= parseCharacters(t.end(), end, isNumeric)) {
				if (u = parseConstant(t.end(), end, DECIMAL_POINT)) {
					u &= parseCharacters(t.end(), end, isNumeric);
					
					if (u)
						t += u;
				}			
			}
			
			return t;
		}
		
		Token parseString (StringIteratorT begin, StringIteratorT end) {
			static const StringT QUOTE = "\"";
			static const StringT ESCAPED_QUOTE = "\\\"";
			
			Token t(begin, STRING);
			
			if (t &= parseConstant(t.end(), end, QUOTE)) {
				while (t.end() != end) {
					Token u;
					
					if (u = parseConstant(t.end(), end, QUOTE)) {
						t += u;
						
						return t;
					} else if (u = parseConstant(t.end(), end, ESCAPED_QUOTE)) {
						t += u;
					} else {
						++t;
					}
				}
				
				throw FatalParseFailure(t, "Unterminated string");
			}
			
			return t;
		}
		
		bool isIdentifierEnding (StringIteratorT i) {
			return *i == '?' || *i == '!' || *i == '=';
		}
	
		// Parses a variable name
		Token parseIdentifier (StringIteratorT begin, StringIteratorT end) {
			Token t(begin, SYMBOL);
			
			t &= parseCharacters(t.end(), end, isAlpha, Counter(1, 1));
			if (!t) return t;
			
			t += parseCharacters(t.end(), end, isAlphaNumeric);
			t += parseCharacters(t.end(), end, isIdentifierEnding, Counter(0, 1));
			
			return t;
		}
		
	}
}
