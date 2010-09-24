/*
 *  Ensure.cpp
 *  This file is part of the "Kai" project, and is licensed under the GNU GPLv3.
 *
 *  Created by Samuel Williams on 1/03/10.
 *  Copyright 2010 Samuel Williams. All rights reserved.
 *
 */

#include "Ensure.h"

#include "Kai.h"

namespace Kai {

	InternalError::InternalError (const char * expression, const char * func, const char * file, unsigned line) throw ()
		: m_function(func), m_expression (expression), m_file (file), m_line (line)
	{
		using namespace std;

		try
		{
			StringStreamT buffer;
			buffer << "Internal Error [" << m_file << ":" << m_line << "] [" << m_function << "]: " << m_expression;
			m_what = buffer.str();
		}
		catch (...)
		{
			m_what = "Undefined Internal Error";
		}
	}

	InternalError::~InternalError () throw ()
	{
	}

	const char * InternalError::what () const throw ()
	{
		return m_what.c_str();
	}

	void InternalError::ensureHandler (bool condition, const char * expression, const char * func, const char * file, unsigned line)
	{
		if (!condition)
		{
			throw InternalError(expression, func, file, line);
		}
	}
	
}
