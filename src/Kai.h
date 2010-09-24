/*
 *  Kai.h
 *  This file is part of the "Kai" project, and is licensed under the GNU GPLv3.
 *
 *  Created by Samuel Williams on 24/06/09.
 *  Copyright 2009 Samuel Williams. All rights reserved.
 *
 */

#ifndef _KAI_H
#define _KAI_H


#include <cstddef>
#include <string>
#include <sstream>

#include <iostream>
#include <vector>

#include <gc/gc_cpp.h>
#include <gc/gc_allocator.h>

#define abstract = 0

namespace Kai {

	typedef std::string StringT;
	typedef std::size_t SizeT;
	typedef std::stringstream StringStreamT;
	typedef StringT::const_iterator StringIteratorT;

	class Time {
		public:
			typedef uint64_t SecondsT;
			typedef double FractionT;
			
		protected:
			SecondsT m_seconds;
			FractionT m_fraction;
			
			void normalize();
			
		public:
			Time();
			Time(FractionT time);
			
			Time operator+(const Time & other);
			Time operator-(const Time & other);
			
			Time & operator+=(const Time & other);
			Time & operator-=(const Time & other);
			
			Time operator*(FractionT other);
			Time operator/(FractionT other);
			
			const SecondsT & seconds() const { return m_seconds; }
			const FractionT & fraction() const { return m_fraction; }
			const FractionT total() const { return (FractionT)m_seconds + m_fraction; }
	};
	
	std::ostream & operator<<(std::ostream & output, const Time & time);
}

#endif

