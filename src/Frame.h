/*
 *  Frame.h
 *  This file is part of the "Kai" project, and is licensed under the GNU GPLv3.
 *
 *  Created by Samuel Williams on 12/04/10.
 *  Copyright 2010 Samuel Williams. All rights reserved.
 *
 */

#ifndef _KAI_FRAME_H
#define _KAI_FRAME_H

#include "Object.h"
#include <map>

// #define KAI_TRACE
// #define KAI_DEBUG

namespace Kai {
	
	class Cell;
	class ArgumentExtractor;
	
	class Tracer : public Object {
	protected:
		struct Statistics {
			Statistics();
			
			std::vector<Time> frames;
			//std::set<Value*> children;
			
			//TimeT selfTime;
			Time total_time;
			uint64_t count;
		};
		
		typedef std::map<Object *, Statistics> StatisticsMapT;
		StatisticsMapT _statistics;
		
	public:
		virtual ~Tracer();
		
		virtual Ref<Symbol> identity(Frame * frame) const;
		
		virtual void mark(Memory::Traversal *) const;
		
		void enter(Object* value);
		void exit(Object* value);
		
		void dump(Frame * frame, std::ostream & buffer);
		
		static Ref<Object> dump(Frame * frame);
		
		static void import(Frame * frame);
	};
	
	/** The Frame class represents the stack of a running program, and is dynamically allocated.
	 
	 */
	class Frame : public Object {
	protected:
		/// Previous stack frame
		Frame * _previous;
		
		/// The scope of the stack frame, if any.
		Object * _scope;
		
		/// The original message which created this frame, if any.
		Cell * _message;
		
		/// The evaluated function.
		Object * _function;
		
		/// The unwrapped arguments.
		Cell * _arguments;
		
		/// For debugging - the depth of the stack.
		unsigned _depth;
		
		/// Given a stack frame, apply the function to the arguments.
		Ref<Object> apply();
		
	public:
		/// Create a root level stack frame with a given scope.
		Frame(Object * scope);
		/// Create an intermediate stack frame with a given scope and previous frame.
		Frame(Object * scope, Frame * previous);
		/// Create an intermediate stack frame as above, but with a given message.
		Frame(Object * scope, Cell * message, Frame * previous);
		
		virtual ~Frame();
		
		virtual Ref<Symbol> identity(Frame * frame) const;
		
		virtual void mark(Memory::Traversal * traversal) const;
		
		/// Attempt to update a named variable with the given value.
		virtual Ref<Object> update(Symbol * identifier, Object * value, bool local = false);
		
		/// Lookup an identifier using the stack, starting at this frame.
		virtual Ref<Object> lookup(Symbol * identifier);
		
		/// Lookup an identifier as above, but return the frame which defines the value.
		Ref<Object> lookup(Symbol * identifier, Frame *& frame);
		
		// Evaluate a given message in the specified scope.
		Ref<Object> call(Object * scope, Cell * message);
		Ref<Object> call(Cell * message) {
			return call(NULL, message);
		}
		
		/// Return the previous stack frame.
		Frame * previous();
		
		/// This function searches up the stack for the current scope.
		Object * scope();
		
		/// Return the message (m o1 o2 o3) if it is defined.
		Cell * message();
		
		/// Return the defined function (m), if it is known.
		Ref<Object> function();
		
		/// Return the operands (o1 o2 o3) if they are given.
		Cell * operands();
		
		/// Evaluate the operands in the current stack frame.
		Cell * unwrap();
		
		/// Return the arguments if they have been evaluated.
		Cell * arguments();
		
		ArgumentExtractor extract(bool evaluate = true);
		
		bool top();
		void debug(bool ascend = true);
		
		// Returns the frame which defines a given frame->symbol
		static Ref<Object> where (Frame * frame);
		
		// Attempt to update inplace a value in a frame
		static Ref<Object> update (Frame * frame);
		
		// Returns the caller of the current frame, similar to the "this" keyword.
		static Ref<Object> scope (Frame * frame);
		
		// Marks a trace point in the stack frame, and prints out the given unwrapped arguments.
		static Ref<Object> trace (Frame * frame);
		
		// Run the enclosed code and report the amount of time taken.
		static Ref<Object> benchmark (Frame * frame);
		
		// Returns the arguments evaluated in the caller's context.
		static Ref<Object> unwrap (Frame * frame);
		
		// Returns a function such that when evaluated, returns the arguments unevaluated.
		static Ref<Object> wrap (Frame * frame);
		
		// Processing chains of expressions
		static Ref<Object> with (Frame * frame);
		
		static Ref<Object> operands(Frame * frame);
		static Ref<Object> arguments(Frame * frame);
				
		static void import (Frame * frame);
		
		/// Convenience functions for allocation:
		Symbol * sym(const char * name);
		
		//Symbol * builtin_function();
	};
}

#endif
