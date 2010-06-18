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

#include "Value.h"

namespace Kai {
	
	class Value;
	class Symbol;
	class Cell;

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
			
			Value * lookup (Symbol * identifier);
			
			Value * call (Cell * functionAndOperands);
			Value * call (Symbol * function, Cell * operands = NULL);
			Value * call (Value * caller, Value * target, Symbol * function, Cell * operands);
			
			Frame * previous ();
			Value * caller ();
			Symbol * function ();
			
			Cell * operands ();
			Cell * unwrap ();
			Cell * arguments ();
			
			Cell::ArgumentExtractor extract();
			
			bool top ();
			void debug ();

			static void import (Table * context);
			
			// Returns the caller of the current frame, similar to the "this" keyword.
			static Value * caller (Frame * frame);
			
			// Marks a trace point in the stack frame, and prints out the given unwrapped arguments.
			static Value * trace (Frame * frame);
			
			// Returns the arguments evaluated in the caller's context.
			static Value * unwrap (Frame * frame);
			
			// Returns a function such that when evaluated, returns the arguments unevaluated.
			static Value * wrap (Frame * frame);
	};
}

#endif
