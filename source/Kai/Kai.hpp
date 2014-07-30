//
//  Kai.h
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 24/06/09.
//  Copyright 2009 Samuel Williams. All rights reserved.
//
//

#ifndef _KAI_H
#define _KAI_H

#include <cstddef>
#include <string>
#include <sstream>

#include <iostream>
#include <vector>
#include <stdint.h>

namespace Kai {
	
	// Immutability is an important trait of a programmable system, because it provides a defined set of axioms on which further abstractions can be built.
	
	// Fundamentally, some parts of a given system must be immutable. It is whether these are well defined, or unspecified, which dictates the reliability of a given system.
	
	// Kai has a set of fundamental data types which are immutable, but can be extended within a given execution context namespace. Functionality is tied to objects using the object's identity. This way, an object represents state but functionality is dependent on the context. Objects don't carry functionality explicitly with them by default, but instead the identity is used to look up a prototype which provides functionality.
	
	typedef std::string StringT;
	typedef std::size_t SizeT;
	typedef std::stringstream StringStreamT;
	typedef StringT::const_iterator StringIteratorT;
	
	class Time {
	public:
		typedef uint64_t SecondsT;
		typedef double FractionT;
		
	protected:
		SecondsT _seconds;
		FractionT _fraction;
		
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
		
		const SecondsT & seconds() const { return _seconds; }
		const FractionT & fraction() const { return _fraction; }
		const FractionT total() const { return (FractionT)_seconds + _fraction; }
	};
	
	std::ostream & operator<<(std::ostream & output, const Time & time);
}

#endif

