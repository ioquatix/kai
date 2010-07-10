/*
 *  Kai.cpp
 *  This file is part of the "Kai" project, and is licensed under the GNU GPLv3.
 *
 *  Created by Samuel Williams on 24/06/09.
 *  Copyright 2009 Samuel Williams. All rights reserved.
 *
 */

#include "Kai.h"

#include <ctime>
#include <sys/time.h>

namespace Kai {
	TimeT systemTime () {
		struct timeval t;
		gettimeofday (&t, (struct timezone*)0);
		return ((TimeT)t.tv_sec) + ((TimeT)t.tv_usec / 1000000.0);
	}
}