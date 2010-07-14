/*
 *  Parser.cpp
 *  Kai
 *
 *  Created by Samuel Williams on 17/06/10.
 *  Copyright 2010 Samuel Williams. All rights reserved.
 *
 */

#include "SourceCode.h"
#include "Token.h"
#include "Value.h"

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
	
		Token parseExpression(StringIteratorT begin, StringIteratorT end);
	
		Token parseNumber (StringIteratorT begin, StringIteratorT end) {
			Token t(begin, NUMBER);
			
			return t &= parseCharacters(t.end(), end, isNumeric);
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
		
		Token parseArguments (StringIteratorT begin, StringIteratorT end) {
			Token t(begin, ARGUMENTS);
			
			t += parseCharacters(t.end(), end, isWhitespace);
			
			t &= parseConstant(t.end(), end, "(");
			if (!t) return t;
			
			bool first = true;
			while (true) {
				Token k(t.end());
				
				if (!first) {
					k &= parseConstant(k.end(), end, ",");
					if (!k) break;
				} else {
					first = false;
				}
				
				k += parseCharacters(k.end(), end, isWhitespace);

				k &= parseExpression(k.end(), end);
				
				if (k)
					t += k;
				else
					break;
			}
			
			t += parseCharacters(t.end(), end, isWhitespace);

			t &= parseConstant(t.end(), end, ")");
			
			return t;
		}
		
		Token parseFExpression (StringIteratorT begin, StringIteratorT end) {
			Token t(begin, FUNCTION);
			
			t += parseCharacters(t.end(), end, isWhitespace);
			
			t << parseIdentifier(t.end(), end);
			if (!t) return t;
			
			Token arguments = parseArguments(t.end(), end);
			
			if (arguments)
				t << arguments;
			
			return t;
		}
		
		Token parseFExpressionChain (StringIteratorT begin, StringIteratorT end) {
			Token t(begin, CHAIN);
			
			bool first = true;
			while (true) {
				Token k(t.end());
				
				if (!first) {
					k &= parseConstant(k.end(), end, ".");
					if (!k) break;
				} else {
					first = false;
				}
				 
				k << parseFExpression(k.end(), end);
				
				if (k)
					t += k;
				else
					break;
			}
			
			// This is a bit of a hack.
			if (t.length() == 0) {
				return invalid();
			} else {
				return t;
			}
		}

		Token parseValue(StringIteratorT begin, StringIteratorT end) {
			Token t;
			
			if (t = parseDecimal(begin, end)) {
				return t;
			}
			
			if (t = parseNumber(begin, end)) {
				return t;
			}
			
			if (t = parseString(begin, end)) {
				return t;
			}
			
			if (t = parseIdentifier(begin, end)) {
				return t;
			}
			
			return t;
		}
		
		Token parseFactor(StringIteratorT begin, StringIteratorT end) {
			Token t;
			
			if (t = parseFExpressionChain(begin, end)) {
				return t;
			}
			
			if (t = parseValue(begin, end)) {
				return t;
			}
			
			return t;
		}
		
		Token parseExpression(StringIteratorT begin, StringIteratorT end) {
			Token t(begin, EXPRESSION);
			
			OperatorParser factorOperatorParser;
			factorOperatorParser << "*" << "/";
			
			auto_type(factorParser, binaryOperatorParser(parseFactor, factorOperatorParser));
			
			OperatorParser termOperatorParser;
			termOperatorParser << "+" << "-";
			
			auto_type(termParser, binaryOperatorParser(factorParser, termOperatorParser));
			
			OperatorParser comparisonOperatorParser;
			comparisonOperatorParser << "<" << ">" << "<=" << ">=";
			
			auto_type(comparisonParser, binaryOperatorParser(termParser, comparisonOperatorParser));
			
			OperatorParser assignmentOperatorParser;
			assignmentOperatorParser << ":=";
			
			auto_type(assignmentParser, binaryOperatorParser(comparisonParser, assignmentOperatorParser));
			
			if (t &= parseConstant(t.end(), end, "[")) {
				t << assignmentParser(t.end(), end);
				t &= parseConstant(t.end(), end, "]");
			} else {
				return invalid();
			}
			
			return t;
		}
		
		Token parseGroup(StringIteratorT begin, StringIteratorT end) {
			Token t(begin);
			
			if (t += parseConstant(t.end(), end, "(")) {
				t &= parseExpression(begin, end);
				t &= parseConstant(t.end(), end, ")");
				
				return t;
			}
			
			return Token();
		}
		
		Token parseCellItem (StringIteratorT begin, StringIteratorT end);
		
		Token parseCell (StringIteratorT begin, StringIteratorT end) {
			Token t(begin, CELL);
			
			if (t &= parseConstant(t.end(), end, "(")) {
				while (true) {
					Token u = parseCellItem(t.end(), end);
					
					if (u)
						t << u;
					
					t += parseWhitespace(t.end(), end);
					
					if (!u)
						break;
				}
				
				t &= parseConstant(t.end(), end, ")");
				
				return t;
			}
			
			return Token();			
		}
		
		Token parseCellValue (StringIteratorT begin, StringIteratorT end) {
			Token t;
			
			if (t = parseCell(begin, end)) {
				return t;
			}
			
			if (t = parseExpression(begin, end)) {
				return t;
			}
			
			if (t = parseValue(begin, end)) {
				return t;
			}
			
			return t;
		}
		
		Token parseCellItem (StringIteratorT begin, StringIteratorT end) {
			Token t;
			
			if (t = parseConstant(begin, end, "'")) {
				t.setIdentity(VALUE);
				t << parseCellValue(t.end(), end);
				
				return t;
			}
			
			return parseCellValue(begin, end);
		}
		
		Value * convertToCells (Token & t);
		
		Cell * convertToCells (Token::ChildrenT & children) {
			if (children.size() == 0) {
				return NULL;
			}

			Cell * first = NULL, * list = NULL;
			
			for (unsigned i = 0; i < children.size(); i += 1) {
				list = Cell::append(list, convertToCells(children[i]), first);
			}
			
			return first;
		}
		
		Value * convertToCells (Token & t) {
			switch (t.identity()) {
				case CELL: {
					return convertToCells(t.children());
				}
				
				case EXPRESSION: {
					return convertToCells(t.children()[0]);
				}
				
				case BINARY_OPERATOR: {
					Cell * first = NULL, * list = NULL;
					
					list = Cell::append(list, new Symbol("with"), first);
					for (unsigned i = 0; i < t.children().size(); i += 1) {
						list = Cell::append(list, convertToCells(t.children()[i]), first);
					}
					
					return first;
				}
				
				case BINARY_OPERATOR_FUNCTION: {
					Cell * first = NULL, * list = NULL;
					
					list = Cell::append(list, new Symbol(t.children()[0].value()), first);
					list = Cell::append(list, convertToCells(t.children()[1]), first);
					
					return first;
				}
				
				case NUMBER:
				case DECIMAL: {
					return new Integer(convert<int>(t.value()));
				}
				
				case STRING: {
					return new String(t.value());
				}
				
				case SYMBOL: {
					return new Symbol(t.value());
				}
				
				case VALUE: {
					return new Cell(new Symbol("value"),convertToCells(t.children()));
				}
				
				default: {
					return convertToCells(t.children());
				}
			}
		}
		
		Value * parse (const SourceCode & code) {
			StringT::const_iterator begin = code.begin();
			StringT::const_iterator end = code.end();
					
			try {
				// Parse the source code
				Token t = Parser::parseCellItem(begin, end);				
				
				if (!t) {
					throw FatalParseFailure(Token(begin, end), "Could not parse source code");
				}
				
				if (t.end() != end) {
					throw FatalParseFailure(Token(t.end(), end), "Could not parse entire source code!");
				}
				
				return convertToCells(t);
			} catch (FatalParseFailure & failure) {
				// Print error message
				failure.printError(std::cerr, code);				
				throw failure;
			}
		}
	}
}
