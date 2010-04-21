/*
 *  Ensure.h
 *  This file is part of the "Kai" project, and is licensed under the GNU GPLv3.
 *
 *  Created by Samuel Williams on 1/03/10.
 *  Copyright 2010 Samuel Williams. All rights reserved.
 *
 */

#ifndef _KAI_ENSURE_H
#define _KAI_ENSURE_H

#include <exception>
#include <string>

#ifndef __GNUC__
#define ensure(e) ::Kai::InternalError::ensureHandler(e, #e, __PRETTY_FUNCTION__, __FILE__, __LINE__)
#else
#define ensure(e) ::Kai::InternalError::ensureHandler(e, #e, __func__, __FILE__, __LINE__)
#endif

namespace Kai {
	
	class InternalError : public std::exception {
		const char * m_function;
		const char * m_expression;
		const char * m_file;

		unsigned m_line;

		std::string m_what;
	public:
		InternalError (const char * expression, const char * func, const char * file, unsigned line) throw ();
		virtual ~InternalError () throw ();

		/// A descriptive string relating to the assertion that failed.
		virtual const char * what () const throw ();

		/// The ensure() macro calls this function to handle throwing the actual exception.
		static void ensureHandler (bool condition, const char * expression, const char * func, const char * file, unsigned line);
	};

	
}

#endif
