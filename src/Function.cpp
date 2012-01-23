/*
 *  Function.cpp
 *  This file is part of the "Kai" project, and is licensed under the GNU GPLv3.
 *
 *  Created by Samuel Williams on 13/04/10.
 *  Copyright 2010 Samuel Williams. All rights reserved.
 *
 */

#include "Function.h"
#include "Frame.h"
#include "Exception.h"
#include "Ensure.h"

#include <iostream>

namespace Kai {

#pragma mark -

	const char * const DynamicFunction::NAME = "DynamicFunction";
	
	DynamicFunction::DynamicFunction (EvaluateFunctionT evaluateFunction) : _evaluateFunction(evaluateFunction) {
	}
	
	DynamicFunction::~DynamicFunction () {
	}

	Ref<Object> DynamicFunction::evaluate (Frame * frame) {
		return _evaluateFunction(frame);
	}

	void DynamicFunction::to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) {
		buffer << "(dynamic-function " << _evaluateFunction << ")";
	}
}
