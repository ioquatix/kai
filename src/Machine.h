/*
 *  Machine.h
 *  Kai
 *
 *  Created by Samuel Williams on 12/07/10.
 *  Copyright 2010 Samuel Williams. All rights reserved.
 *
 */

#include <llvm/Value.h>
#include "Frame.h"
#include "Value.h"

extern "C" {
	void test ();
	
	Kai::Value * debug (Kai::Frame * frame);
		
	//void kai_module_import (Kai::Frame * frame);
};
