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

	class Exception {
		protected:
			Frame * m_frame;
			Cell * m_arguments;
			
			StringT m_what;
		
		public:
			Exception(StringT message, Frame * frame);
			
			Frame * top ();
			const StringT & what ();
	};

}

#endif
