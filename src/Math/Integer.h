/*
 *  Integer.h
 *  RSA
 *
 *  Created by Samuel Williams on 7/08/10.
 *  Copyright 2010 Orion Transfer Ltd. All rights reserved.
 *
 */

#ifndef __MULTIPLE_INTEGER_H__
#define __MULTIPLE_INTEGER_H__

#include <vector>
#include <stdint.h>
#include <iostream>

namespace Kai {
	namespace Math {
		// Prints out a std::vector of values for debugging.
		template <typename AnyT>
		std::ostream & operator<<(std::ostream & output, const std::vector<AnyT> & v) {
			output << "{";
			bool first = true;
			
			for(unsigned i = 0; i < v.size(); i += 1) {
				if(!first) {
					output << ", ";
				} else {
					first = false;
				}
				
				output << v[i];
			}
			
			output << "}";
			
			return output;
		}
		
		struct BarrettReduction;
		
		class Integer {
		public:
			typedef uint32_t DigitT;
			typedef uint64_t IntermediateT;
			typedef std::vector<DigitT> ValueT;
			static const std::size_t DIGIT_BITS = sizeof(DigitT) * 8;
			static const IntermediateT B = (IntermediateT)1 << DIGIT_BITS;
			
			// Remove excessive trailing zeros
			void normalize();
			
			// For use with compare_with
			static const int LESSER = -1;
			static const int EQUAL = 0;
			static const int GREATER = 1;
			
			static char nibble_to_hex(DigitT);
			static char convert_to_character(DigitT);
			static DigitT convert_to_digit(char);
			static const char * prefix_for_base(std::size_t base);
			
		protected:
			static void convert_arbitrary_base(std::string value, const Integer & base, Integer & result);
			
			ValueT _value;
			
		public:
			Integer(IntermediateT value = 0);
			Integer(ValueT digits);
			Integer(std::string value, std::size_t base = 16);
			
			~Integer();
			
			void swap(Integer & other) { this->_value.swap(other._value); }
			
			// Packing and Unpacking
			Integer(const DigitT * data, std::size_t size);
			void unpack(DigitT * data, std::size_t size) const;
			
			int compare_with(const Integer & other) const;
			
			Integer & operator=(const Integer & other);
			
			bool operator<(const Integer & other) const;
			bool operator>(const Integer & other) const;
			bool operator<=(const Integer & other) const;
			bool operator>=(const Integer & other) const;
			
			bool is_zero() const;
			
			bool operator!=(const Integer & other) const;
			bool operator==(const Integer & other) const;
			
			const DigitT & operator[](std::size_t index) const { return _value.at(index); }
			DigitT & operator[](std::size_t index) { return _value.at(index); }
			
			std::size_t size() const { return _value.size(); }
			
			// Adds a given amount to the current integer.
			// a can be aliased for this.
			void add(const Integer & a);
			
			// a must be smaller than this.
			// a cannot be aliased for this - it doesn't make sense anyway :)
			void subtract(const Integer & a);
			
			// Convenience function
			void multiply(const Integer & other);
			
			// Convenience function
			void modulus(const Integer & m);
			
			// Cannot be aliased
			void set_product(const Integer & a, const Integer & b);
			
			// Returns false if the division had a remainder.
			bool set_fraction_slow(const Integer & numerator, const Integer & denominator, Integer & remainder);
			
			// Slightly faster implementation, calculates x = qy + r, saves q in this, r in x
			void set_fraction(const Integer & numerator, const Integer & denominator, Integer & remainder);			
			
		protected:
			// Remove leading zero digits
			void trim();
			
			void set_fraction(Integer & x, Integer y);
			
		public:
			//void set_power(Integer base, Integer exponent);
			void set_power(Integer base, Integer exponent);
			
			// Uses Barrett Reduction
			void set_power(Integer base, Integer exponent, const Integer & mod);
			void set_power(Integer base, Integer exponent, const BarrettReduction & r);
			
			// Shift left makes a number bigger.
			void shift_left(DigitT amount);
			
			// Shift right makes a number smaller.
			void shift_right(DigitT amount);
			
			void binary_and(const Integer & other);
			void binary_or(const Integer & other);
			void binary_not();
			
			// Returns a large random number. Length is in multiple of 32 bits.
			void generate_random_number(DigitT length);
			
			// Returns a large random number between min and max.
			void generate_random_number(Integer min, Integer max);
			
			// Returns the greatest common divisor of a and b.
			void calculate_greatest_common_divisor(Integer a, Integer b);
			
			//	Computes inv = u^(-1) mod v
			void calculate_inverse(Integer u, Integer v);
			
			// Generates a prime number
			void generate_prime(Integer min, Integer max);
			
			// Generates a prime number - length is in multiple of 32 bits.
			void generate_prime(DigitT length);
			
			// Use probabilistic methods to check for prime number
			bool is_probably_prime(int tests = 10) const;
			
			// Finds a prime less than the given max, or returns false.
			bool find_prime_less_than(Integer max);
			
			std::string to_hexadecimal(bool prefix = true) const;
			std::string to_string(const Integer & base, bool prefix = true) const;
			
			void debug();
			
			// Leaky Abstraction :p
			const ValueT & value() const { return _value; }
			
			Integer operator%(const Integer & base) {
				Integer result = *this;
				result.modulus(base);
				
				return result;
			}
			
			Integer & operator+=(const Integer & other) { add(other); return *this; }
			Integer & operator-=(const Integer & other) { subtract(other); return *this; }
			Integer & operator*=(const Integer & other) { multiply(other); return *this; }
			
			Integer & operator/=(const Integer & denominator) { 
				Integer numerator, remainder;
				this->swap(numerator);
				
				this->set_fraction(numerator, denominator, remainder);
				
				return *this;
			};
			
			int64_t to_int64() const {
				if(_value.size() == 0) {
					return 0;
				} else if(_value.size() == 1) {
					return _value[0];
				} else {
					
					throw std::domain_error("overflow converting integer to int64_t");
				}
			}
		};
		
		struct BarrettReduction {
			// The modulus we are calculating.
			Integer mod;
			Integer b, bk, mu;
			
			// Used for division by shifting.
			std::size_t bn, bp;
			
			// This one is used for doing fast modulus using binary and.
			// bkm = bkp - 1
			Integer bkp, bkm;
			
			BarrettReduction(const Integer & _mod);
			void modulus(Integer & x) const;
		};
		
		
		std::ostream & operator<<(std::ostream &, const Integer &);
	}
}

#endif

