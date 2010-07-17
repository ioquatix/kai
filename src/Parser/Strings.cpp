/*
 *  Strings.cpp
 *  Kai
 *
 *  Created by Samuel Williams on 17/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "Strings.h"

namespace Kai {
	namespace Parser {
	
		StringT unescapeString (const StringT & value) {
			StringStreamT buffer;
			
			StringT::const_iterator i = value.begin(), end = value.end();
			
			// Skip enclosing quotes
			++i;
			--end;
			
			for (; i != end; ++i) {
				if (*i == '\\') {
					++i;
					
					switch (*i) {
						case 't':
							buffer << '\t';
							continue;
						case 'r':
							buffer << '\r';
							continue;
						case 'n':
							buffer << '\n';
							continue;
						case '\\':
							buffer << '\\';
							continue;
						case '.':
							continue;
					}
					
					Token t = parseCharacters(i, end, &isNumeric);
					
					if (t.length() > 0) {
						buffer << convert<StringT::value_type>(t.value());
						i = t.end();
					} else {
						buffer << '\\' << *i;
					}
				} else {
					buffer << *i;
				}
			}
		
			return buffer.str();
		}
		
		StringT escapeString (const StringT & value) {
			return value;
		}
	
	}
}