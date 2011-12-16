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
				t += parseCharacters(t.end(), end, &Unicode::isNotNewline);
			}
			
			return t;
		}
		
		Token parseMultiLineComment (StringIteratorT begin, StringIteratorT end) {
			static const StringT OPEN_COMMENT = "/*";
			static const StringT CLOSE_COMMENT = "*/";
			
			Token t(begin, COMMENTS);
			
			if (t += parseConstant(t.end(), end, OPEN_COMMENT)) {				
				while (t.end() != end) {
					Token u = parseConstant(t.end(), end, CLOSE_COMMENT);
					
					if (u) {
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
			
			t = parseSingleLineComment(begin, end);
			if (t) {
				return t;
			}
			
			t = parseMultiLineComment(begin, end);
			if (t) {
				return t;
			}
			
			return t;
		}
		
		Token parseWhitespace (StringIteratorT begin, StringIteratorT end) {
			Token t(begin);
			StringIteratorT cur = begin;
			
			while (true) {
				t += parseCharacters(t.end(), end, Unicode::isWhitespace);
				t += parseComment(t.end(), end);
				t += parseCharacters(t.end(), end, Unicode::isWhitespace);
				
				if (t.end() == cur)
					break;
				
				cur = t.end();
			}
			
			return t;
		}
		
		Token parseNumber (StringIteratorT begin, StringIteratorT end) {
			Token t(begin, NUMBER);
			
			t += parseConstant(t.end(), end, "-");
			t &= parseCharacters(t.end(), end, Unicode::isNumeric);
			
			return t;
		}

		Token parseDecimal (StringIteratorT begin, StringIteratorT end) {
			static const StringT DECIMAL_POINT = ".";
			
			Token t(begin, DECIMAL);
			
			if (t &= parseCharacters(t.end(), end, Unicode::isNumeric)) {
				Token u;
			
				if ((u = parseConstant(t.end(), end, DECIMAL_POINT))) {
					u &= parseCharacters(t.end(), end, Unicode::isNumeric);
					
					if (u)
						t += u;
				}			
			}
			
			return t;
		}
		
		Token parseHexadecimal(StringIteratorT begin, StringIteratorT end) {
			static const StringT PREFIX = "0x";
			
			Token t(begin, NUMBER);
			
			if (t &= parseConstant(t.end(), end, PREFIX)) {
				return parseCharacters(t.end(), end, Unicode::isHexadecimal);
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
					
					if ((u = parseConstant(t.end(), end, QUOTE))) {
						t += u;
						
						return t;
					} else if ((u = parseConstant(t.end(), end, ESCAPED_QUOTE))) {
						t += u;
					} else {
						++t;
					}
				}
				
				throw FatalParseFailure(t, "Unterminated string");
			}
			
			return t;
		}
		
		bool isIdentifierStart (Unicode::CodePointT codePoint) {
			return Unicode::isLetter(codePoint) || codePoint == '_';
		}
		
		bool isIdentifierMiddle (Unicode::CodePointT codePoint) {
			return Unicode::isLetter(codePoint) || Unicode::isAlphaNumeric(codePoint) || codePoint == '_' || codePoint == '-';
		}
		
		bool isIdentifierEnding (Unicode::CodePointT codePoint) {
			return codePoint == '?' || codePoint == '!' || codePoint == '=';
		}
	
		// Parses a variable name
		Token parseIdentifier (StringIteratorT begin, StringIteratorT end) {
			Token t(begin, SYMBOL);
			
			t &= parseCharacters(t.end(), end, isIdentifierStart, Counter(1, 1));
			if (!t) return t;
			
			t += parseCharacters(t.end(), end, isIdentifierMiddle);
			t += parseCharacters(t.end(), end, isIdentifierEnding, Counter(0, 1));
			
			return t;
		}
		
	}
}
