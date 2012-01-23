//
//  Ensure.h
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 1/03/10.
//  Copyright 2010 Samuel Williams. All rights reserved.
//
//

#ifndef _KAI_ENSURE_H
#define _KAI_ENSURE_H

#include <exception>
#include <string>

#ifndef __GNUC__
#define KAI_ENSURE(e) ::Kai::InternalError::ensureHandler(e, #e, __PRETTY_FUNCTION__, __FILE__, __LINE__)
#else
#define KAI_ENSURE(e) ::Kai::InternalError::ensureHandler(e, #e, __func__, __FILE__, __LINE__)
#endif

namespace Kai {
	
	class InternalError : public std::exception {
		const char * _function;
		const char * _expression;
		const char * _file;

		unsigned _line;

		std::string _what;
	public:
		InternalError (const char * expression, const char * func, const char * file, unsigned line) throw ();
		virtual ~InternalError () throw ();

		/// A descriptive string relating to the assertion that failed.
		virtual const char * what () const throw ();

		/// The KAI_ENSURE() macro calls this function to handle throwing the actual exception.
		static void ensureHandler (bool condition, const char * expression, const char * func, const char * file, unsigned line);
	};
	
	
}

#endif
