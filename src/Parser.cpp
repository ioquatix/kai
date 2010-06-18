
#include "Parser.h"
#include <assert.h>

namespace Kai {
	Cell * Parser::parse (const StringT & code) {
		const char * begin = code.data();
		const char * end = code.data() + code.size();
		
		Cell * value = Parser::parse(begin, end);
		
		// We parsed everything
		assert(begin == end);
		
		return value;
	}

	template <typename PrimitiveT>
	PrimitiveT convert (const StringT& stringValue) {
		StringStreamT stream(stringValue);
		
		PrimitiveT primitiveValue;
		stream >> primitiveValue;
		
		return primitiveValue;
	}

	int isWhitespace (const char * c) {
		return *c == ' ' || *c == '\r' || *c == '\n' || *c == '\t';
	}

	int isBrackets (const char * c) {
		return *c == '(' || *c == ')';
	}

	int isNumeric (const char * c) {
		return *c >= '0' && *c <= '9';
	}

	Cell * Parser::parse(const char *& begin, const char * end) {
		Cell * list = NULL;
		
		while (begin < end) {
			while (isWhitespace(begin)) {
				++begin;
			}
			
			Value * value = NULL;
			
			if (*begin == '(') {
				value = parse(++begin, end);
			} else if (*begin == ')') {
				++begin;
				break;
			} else if (*begin == '"') {
				value = parseString(begin, end);
			} else if (isNumeric(begin) || *begin == '-') {
				value = parseInteger(begin, end);
			} else if (*begin == '#') {
				while (*begin != '\n' && begin < end) {
					++begin;
				}
				
				continue;
			} else {
				value = parseSymbol(begin, end);
			}
			
			// value != NULL -> empty list
			// assert(value != NULL);
			
			//std::cout << "Appending: ";
			//value->debug();
			
			if (list)
				list->append(value);
			else
				list = new Cell(value);
			
		}
		
		return list;
	}

	String * Parser::parseString(const char *& begin, const char * end) {
		assert(*begin == '"');
		
		StringStreamT characters;
		
		++begin;
		
		while (begin != end && *begin != '"') {
			if (*begin == '\\') {
				++begin;
				
				switch (*begin) {
					case 'r': characters << '\r'; break;
					case 'n': characters << '\n'; break;
					case 't': characters << '\t'; break;
					case '0': characters << '\0'; break;
				}
			}
			
			characters << *begin;
			
			++begin;
		}

		// Move to next character
		++begin;

		return new String(characters.str());
	}

	Integer * Parser::parseInteger(const char *& begin, const char * end) {
		const char * start = begin;
		
		if (*begin == '-')
			++begin;
		
		while (begin < end && isNumeric(begin))
			++begin;
		
		int value = convert<int>(StringT(start, begin));
		
		return new Integer(value);
	}

	Symbol * Parser::parseSymbol(const char *& begin, const char * end) {
		const char * start = begin;
		
		while (begin < end && !isWhitespace(begin) && !isBrackets(begin)) {
			++begin;
		}
		
		return new Symbol(StringT(start, begin));
	}
}
