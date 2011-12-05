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
#include <map>

// #define KAI_TRACE
#define KAI_DEBUG

namespace Kai {
	
	class Value;
	class Symbol;
	class Cell;

	class Tracer : public Value {
		protected:
			struct Statistics {
				Statistics();
				
				std::vector<Time> frames;
				//std::set<Value*> children;
				
				//TimeT selfTime;
				Time totalTime;
				uint64_t count;
			};
			
			typedef std::map<Value*, Statistics> StatisticsMapT;
			StatisticsMapT m_statistics;
			
		public:
			void enter(Value* value);
			void exit(Value* value);
			
			void dump(std::ostream & buffer);

			static Value * dump(Frame * frame);
			
			virtual Value * prototype ();
			static Value * globalPrototype ();
			static void import (Table * context);
			
			static Tracer * globalTracer ();
	};
	
	/** The Frame class represents the stack of a running program, and is dynamically allocated.
	
	*/
	class Frame : public gc_cleanup {
		protected:
			/// Previous stack frame
			Frame * m_previous;
			
			/// The scope of the stack frame, if any.
			Value * m_scope;
			
			/// The original message which created this frame, if any.
			Cell * m_message;
			
			/// The evaluated function.
			Value * m_function;
			/// The unwrapped arguments.
			Cell * m_arguments;
			
			/// Given a stack frame, apply the function to the arguments.
			Value * apply ();
			
			/// For debugging - the depth of the stack.
			unsigned m_depth;

		public:
			/// Create a root level stack frame with a given scope.
			Frame (Value * scope);
			/// Create an intermediate stack frame with a given scope and previous frame.
			Frame (Value * scope, Frame * previous);
			/// Create an intermediate stack frame as above, but with a given message.
			Frame (Value * scope, Cell * message, Frame * previous);
			
			virtual ~Frame ();
			
			/// Lookup an identifier using the stack, starting at this frame.
			Value * lookup (Symbol * identifier);
			/// Lookup an identifier as above, but return the frame which defines the value.
			Value * lookup (Symbol * identifier, Frame *& frame);
			
			template <typename ValueT>
			ValueT * lookupAs (Symbol * identifier) {
				return dynamic_cast<ValueT*>(lookup(identifier));
			}
			
			// Evaluate a given message in the specified scope.
			Value * call (Value * scope, Cell * message);
			Value * call (Cell * message) { return call(NULL, message); }
			
			/// Return the previous stack frame.
			Frame * previous ();
			
			/// This function searches up the stack for the current scope.
			Value * scope ();

			/// Return the message (m o1 o2 o3) if it is defined.
			Cell * message ();
			
			/// Return the defined function (m), if it is known.
			Value * function ();
			/// Return the operands (o1 o2 o3) if they are given.
			Cell * operands ();
			/// Evaluate the operands in the current stack frame.
			Cell * unwrap ();
			/// Return the arguments if they have been evaluated.
			Cell * arguments ();
			
			Cell::ArgumentExtractor extract (bool evaluate = true);
			
			bool top ();
			void debug (bool ascend = true);

			static void import (Table * context);
			
			// Returns the frame which defines a given symbol
			//static Value * where (Frame * frame);
			
			// Attempt to update inplace a value in a frame
			static Value * update (Frame * frame);
			
			// Returns the caller of the current frame, similar to the "this" keyword.
			static Value * scope (Frame * frame);
			
			// Marks a trace point in the stack frame, and prints out the given unwrapped arguments.
			static Value * trace (Frame * frame);

			// Run the enclosed code and report the amount of time taken.
			static Value * benchmark (Frame * frame);
			
			// Returns the arguments evaluated in the caller's context.
			static Value * unwrap (Frame * frame);
			
			// Returns a function such that when evaluated, returns the arguments unevaluated.
			static Value * wrap (Frame * frame);
			
			// Processing
			static Value * with (Frame * frame);
	};
}

#endif
