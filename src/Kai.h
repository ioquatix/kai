/*
 *  Kai.h
 *  This file is part of the "Kai" project, and is licensed under the GNU GPLv3.
 *
 *  Created by Samuel Williams on 24/06/09.
 *  Copyright 2009 Samuel Williams. All rights reserved.
 *
 */

#ifndef _KAI_H
#define _KAI_H


#include <cstddef>
#include <string>
#include <sstream>

#include <iostream>
#include <vector>

#include <gc/gc_cpp.h>

#define abstract = 0

namespace Kai {

	typedef std::string StringT;
	typedef std::size_t SizeT;
	typedef std::stringstream StringStreamT;
	typedef StringT::value_type Character;
	typedef StringT::const_iterator Iterator;

	typedef double TimeT;
	
	TimeT systemTime ();
}

#endif

