//
//  Kai.cpp
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 24/06/09.
//  Copyright 2009 Samuel Williams. All rights reserved.
//
//

#include "Kai.hpp"

#include <ctime>
#include <sys/time.h>
#include <cmath>
#include "Ensure.hpp"

namespace Kai {
	
	bool close_enough (double a, double b) {
		double d = a - b;
		if (d < 0) d *= -1;
		
		if (d < 0.001)
			return true;
		else
			return false;
	}
	
	Time::Time()
	{
		struct timeval time_value;
		gettimeofday (&time_value, (struct timezone*)0);
		
		_seconds = time_value.tv_sec;
		_fraction = (FractionT)time_value.tv_usec / 1000000.0;
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
		Time time(*this);
		
		time._seconds += other._seconds;
		time._fraction += other._fraction;
		
		time.normalize();
		
		KAI_ENSURE(close_enough(this->total() + other.total(), time.total()));
		
		return time;
	}
	
	Time Time::operator-(const Time & other)
	{
		Time time(*this);
		
		time._seconds -= other._seconds;
		time._fraction -= other._fraction;
		
		time.normalize();
		
		KAI_ENSURE(close_enough(this->total() - other.total(), time.total()));
		
		return time;
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
		Time time(*this);
		
		time._seconds *= other;
		time._fraction *= other;
		
		time.normalize();
		
		KAI_ENSURE(close_enough(this->total() * other, time.total()));
		
		return time;
	}
	
	Time Time::operator/(FractionT other)
	{
		Time time(*this);
		
		FractionT current = this->total();
		time._seconds = 0;
		time._fraction = current / other;
		
		time.normalize();
		
		KAI_ENSURE(close_enough(this->total() / other, time.total()));
		
		return time;
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
