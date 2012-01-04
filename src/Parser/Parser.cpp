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
		Token parse_single_line_comment (StringIteratorT begin, StringIteratorT end) {
			static const StringT SINGLE_LINE_COMMENT = "#";
			
			Token t(begin, COMMENTS);
			
			if (t &= parse_constant(t.end(), end, SINGLE_LINE_COMMENT)) {
				t += parse_characters(t.end(), end, &Unicode::isNotNewline);
			}
			
			return t;
		}
		
		Token parse_multiple_line_comment (StringIteratorT begin, StringIteratorT end) {
			static const StringT OPEN_COMMENT = "/*";
			static const StringT CLOSE_COMMENT = "*/";
			
			Token t(begin, COMMENTS);
			
			if (t += parse_constant(t.end(), end, OPEN_COMMENT)) {				
				while (t.end() != end) {
					Token u = parse_constant(t.end(), end, CLOSE_COMMENT);
					
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
		
		Token parse_comment (StringIteratorT begin, StringIteratorT end) {
			Token t;
			
			t = parse_single_line_comment(begin, end);
			if (t) {
				return t;
			}
			
			t = parse_multiple_line_comment(begin, end);
			if (t) {
				return t;
			}
			
			return t;
		}
		
		Token parse_whitespace (StringIteratorT begin, StringIteratorT end) {
			Token t(begin);
			StringIteratorT cur = begin;
			
			while (true) {
				t += parse_characters(t.end(), end, Unicode::isWhitespace);
				t += parse_comment(t.end(), end);
				t += parse_characters(t.end(), end, Unicode::isWhitespace);
				
				if (t.end() == cur)
					break;
				
				cur = t.end();
			}
			
			return t;
		}
		
		Token parse_integer (StringIteratorT begin, StringIteratorT end) {
			Token t(begin, NUMBER);
			
			t &= parse_characters(t.end(), end, Unicode::isNumeric);
			
			return t;
		}

		Token parse_decimal (StringIteratorT begin, StringIteratorT end) {
			static const StringT Number_POINT = ".";
			
			Token t(begin, DECIMAL), u;
			
			t += parse_constant(t.end(), end, "-");
			
			if (t &= parse_characters(t.end(), end, Unicode::isNumeric)) {
				if ((u = parse_constant(t.end(), end, Number_POINT))) {
					u &= parse_characters(u.end(), end, Unicode::isNumeric);
					
					if (u)
						t += u;
				}			
			}
			
			return t;
		}
		
		Token parse_hexadecimal(StringIteratorT begin, StringIteratorT end) {
			static const StringT PREFIX = "0x";
			
			Token t(begin, NUMBER);
			
			if (t &= parse_constant(t.end(), end, PREFIX)) {
				return parse_characters(t.end(), end, Unicode::isHexadecimal);
			}
			
			return t;
		}
		
		Token parse_string (StringIteratorT begin, StringIteratorT end) {
			static const StringT QUOTE = "\"";
			static const StringT ESCAPED_QUOTE = "\\\"";
			
			Token t(begin, STRING);
			
			if (t &= parse_constant(t.end(), end, QUOTE)) {
				while (t.end() != end) {
					Token u;
					
					if ((u = parse_constant(t.end(), end, QUOTE))) {
						t += u;
						
						return t;
					} else if ((u = parse_constant(t.end(), end, ESCAPED_QUOTE))) {
						t += u;
					} else {
						++t;
					}
				}
				
				throw FatalParseFailure(t, "Unterminated string");
			}
			
			return t;
		}
		
		bool is_identifier_start (Unicode::CodePointT codePoint) {
			return Unicode::isLetter(codePoint) || codePoint == '_';
		}
		
		bool is_identifier_middle (Unicode::CodePointT codePoint) {
			return Unicode::isLetter(codePoint) || Unicode::isAlphaNumeric(codePoint) || codePoint == '_' || codePoint == '-';
		}
		
		bool is_identifier_end (Unicode::CodePointT codePoint) {
			return codePoint == '?' || codePoint == '!' || codePoint == '=';
		}
	
		// Parses a variable name
		Token parse_identifier (StringIteratorT begin, StringIteratorT end) {
			Token t(begin, SYMBOL);
			
			t &= parse_characters(t.end(), end, is_identifier_start, Counter(1, 1));
			if (!t) return t;
			
			t += parse_characters(t.end(), end, is_identifier_middle);
			t += parse_characters(t.end(), end, is_identifier_end, Counter(0, 1));
			
			return t;
		}
		
	}
}
