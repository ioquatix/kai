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

	DynamicFunction::DynamicFunction (EvaluateFunctionT evaluateFunction)
		: m_evaluateFunction(evaluateFunction) {
		
	}
	
	DynamicFunction::~DynamicFunction () {
		
	}

	Ref<Value> DynamicFunction::evaluate (Frame * frame) {
		return m_evaluateFunction(frame);
	}

	void DynamicFunction::toCode(StringStreamT & buffer, MarkedT & marks, std::size_t indentation) {
		buffer << "(dynamic-function " << m_evaluateFunction << ")";
	}
}
