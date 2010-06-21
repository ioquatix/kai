/*
 *  Exception.cpp
 *  This file is part of the "Kai" project, and is licensed under the GNU GPLv3.
 *
 *  Created by Samuel Williams on 13/04/10.
 *  Copyright 2010 Samuel Williams. All rights reserved.
 *
 */

#include "Exception.h"
#include "Frame.h"

namespace Kai {

	Exception::Exception(StringT what, Frame * frame)
		: m_what(what), m_value(NULL), m_frame(frame)
	{

	}
	
	Exception::Exception(StringT what, Value * value, Frame * frame) 
		: m_what(what), m_value(value), m_frame(frame)
	{
	
	}

	Frame * Exception::top ()
	{
		return m_frame;
	}

	StringT Exception::what ()
	{
		if (m_value)
			return m_what + " : " + Value::toString(m_value);
		else
			return m_what;
	}

}
