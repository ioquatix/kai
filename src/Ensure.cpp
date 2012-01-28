//
//  Ensure.cpp
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 1/03/10.
//  Copyright 2010 Samuel Williams. All rights reserved.
//
//

#include "Ensure.h"

#include "Kai.h"

namespace Kai {

	InternalError::InternalError(const char * expression, const char * func, const char * file, unsigned line) throw () : _function(func), _expression (expression), _file (file), _line (line) {
		try {
			StringStreamT buffer;
			buffer << "Internal Error [" << _file << ":" << _line << "] [" << _function << "]: " << _expression;
			_what = buffer.str();
		} catch (...) {
			_what = "Undefined Internal Error";
		}
	}

	InternalError::~InternalError() throw () {
	}

	const char * InternalError::what() const throw () {
		return _what.c_str();
	}

	void InternalError::ensure_handler(bool condition, const char * expression, const char * func, const char * file, unsigned line) {
		if (!condition) {
			throw InternalError(expression, func, file, line);
		}
	}
	
}
