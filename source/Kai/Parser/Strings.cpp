//
//  Strings.cpp
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 17/07/10.
//  Copyright 2010 Orion Transfer Ltd. All rights reserved.
//
//

#include "Strings.hpp"
#include "../Math/Integer.hpp"
#include <stdexcept>

namespace Kai {
	namespace Parser {
	
		StringT unescape_string (const StringT & value) {
			StringStreamT buffer;
			
			StringT::const_iterator i = value.begin(), end = value.end();
			
			// Skip enclosing quotes
			++i;
			--end;
			
			for (; i < end; ++i) {
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
						case '"':
							buffer << '"';
							continue;
						case '\'':
							buffer << '\'';
							continue;
						case 'x':
							if ((end - i) >= 2) {
								StringT::value_type value = Math::convert_to_digit(*(++i)) << 4;
								value |= Math::convert_to_digit(*(++i));
								buffer << (StringT::value_type)value;
								continue;
							} else {
								break;
							}
						case '.':
							continue;
					}
					
					throw std::runtime_error("Could not parse string escape!");
				} else {
					buffer << *i;
				}
			}
		
			return buffer.str();
		}
		
		StringT escape_string (const StringT & value) {
			StringStreamT buffer;
			
			StringT::const_iterator i = value.begin(), end = value.end();
			buffer << '"';
			
			for (; i != end; ++i) {
				if (*i == '"') {
					buffer << "\\\"";
				} else {
					buffer << *i;
				}
			}
			
			buffer << '"';
			return buffer.str();
		}
	
	}
}
