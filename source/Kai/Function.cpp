//
//  Function.cpp
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 13/04/10.
//  Copyright 2010 Samuel Williams. All rights reserved.
//
//

#include "Function.hpp"
#include "Frame.hpp"
#include "Exception.hpp"
#include "Ensure.hpp"

#include <iostream>

namespace Kai {
	
// MARK: -
	
	const char * const DynamicFunction::NAME = "DynamicFunction";
	
	DynamicFunction::DynamicFunction (EvaluateFunctionT evaluate_function) : _evaluate_function(evaluate_function) {
	}
	
	DynamicFunction::~DynamicFunction () {
	}
	
	Ref<Object> DynamicFunction::evaluate (Frame * frame) {
		return _evaluate_function(frame);
	}
	
	void DynamicFunction::to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const {
		buffer << "(dynamic-function " << _evaluate_function << ")";
	}
}
