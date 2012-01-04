/*
 *  Kai.cpp
 *  This file is part of the "Kai" project, and is licensed under the GNU GPLv3.
 *
 *  Created by Samuel Williams on 24/06/09.
 *  Copyright 2009 Samuel Williams. All rights reserved.
 *
 */

#include "Kai.h"

#include <ctime>
#include <sys/time.h>
#include <cmath>
#include <cassert>

namespace Kai {
	
	bool closeEnough (double a, double b) {
		double d = a - b;
		if (d < 0) d *= -1;
		
		if (d < 0.001)
			return true;
		else
			return false;
	}
	
	Time::Time()
	{
		struct timeval t;
		gettimeofday (&t, (struct timezone*)0);
		
		_seconds = t.tv_sec;
		_fraction = (FractionT)t.tv_usec / 1000000.0;
	}
	
	Time::Time(FractionT time)
	{
		_seconds = 0;
		_fraction = time;
		
		normalize();
	}
	
	void Time::normalize()
	{
		if (_fraction < 0) {
			FractionT seconds = std::floor(_fraction);
			
			_fraction -= seconds;
			_seconds += seconds;
		} else if (_fraction > 1) {
			FractionT seconds = std::floor(_fraction);
			
			_fraction -= seconds;
			_seconds += seconds;
		}
	}
	
	Time Time::operator+(const Time & other)
	{
		Time t(*this);
		
		t._seconds += other._seconds;
		t._fraction += other._fraction;
		
		t.normalize();
		
		assert(closeEnough(this->total() + other.total(), t.total()));
		
		return t;
	}
	
	Time Time::operator-(const Time & other)
	{
		Time t(*this);
		
		t._seconds -= other._seconds;
		t._fraction -= other._fraction;
		
		t.normalize();
		
		assert(closeEnough(this->total() - other.total(), t.total()));
		
		return t;
	}
	
	Time & Time::operator+=(const Time & other)
	{
		_seconds += other._seconds;
		_fraction += other._fraction;
		
		normalize();
		
		return *this;
	}
	
	Time & Time::operator-=(const Time & other)
	{
		_seconds -= other._seconds;
		_fraction -= other._fraction;
		
		normalize();
	
		return *this;
	}
	
	Time Time::operator*(FractionT other)
	{
		Time t(*this);
		
		t._seconds *= other;
		t._fraction *= other;
		
		t.normalize();
		
		assert(closeEnough(this->total() * other, t.total()));
		
		return t;
	}
	
	Time Time::operator/(FractionT other)
	{
		Time t(*this);
		
		FractionT current = this->total();
		t._seconds = 0;
		t._fraction = current / other;
		
		t.normalize();
		
		assert(closeEnough(this->total() / other, t.total()));
		
		return t;
	}
	
	std::ostream & operator<<(std::ostream & output, const Time & time)
	{
		if (time.seconds()) {
			output << time.total() << "s";
		} else {
			const char * postfix[] = {"s", "ms", "µs", "ns"};
			Time::FractionT f = time.fraction();
			std::size_t k = 0;
			
			while (f < 1.0 && k < 3) {
				k++;
				f *= 1000.0;
			}
			
			output << f << postfix[k];
		}
		
		return output;
	}
}
