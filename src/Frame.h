/*
 *  Frame.h
 *  This file is part of the "Kai" project, and is licensed under the GNU GPLv3.
 *
 *  Created by Samuel Williams on 12/04/10.
 *  Copyright 2010 Samuel Williams. All rights reserved.
 *
 */

#ifndef _KFRAME_H
#define _KFRAME_H

#include "Cell.h"

namespace Kai {
	class Frame : public gc {
		private:
			// Previous stack frame
			Frame * m_previous;
			
			// Typically an instance of Table
			Value * m_caller;
			
			Symbol * m_function;
			
			Cell * m_operands;
			Cell * m_arguments;

		public:
			Frame (Value * caller);
			Frame (Value * caller, Symbol * function, Cell * operands, Frame * previous);
			
			Value * call (Cell * functionAndOperands);
			Value * call (Symbol * function, Cell * operands = NULL);
			Value * call (Value * caller, Value * target, Symbol * function, Cell * operands);
			
			Frame * previous ();
			Value * caller ();
			Symbol * function ();
			
			Cell * operands ();
			Cell * unwrap ();
			Cell * arguments ();
			
			bool top ();
			
			void debug ();
	};
}

#endif
