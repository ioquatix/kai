/*
 *  Compiler.h
 *  Kai
 *
 *  Created by Samuel Williams on 11/07/10.
 *  Copyright 2010 Samuel Williams. All rights reserved.
 *
 */
 
#include "Value.h"

#include <llvm/LLVMContext.h>

namespace Kai {

	class Compiler : public Value {
		public:
			Compiler ();
			virtual ~Compiler ();
			
			// Returns a compiled function corresponding to the given arguments.
			virtual Value * evaluate (Frame * frame);
			
			// Lookup a compiled function with a given name
			virtual Value * lookup (Symbol * identifier);
			
			// A prototype specifies the behaviour of the current value.
			virtual Value * prototype ();
			
			virtual void toCode (StringStreamT & buffer);
	};

}

