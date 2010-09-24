/*
 *  Exception.h
 *  This file is part of the "Kai" project, and is licensed under the GNU GPLv3.
 *
 *  Created by Samuel Williams on 13/04/10.
 *  Copyright 2010 Samuel Williams. All rights reserved.
 *
 */

#ifndef _KEXCEPTION_H
#define _KEXCEPTION_H

#include <exception>
#include "Kai.h"

namespace Kai {

	class Cell;
	class Frame;
	class Value;

	class Exception {
		protected:
			StringT m_what;

			Value * m_value;
						
			Frame * m_frame;
			
		public:
			Exception(StringT what, Frame * frame);
			Exception(StringT what, Value * value, Frame * frame);
			
			Frame * top ();
			StringT what ();
	};

}

#endif
