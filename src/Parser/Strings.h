/*
 *  Strings.h
 *  Kai
 *
 *  Created by Samuel Williams on 17/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
 
#include "Token.h"

namespace Kai {
	namespace Parser {
		StringT unescapeString (const StringT &);
		StringT escapeString (const StringT &);
	}
}