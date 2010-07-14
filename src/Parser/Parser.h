/*
 *  Parser.h
 *  Kai
 *
 *  Created by Samuel Williams on 14/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
 
#include "Token.h"

namespace Kai {
	namespace Parser {
		
		// May throw FatalParseFailure
		Value * parse (const SourceCode & code);
		
	}
}