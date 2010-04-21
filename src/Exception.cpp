/*
 *  Exception.cpp
 *  This file is part of the "Kai" project, and is licensed under the GNU GPLv3.
 *
 *  Created by Samuel Williams on 13/04/10.
 *  Copyright 2010 Samuel Williams. All rights reserved.
 *
 */

#include "Exception.h"

namespace Kai {

	Exception::Exception(StringT what, Frame * frame)
		: m_what(what), m_frame(frame)
	{

	}

	Frame * Exception::top ()
	{
		return m_frame;
	}

	const StringT & Exception::what ()
	{
		return m_what;
	}

}
