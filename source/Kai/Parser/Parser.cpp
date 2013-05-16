//
//  Parser.cpp
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 17/06/10.
//  Copyright 2010 Samuel Williams. All rights reserved.
//
//

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
				t += parse_characters(t.end(), end, &Unicode::is_not_newline);
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
				t += parse_characters(t.end(), end, Unicode::is_whitespace_or_newline);
				t += parse_comment(t.end(), end);
				t += parse_characters(t.end(), end, Unicode::is_whitespace_or_newline);
				
				if (t.end() == cur)
					break;
				
				cur = t.end();
			}
			
			return t;
		}
		
		Token parse_integer (StringIteratorT begin, StringIteratorT end) {
			Token t(begin, NUMBER);
			
			t &= parse_characters(t.end(), end, Unicode::is_numeric);
			
			return t;
		}

		Token parse_decimal (StringIteratorT begin, StringIteratorT end) {
			static const StringT DECIMAL_POINT = ".";
			
			Token t(begin, DECIMAL), u;
			
			t += parse_constant(t.end(), end, "-");
			
			if (t &= parse_characters(t.end(), end, Unicode::is_numeric)) {
				if ((u = parse_constant(t.end(), end, DECIMAL_POINT))) {
					u &= parse_characters(u.end(), end, Unicode::is_numeric);
					
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
				return parse_characters(t.end(), end, Unicode::is_hexadecimal);
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
		
		bool is_identifier_start (Unicode::CodePointT code_point) {
			return Unicode::is_letter(code_point) || code_point == '_';
		}
		
		bool is_identifier_middle (Unicode::CodePointT code_point) {
			return Unicode::is_letter(code_point) || Unicode::is_alpha_numeric(code_point) || code_point == '_' || code_point == '-';
		}
		
		bool is_identifier_end (Unicode::CodePointT code_point) {
			return code_point == '?' || code_point == '!' || code_point == '=';
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
		
		Token parse_heredoc(StringIteratorT begin, StringIteratorT end) {
			Token start = parse_constant(begin, end, "<-");
			
			if (start) {
				Token identifier = parse_identifier(start.end(), end);
				
				if (identifier) {
					Token eol = parse_constant(identifier.end(), end, "\n");
					
					if (eol) {
						// Setup the buffer to contain the body of the heredoc:
						Token buffer(eol.end());
						
						// This identifier will finish the heredoc:
						std::string eof_string = identifier.value();
						
						// Consume text until we encounter the eof_string on a line with only whitespace preceeding it:
						while (buffer.end() != end) {
							Token line(buffer.end());
							line << parse_characters(line.end(), end, &Unicode::is_whitespace, Counter(0));
							
							Token eof = parse_constant(line.end(), end, eof_string);
							if (eof) {
								// Return the identifier, buffer, eof marker and indentation.
								start << identifier;
								start << buffer;
								
								line << eof;
								start << line;
								
								return start;
							}
							
							line += parse_characters(line.end(), end, &Unicode::is_not_newline);
							
							// TODO: What happens if this fails!?
							line += parse_constant(line.end(), end, "\n");
							
							buffer += line;
						}
						
						// We never encountered the end of the heredoc:
						throw FatalParseFailure(buffer, "Unterminated heredoc");
					} else {
						return eol;
					}
				} else {
					return identifier;
				}
			} else {
				return start;
			}
		}
		
		/*
		Token parse_interpolation(StringIteratorT begin, StringIteratorT end) {
			Token t(begin, STRING);
			
			StringIteratorT s = begin;
			
			while (s != end) {
				StringIteratorT t = s;
				
				Unicode::CodePointT code_point = Unicode::next(t, end);
				
				if (t != end) {
					Unicode::CodePointT next_code_point = Unicode::next(t, end);
					
					if (code_point == '#' && next_code_point == '(') {
						// Now processing expression directive
						
						t &= parse_interpolation_expression(s, end);
					} else if (code_point == '<' && next_code_point == '#') {
						// Now processing tag directive
						
						t &= parse_interpolation_tag(s, end);
					}
				}
			}
		}*/
	}
}
