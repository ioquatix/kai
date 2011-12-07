//
//  Unicode.h
//  Kai
//
//  Created by Samuel Williams on 7/12/11.
//  Copyright (c) 2011 Orion Transfer Ltd. All rights reserved.
//

#ifndef _KAI_STRING_UNICODE_H
#define _KAI_STRING_UNICODE_H

#include <cstdint>
#include "utf8.h"

namespace Kai {
	namespace Unicode {
		using namespace utf8;
		
		typedef std::uint32_t CodePointT;
		typedef bool (*PredicateFn)(CodePointT);
		
		bool isLetter(CodePointT);
		
		bool isAlpha (CodePointT);
		bool isNumeric (CodePointT);
		bool isAlphaNumeric (CodePointT i);
		
		bool isTab (CodePointT i);
		bool isSpace (CodePointT i);
		bool isWhitespace (CodePointT i);
		bool isNewline (CodePointT i);
		bool isNotNewline (CodePointT i);
	};
}

#endif
