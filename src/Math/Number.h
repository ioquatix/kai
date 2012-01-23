//
//  Number.h
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 19/12/11.
//  Copyright (c) 2011 Orion Transfer Ltd. All rights reserved.
//

#ifndef Kai_Number_h
#define Kai_Number_h


#include "Integer.h"

namespace Kai {
	namespace Math {
		
		/* An arbitrary precision and size number with a fractional part.
		 */
		class Number {
		public:
			typedef int IntermediateT;
			
		protected:
			bool _negative;
			Integer _value;
			
			// Scale is (10 ^ _scale) in base 10.
			ScaleT _scale;
		
		public:
			Number(IntermediateT value = 0);
			Number(std::string value);
			Number(Integer value, unsigned scale, bool negative = false);
						
			int compare_with(const Number & other) const;
			
			Integer fractional_part(const Integer & base = 10);
			Integer whole_part(const Integer & base = 10);
			
			Number floor();
			Number ceil();
			Number absolute();

			void multiply(const Number & other);
			
			Number & operator*=(const Number & other) { this->multiply(other); return *this; }
			
			std::string to_string() const;
		};
		
	}
}

#endif
