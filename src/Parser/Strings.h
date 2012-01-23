//
//  Strings.h
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 17/07/10.
//  Copyright 2010 Orion Transfer Ltd. All rights reserved.
//
//
 
#include "Token.h"

namespace Kai {
	namespace Parser {
		StringT unescape_string (const StringT &);
		StringT escape_string (const StringT &);
	}
}
