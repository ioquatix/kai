//
//  Exception.cpp
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 13/04/10.
//  Copyright 2010 Samuel Williams. All rights reserved.
//
//

#include "Exception.h"
#include "Object.h"
#include "Symbol.h"
#include "Frame.h"

namespace Kai {
	
	Exception::Exception(StringT what, Frame * frame) : _what(what), _object(NULL), _frame(frame) {
		
	}
	
	Exception::Exception(StringT what, Object * object, Frame * frame) : _what(what), _object(object), _frame(frame) {
		
	}
	
	Exception::~Exception() {
		
	}
	
	Frame * Exception::top () {
		return _frame;
	}
	
	StringT Exception::what () {
		if (_object)
			return name() + " : " + _what + " : " + Object::to_string(_frame, _object);
		else
			return name() + " : " + _what;
	}
	
	StringT Exception::name() {
		return "Exception";
	}
	
#pragma mark -
	
	ArgumentError::ArgumentError(StringT name, StringT type, Object * value, Frame * frame) : Exception("Error converting argument", value, frame), _name(name), _type(type) {
		
	}
	
	StringT ArgumentError::what() {
		if (_object) {
			return name() + " : Expecting " + _name + " of type " + _type + ", got " + _object->identity(_frame)->value() + "!";
		} else {
			return name() + " : Expecting " + _name + " of type " + _type + ", got nil!";
		}
	}
	
	StringT ArgumentError::name() {
		return "Argument Error";
	}
	
#pragma mark -
	
	RangeError::RangeError(StringT what, Object * value, Frame * frame) : Exception(what, value, frame) {
		
	}
	
	StringT RangeError::name() {
		return "Range Error";
	}
}
