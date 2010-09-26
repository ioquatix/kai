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
		
		m_seconds = t.tv_sec;
		m_fraction = (FractionT)t.tv_usec / 1000000.0;
	}
	
	Time::Time(FractionT time)
	{
		m_seconds = 0;
		m_fraction = time;
		
		normalize();
	}
	
	void Time::normalize()
	{
		if (m_fraction < 0) {
			FractionT seconds = std::floor(m_fraction);
			
			m_fraction -= seconds;
			m_seconds += seconds;
		} else if (m_fraction > 1) {
			FractionT seconds = std::floor(m_fraction);
			
			m_fraction -= seconds;
			m_seconds += seconds;
		}
	}
	
	Time Time::operator+(const Time & other)
	{
		Time t(*this);
		
		t.m_seconds += other.m_seconds;
		t.m_fraction += other.m_fraction;
		
		t.normalize();
		
		assert(closeEnough(this->total() + other.total(), t.total()));
		
		return t;
	}
	
	Time Time::operator-(const Time & other)
	{
		Time t(*this);
		
		t.m_seconds -= other.m_seconds;
		t.m_fraction -= other.m_fraction;
		
		t.normalize();
		
		assert(closeEnough(this->total() - other.total(), t.total()));
		
		return t;
	}
	
	Time & Time::operator+=(const Time & other)
	{
		m_seconds += other.m_seconds;
		m_fraction += other.m_fraction;
		
		normalize();
		
		return *this;
	}
	
	Time & Time::operator-=(const Time & other)
	{
		m_seconds -= other.m_seconds;
		m_fraction -= other.m_fraction;
		
		normalize();
	
		return *this;
	}
	
	Time Time::operator*(FractionT other)
	{
		Time t(*this);
		
		t.m_seconds *= other;
		t.m_fraction *= other;
		
		t.normalize();
		
		assert(closeEnough(this->total() * other, t.total()));
		
		return t;
	}
	
	Time Time::operator/(FractionT other)
	{
		Time t(*this);
		
		FractionT current = this->total();
		t.m_seconds = 0;
		t.m_fraction = current / other;
		
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
			
			while (f < 0.01 && k < 3) {
				k++;
				f *= 1000.0;
			}
			
			output << f << postfix[k];
		}
		
		return output;
	}
}