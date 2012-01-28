//
//  Number.cpp
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 19/12/11.
//  Copyright (c) 2011 Orion Transfer Ltd. All rights reserved.
//

#include "Number.h"
#include <sstream>

namespace Kai {
	namespace Math {
		bool is_negative(bool lhs, bool rhs) {
			if (lhs == rhs) {
				return false;
			} else {
				return true;
			}
		}
		
		Number::Number(IntermediateT value) : _scale(0)
		{
			if (value < 0) {
				_negative = true;
				_value = (value * -1);
			} else {
				_negative = false;
				_value = value;
			}
		}
		
		Number::Number(std::string value) : _scale(0)
		{
			std::stringstream buffer;
			
			std::size_t i = 0;
			
			// Is the number negative?
			if (value[i] == '-') {
				_negative = true;
				i += 1;
			} else {
				_negative = false;
			}
			
			// Remove the Number point and calculate the size of the fractional component:
			for (; i < value.size(); i += 1) {
				if (value[i] == '.')
					_scale = value.size() - (i + 1);
				else
					buffer << value[i];
			}
			
			// Update the value based on the whole integer number (base-10).
			_value = Integer(buffer.str(), 10);
		}
		
		Number::Number(Integer value, unsigned scale, bool negative) : _negative(negative), _value(value), _scale(scale)
		{
			
		}
		
		int Number::compare_with(const Number & other) const
		{
			if (_negative && !other._negative) {
				return Integer::LESSER;
			}
			
			if (!_negative && other._negative) {
				return Integer::GREATER;
			}
			
			// We need to normalize the integers based on the scale factor.
			
			// Broken =)
			return Integer::EQUAL;
		}
		
		Integer Number::fractional_part(const Integer & base) {
			if (_scale) {
				return _value.fractional_part(_scale, base);
			} else {
				return 0;
			}
		}
		
		Integer Number::whole_part(const Integer & base) {
			if (_scale) {
				Integer copy = _value;
				
				copy.subtract(fractional_part(base));
				
				return copy;
			} else {
				return _value;
			}
		}
		
		void Number::multiply(const Number & other)
		{
			_negative = is_negative(_negative, other._negative);
			_value.multiply(other._value);
			_scale += other._scale;
		}
		
		Number Number::floor() {
			std::cerr << "Unimplemented: " << __func__ << std::endl;
			// floor(n) = n - frac(n)
			return 0;
		}
		
		Number Number::ceil() {
			std::cerr << "Unimplemented: " << __func__ << std::endl;
			// ceil(n) = floor(n) + 1
			return 0;
		}
		
		Number Number::absolute() {
			std::cerr << "Unimplemented: " << __func__ << std::endl;
			return 0;
		}
		
		std::string Number::to_string() const
		{
			std::stringstream buffer;
			std::string string = _value.to_string(10);
			
			std::size_t fractional_offset = string.size() - _scale;
			
			if (_negative) {
				buffer << '-';
			}
			
			buffer.write(string.data(), fractional_offset);
			
			if (_scale) {
				buffer << '.';
				buffer.write(string.data() + fractional_offset, _scale);
			}
			
			return buffer.str();
		}
	}
}
