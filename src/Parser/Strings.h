/*
 *  Strings.h
 *  Kai
 *
 *  Created by Samuel Williams on 17/07/10.
 *  Copyright 2010 Orion Transfer Ltd. All rights reserved.
 *
 */
 
#include "Token.h"

namespace Kai {
	namespace Parser {
		StringT unescape_string (const StringT &);
		StringT escape_string (const StringT &);
	}
}
