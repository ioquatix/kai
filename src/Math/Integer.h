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
		std::ostream & operator<< (std::ostream & output, const std::vector<AnyT> & v) {
			output << "{";
			bool first = true;
			
			for (unsigned i = 0; i < v.size(); i += 1) {
				if (!first) {
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
			void normalize ();
			
		protected:
			ValueT m_value;
			
			static char nibbleToHex (DigitT);
			static char convertToChar (DigitT);
			static DigitT convertToDigit (char);
			
			// For use with compareWith
			static const int LESSER = -1;
			static const int EQUAL = 0;
			static const int GREATER = 1;
			
			int compareWith(const Integer & other) const;
			
		public:
			Integer(IntermediateT value = 0);
			Integer(ValueT digits);
			Integer(std::string value, std::size_t base = 16);
			
			~Integer ();
			
			// Packing and Unpacking
			Integer(const DigitT * data, std::size_t size);
			void unpack(DigitT * data, std::size_t size) const;
			
			Integer & operator=(const Integer & other);
			
			bool operator<(const Integer & other) const;
			bool operator>(const Integer & other) const;
			bool operator<=(const Integer & other) const;
			bool operator>=(const Integer & other) const;
			
			bool isZero () const;
			bool operator!=(const Integer & other) const;
			bool operator==(const Integer & other) const;
			
			const DigitT & operator[](std::size_t index) const { return m_value.at(index); }
			DigitT & operator[](std::size_t index) { return m_value.at(index); }
			
			std::size_t size () const { return m_value.size(); }
			
			// Adds a given amount to the current integer.
			// a can be aliased for this.
			void add(const Integer & a);
			
			// a must be smaller than this.
			// a cannot be aliased for this - it doesn't make sense anyway :)
			void subtract(const Integer & a);
			
			// Convenience function
			void multiply(const Integer & other);
			
			// Convenience function
			void modulus (const Integer & m);
			
			// Cannot be aliased
			void setProduct(const Integer & a, const Integer & b);
			
			// Returns false if the division had a remainder.
			bool setFractionSlow(const Integer & numerator, const Integer & denominator, Integer & remainder);
			void setFraction(const Integer & numerator, const Integer & denominator, Integer & remainder);
			
			// Slightly faster implementation, calculates x = qy + r, saves q in this, r in x
			void setFraction(Integer & x, Integer y);
			
			//void setPower (Integer base, Integer exponent);
			void setPower (Integer base, Integer exponent);
			
			// Uses Barrett Reduction
			void setPower (Integer base, Integer exponent, const Integer & mod);
			void setPower (Integer base, Integer exponent, const BarrettReduction & r);
			
			// Shift left makes a number bigger.
			void shiftLeft(DigitT amount);
			
			// Shift right makes a number smaller.
			void shiftRight(DigitT amount);
			
			void binaryAnd(const Integer & other);
			void binaryOr(const Integer & other);
			void binaryNot();
			
			// Returns a large random number. Length is in multiple of 32 bits.
			void generateRandomNumber (DigitT length);
			
			// Returns a large random number between min and max.
			void generateRandomNumber (Integer min, Integer max);
			
			// Returns the greatest common divisor of a and b.
			void calculateGreatestCommonDivisor (Integer a, Integer b);
			
			//	Computes inv = u^(-1) mod v
			void calculateInverse (Integer u, Integer v);
			
			// Generates a prime number
			void generatePrime (Integer min, Integer max);
			
			// Generates a prime number - length is in multiple of 32 bits.
			void generatePrime (DigitT length);
			
			// Use probabilistic methods to check for prime number
			bool isProbablyPrime (int tests = 10) const;
			
			// Finds a prime less than the given max, or returns false.
			bool findPrimeLessThan (Integer max);
			
			std::string toString(std::size_t base = 16, bool prefix = true) const;
			
			void debug();
			
			// Leaky Abstraction :p
			const ValueT & value () const { return m_value; }
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
			
			BarrettReduction (const Integer & _mod);
			void modulus (Integer & x) const;
		};
		
		
		std::ostream & operator<< (std::ostream &, const Integer &);
	}
}

#endif

