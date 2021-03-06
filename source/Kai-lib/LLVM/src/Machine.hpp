//
//  Machine.h
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 12/07/10.
//  Copyright 2010 Samuel Williams. All rights reserved.
//
//

#include <llvm/Value.h>
#include "Frame.hpp"
#include "Value.hpp"

extern "C" {
	void test ();

	// These functions exist so we can get the compiled type information.
	Kai::Object * _valueType ();
	Kai::Frame * _frameType ();
	
	Kai::Object * debug (Kai::Frame * frame);
		
	//void kai_module_import (Kai::Frame * frame);
};
