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
		protected:
			// Previous stack frame
			Frame * m_previous;
			
			Value * m_scope;
						
			Cell * m_message;
			
			Value * m_function;
			Cell * m_arguments;
			
			Value * apply ();
			
			unsigned m_depth;

		public:
			Frame (Value * scope);
			Frame (Value * scope, Frame * previous);
			Frame (Value * scope, Cell * message, Frame * previous);
			
			Value * lookup (Symbol * identifier);
			
			template <typename ValueT>
			ValueT * lookupAs (Symbol * identifier) {
				return dynamic_cast<ValueT*>(lookup(identifier));
			}
			
			// Should a message be restricted to a Cell, or is it suitable to be a Value ?
			Value * call (Value * scope, Cell * message);
			
			Frame * previous ();
			// This function searches up the tree for the current scope.
			Value * scope ();
			Value * function ();
			
			Cell * message ();
			Cell * operands ();
			Cell * unwrap ();
			Cell * arguments ();
			
			Cell::ArgumentExtractor extract (bool evaluate = true);
			
			bool top ();
			void debug (bool ascend = true);

			static void import (Table * context);
			
			// Returns the caller of the current frame, similar to the "this" keyword.
			static Value * scope (Frame * frame);
			
			// Marks a trace point in the stack frame, and prints out the given unwrapped arguments.
			static Value * trace (Frame * frame);
			
			// Returns the arguments evaluated in the caller's context.
			static Value * unwrap (Frame * frame);
			
			// Returns a function such that when evaluated, returns the arguments unevaluated.
			static Value * wrap (Frame * frame);
			
			// Processing
			static Value * with (Frame * frame);
	};
}

#endif
