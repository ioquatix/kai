/*
 *  Parser.h
 *  This file is part of the "Kai" project, and is licensed under the GNU GPLv3.
 *
 *  Created by Samuel Williams on 6/04/10.
 *  Copyright 2010 Samuel Williams. All rights reserved.
 *
 */

#ifndef _KPARSER_H
#define _KPARSER_H

#include "Value.h"
#include <iostream>

namespace Kai {

	class Parser {
		public:
			static Cell * parse (const StringT & code);
			static Cell * parse (const char *& begin, const char * end);

			static String * parseString(const char *& begin, const char * end);
			static Integer * parseInteger(const char *& begin, const char * end);
			static Symbol * parseSymbol(const char *& begin, const char * end);
	};

}

#endif