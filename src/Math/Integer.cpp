/*
 *  Integer.cpp
 *  RSA
 *
 *  Created by Samuel Williams on 7/08/10.
 *  Copyright 2010 Orion Transfer Ltd. All rights reserved.
 *
 */

#include "Integer.h"
#include <stdexcept>
#include <sstream>
#include <cassert>
#include <fstream>

// Memset
#include <string.h>

namespace Kai {
	namespace Math {
		char Integer::convert_to_character(Integer::DigitT d) {
			if (d < 10) {
				return '0' + d;
			} else if (d < 36) {
				return 'A' + (d - 10);
			}
			
			throw std::range_error("Could not convert digit to character - out of range!"); 
		}

		Integer::DigitT Integer::convert_to_digit(char c) {
			Integer::DigitT d = c - '0';
			if (d < 10) {
				return d;
			} else {
				d = c - 'A';
				
				if (d < 26) {
					return d + 10;
				}
			}
			
			throw std::range_error("Could not convert character to digit - out of range!");
		}
		
		const char * Integer::prefix_for_base(std::size_t base) {
			switch (base) {
				case 2:
					return "b";
				
				case 8:
					return "0";
				
				case 16:
					return "0x";
					
				default:
					return "";
			}
		}

		Integer::Integer (IntermediateT value) {
			for (std::size_t i = 0; i < sizeof(value); i += sizeof(DigitT)) {
				DigitT d = (DigitT)value;
				
				_value.push_back(d);
				
				value = value >> (sizeof(DigitT) * 8);
				
				if (value == 0)
					break;
			}
		}
		
		void Integer::convert_arbitrary_base(std::string value, const Integer & base, Integer & result) {
			// We are assuming single bytes per integral value
			for (std::size_t i = 0; i < value.length(); i += 1) {
				DigitT digit = convert_to_digit(value[i]);
				
				result *= base;
				
				result += digit;
			}
		}
		/*
		Integer Integer::convert_power_of_2_base(std::string value, std::size_t base) {
			_value.resize((value.size() * 4 + (DIGIT_BITS - 1)) / DIGIT_BITS);
			
			for (std::size_t i = 0; i < _value.size(); i += 1) {
				DigitT d = 0;
				
				for (std::size_t j = (i*8); j < value.size() && j < ((i+1)*8); j += 1) {
					d = d << 4;			
					d |= convert_to_digit(value[j]);
				}
				
				_value.at(_value.size() - (i+1)) = d;
			}			
		}
		 */
		
		Integer::Integer(std::string value, std::size_t base) {
			_value.push_back(0);
			
			convert_arbitrary_base(value, base, *this);
		}

		Integer::Integer (ValueT v) : _value(v) {
			
		}

		Integer::~Integer () {
			//debug();
		}

		Integer::Integer(const DigitT * data, std::size_t size) {
			_value.reserve(size);
			for (std::size_t i = 0; i < size; i++) {
				_value.push_back(data[i]);
			}
		}

		void Integer::unpack(DigitT * data, std::size_t size) const {	
			std::size_t i = 0;
			
			for (; i < size && i < _value.size(); i++) {
				data[i] = _value[i];
			}
			
			for (; i < size; i++) {
				data[i] = 0;
			}
		}

		Integer & Integer::operator= (const Integer & other) {
			_value = other._value;
			
			return *this;
		}

		void Integer::normalize () {
			if (_value.size() == 0) {
				_value.push_back(0);
				return;
			}
			
			std::size_t c = _value.size() - 1;
			
			// mmm Fancy :p
			while (c > 0 && _value[c] == 0) {
				c -= 1;
			}
			
			_value.resize(c+1);
		}

		int Integer::compare_with (const Integer & other) const {
			std::size_t width = std::max(_value.size(), other._value.size());
			
			for (std::size_t i = 0; i < width; i += 1) {
				std::size_t j = width - (i+1);
				
				DigitT lhs = 0, rhs = 0;
				
				if (j < _value.size())
					lhs = _value[j];
				
				if (j < other._value.size())
					rhs = other._value[j];
				
				if (lhs < rhs) {
					return -1;
				} else if (lhs > rhs) {
					return 1;
				}
			}
			
			return 0;
		}

		bool Integer::operator< (const Integer & other) const {
			int c = compare_with(other);
			
			if (c == -1) {
				return true;
			} else {
				return false;
			}
		}

		bool Integer::operator> (const Integer & other) const {
			return (other < (*this));
		}

		bool Integer::operator<= (const Integer & other) const {
			int c = compare_with(other);
			
			if (c == 1) {
				return false;
			} else {
				return true;
			}
		}

		bool Integer::operator>= (const Integer & other) const {
			return (other <= (*this));
		}

		bool Integer::is_zero () const {
			Integer z = 0;
			
			return (*this) == z;
		}

		bool Integer::operator!= (const Integer & other) const {
			return !((*this) == other);
		}

		bool Integer::operator== (const Integer & other) const {
			int c = compare_with(other);
			
			if (c == 0) {
				return true;
			} else {
				return false;
			}
		}

		void Integer::add(const Integer & a) {
			if (_value.size() < a._value.size()) {
				_value.resize(a._value.size());
			}
			
			IntermediateT carry = 0;
			std::size_t i = 0;
			
			for (; i < a._value.size(); i += 1) {
				IntermediateT result = (IntermediateT)_value[i] + (IntermediateT)a._value[i] + carry;
				
				_value[i] = (DigitT)result;
				carry = result >> DIGIT_BITS;
			}
			
			for (;carry != 0 && i < _value.size(); i += 1) {
				IntermediateT result = (IntermediateT)_value[i] + carry;
				
				_value[i] = (DigitT)result;
				carry = result >> DIGIT_BITS;
			}
			
			if (carry != 0) {
				_value.push_back(carry);
			}
		}

		void Integer::subtract(const Integer & _a) {
			Integer a = _a; a.normalize();
			
			//assert((*this) >= a);
			//Integer start = *this;
			
			std::size_t width = a._value.size();	
			IntermediateT take = 0;
			
			for (std::size_t i = 0; i < width; i += 1) {
				IntermediateT remove = take;
				
				if (i < a._value.size())
					remove += (IntermediateT)a._value[i];
				
				if (_value[i] >= remove) {
					_value[i] -= remove;
					
					take = 0;
				} else {
					width = std::max(width, i+2);
					
					_value[i] = ((IntermediateT)_value[i] + B) - remove;
					
					// Take 1 from the next digit
					take = 1;
				}
			}
			
			// If we subtracted something, the end result should be less than the start.
			//assert(*this <= start);
		}

		void Integer::multiply(const Integer & other) {
			Integer a = *this;
			Integer b = other; // Make a copy of the integer data, to avoid aliasing issues.
			
			this->set_product(a, b);
		}

		void Integer::modulus (const Integer & m) {
			Integer numerator = *this;
			Integer result = 0;
			
			result.set_fraction(numerator, m, *this);
		}

		// My initial attempt, but this version is a bit slow due to the reliance on addition
		// The better implementation includes this addtion in the inner loop, so it's faster.
		/*
		void Integer::set_product(const Integer & a, const Integer & b) {
			// Multiplication by repeated doubling.
			if (a < b)
				set_product(b, a);
			
			(*this) = 0;

			for (std::size_t i = 0; i < b._value.size(); i += 1) {
				IntermediateT carry = 0;
				IntermediateT m = b._value[i];
				Integer row = 0;
				row._value.resize(a._value.size() + 1 + i);
				
				for (std::size_t j = 0; j < a._value.size(); j += 1) {
					IntermediateT result = (m * (IntermediateT)a._value[j]) + carry;
					
					row._value[j+i] = result;
					carry = result >> DIGIT_BITS;
				}
				
				if (carry) {
					row._value[a._value.size() + i] = carry;
				}
				
				this->add(row);
			}
			
			this->normalize();
		}
		*/

		void Integer::set_product(const Integer & x, const Integer & y) {
			assert(x.size() != 0);
			assert(y.size() != 0);
			
			std::size_t n = x.size() - 1, t = y.size() - 1;
			
			//for (std::size_t i = 0; i < _value.size(); i++)
			//	_value[i] = 0;
			
			memset(&_value[0], 0, _value.size() * sizeof(DigitT));
			
			_value.resize(x.size() + y.size() + 2);
			
			for (std::size_t i = 0; i <= t; i += 1) {
				IntermediateT carry = 0;
				
				for (std::size_t j = 0; j <= n; j++) {
					IntermediateT product = (IntermediateT)x[j] * (IntermediateT)y[i];
					IntermediateT result = (IntermediateT)_value[i+j] + product + carry;

					_value[i+j] = (DigitT)result;
					carry = result >> DIGIT_BITS;
				}
				
				_value[i+n+1] = carry;
			}
			
			this->normalize();
		}

		struct Shift {
			Integer count, product;
		};

		bool Integer::set_fraction_slow(const Integer & numerator, const Integer & denominator, Integer & remainder) {
			if (numerator == 0) {
				remainder = 0;
				(*this) = 0;
				
				return true; // no remainder
			}
			
			if (denominator == 0) {
				throw std::runtime_error("Division by 0!");
			}
			
			(*this) = 0; // Number of divisions possible.
			Integer accumulator = 0; // Total value of doublings.
				
			bool top = false;
			
			std::vector<Shift> shifts;
			// This pair must typically be shifted together.
			Shift s = {1, denominator};
			shifts.push_back(s);
			
			std::size_t offset = 0;
			
			while (true) {
				//std::cout << count << std::endl;
				Integer tmp = s.product;
				tmp.add(accumulator);
				
				int r = tmp.compare_with(numerator);
				
				if (r == 0) {
					// We have found a division with no remainder
					this->add(s.count);
					//accumulator.add(s.product);
					remainder = 0;
					
					return true;
				} else if (r == 1) {
					// We have found a division with a remainder
					if (offset == 0) {
						// We have found a divisor with a remainder
						remainder = numerator;
						remainder.subtract(accumulator);
						
						return false;
					}
					
					offset -= 1;
					s = shifts[offset];
					top = true;
					
					continue;
				}
				
				this->add(s.count);
				accumulator.add(s.product);
				
				if (!top) {
					offset += 1;
					
					//if (offset >= shifts.size()) {
						s.count.shift_left(1);
						s.product.shift_left(1);
						
						shifts.push_back(s);
					//} else {
					//	s = shifts[offset];
					//}
				}
			}
		}

		void Integer::set_fraction(const Integer & numerator, const Integer & denominator, Integer & remainder) {
			if (numerator < denominator) {
				(*this) = 0;
				remainder = numerator;
				return;
			}
			
			Integer x = numerator;
			Integer y = denominator;
			x.normalize();
			y.normalize();
			
			// Normalize?
			std::size_t shift = 0;
			
			DigitT back = y._value.back();
			while (back < (B/2)) {
				back = back << 1;
				shift += 1;
				
				assert(back != 0);
			}
			
			if (shift) {
				x.shift_left(shift);
				y.shift_left(shift);
			}
			
			set_fraction(x, y);
			
			if (shift) {
				x.shift_right(shift);
			}
			/*
			Integer c;
			c.set_product(*this, denominator);
			c.add(x); // remainder
			
			// Check the result is correct
			assert(x < denominator);
			assert(numerator == c);
			*/
			remainder = x;
		}
		
		void Integer::trim() {
			std::size_t non_zero = _value.size() - 1;
			
			while (non_zero > 0 && _value[non_zero] == 0) {
				non_zero -= 1;
			}
			
			_value.resize(non_zero + 1);
		}

		void Integer::set_fraction(Integer & x, Integer y) {
			// The number of digits in the result
			const std::size_t n = x.size() - 1;
			const std::size_t t = y.size() - 1;
			
			assert(n >= t);
			assert(t >= 0);
			assert(y[t] != 0);
			
			// numerator is smaller than denominator, done.
			if (n < t) return;
			
			const std::size_t nt = n - t;
			
			Integer & q = *this;
			
			q._value.clear();
			q._value.resize(nt+1);
			
			// Base (radix)
			Integer bb; bb.set_product(B, B);
			Integer bp; bp.set_power(B, nt);
			
			Integer tmp1, tmp2;

			if (y[t] < (B/2)) {
				std::cerr << "Thar be the dragons!" << std::endl;
			}
			
			tmp1.set_product(y, bp);
			while (x >= tmp1) {
				q[nt] += 1;
				x.subtract(tmp1);
			}
			
			for (std::size_t i = n; i > t; i--) {		
				if (x[i] == y[t]) {
					q[i-t-1] = B - 1;
				} else {
					IntermediateT t1 = (IntermediateT)x[i] * B;
					if (i > 0) t1 += x[i-1];
					q[i-t-1] = t1 / (IntermediateT)y[t];
				}
				
				// If we had 128 bit arithmetic, we could do this on the CPU.
				Integer u; 
				u.set_product(y[t], B);
				if (t > 0) u.add(y[t-1]);
				
				Integer v;
				v.set_product(x[i], bb);
				if (i > 0) { tmp1.set_product(x[i-1], B); v.add(tmp1); }
				if (i > 1) v.add(x[i-2]);
						
				while (true) {
					tmp1.set_product(q[i-t-1], u);
					if (tmp1 > v) {
						q[i-t-1] -= 1;
					} else {
						break;
					}
				}
				
				tmp1.set_power(B, i-t-1);
				tmp2.set_product(y, tmp1);
				tmp1.set_product(q[i-t-1], tmp2);
				
				if (tmp1 > x) {
					x.add(tmp2);
					x.subtract(tmp1);
					q[i-t-1] -= 1;
				} else {
					x.subtract(tmp1);
				}
			}
			
			// Removing any preceeding zeros:
			q.trim();
		}

		#pragma mark Modular Exponentiation
			
		BarrettReduction::BarrettReduction (const Integer & _mod) {
			mod = _mod;
			mod.normalize();
			
			// Remainder - temporary.
			Integer r;
			
			// Radix - the number of possible values per digit
			b = (Integer::IntermediateT)1 << Integer::DIGIT_BITS;
			bk.set_power(b, mod.size() * 2);
			mu.set_fraction(bk, mod, r);
			
			// Division by 0x100 is the same as shift_right(2)
			bn = Integer::DIGIT_BITS * (mod.size() - 1);
			bp = Integer::DIGIT_BITS * (mod.size() + 1);

			// Mask for base-2 modulus
			bkp.set_power(b, mod.size() + 1);
			bkm = bkp;
			bkm.subtract(1);
		}
			
		void BarrettReduction::modulus (Integer & x) const {
			Integer q1, q2, q3, r1, r2;

			q1 = x;
			q1.shift_right(bn);
			
			//q2.set_product(q1, mu);
			
			//q3 = q2;
			q3.set_product(q1, mu);
			q3.shift_right(bp);
			
			//r1 = x;
			//r1.binary_and(bkm);
			x.binary_and(bkm);
			
			r2.set_product(q3, mod);
			r2.binary_and(bkm);
			
			if (r2 > x) {
				x.add(bkp);
			}
			
			x.subtract(r2);
			
			while (x >= mod) {
				x.subtract(mod);
			}
			
			//x = r1;
			x.normalize();
		}

		void Integer::set_power (Integer base, Integer exponent) {
			(*this) = 1;
			
			while (exponent != 0) {
				//std::cout << "ex = " << exponent.to_string(16) << std::endl;
				if (exponent._value[0] & 1) {
					this->multiply(base);
					//this->modulus(mod);
				}
				
				exponent.shift_right(1);
				base.multiply(base);
				//base.modulus(mod);
			}
		}

		void Integer::set_power (Integer base, Integer exponent, const Integer & mod) {
			BarrettReduction r (mod);
			set_power(base, exponent, r);
		}

		void Integer::set_power (Integer base, Integer exponent, const BarrettReduction & r) {
			(*this) = 1;
			Integer tmp;
			
			while (exponent != 0) {
				if (exponent._value[0] & 1) {
					//this->multiply(base);
					tmp.set_product(*this, base);
					tmp._value.swap(this->_value);
					
					r.modulus(*this);
					//this->modulus(r.mod);
				}
				
				exponent.shift_right(1);
				
				//base.multiply(base);
				tmp.set_product(base, base);
				tmp._value.swap(base._value);
				
				r.modulus(base);
				//base.modulus(r.mod);
			}
		}

		/*
		// Not quite finished, sliding window method
		void Integer::set_power (Integer base, Integer exponent, const BarrettReduction & r) {
			const std::size_t k = 3;
			const std::size_t l = 1 << (k - 1);
			
			(*this) = 1;
			
			Integer tmp;
			tmp.set_product(base, base);
			
			std::vector<Integer> g;
			g.resize(2*l + 1);
			
			g[1] = base;
			g[2] = tmp;
			
			for (std::size_t i = 1; i < l; i++) {
				tmp.set_product(g[2*i - 1], g[2])
				g[2*i + 1] = tmp;
			}
			
			(*this) = 1;
			std::ptrdiff_t i = exponent.size();
			
			while (exponent != 0) {
				if ((exponent._value[0] & 1) == 0) {
					this->multiply(*this);
					r.modulus(*this);
					
					i = i - 1;
					
					exponent.shift_right(1);
				} else {
					
				}
			}
		}
		*/
		void Integer::shift_left(DigitT amount) {
			DigitT steps = (amount / DIGIT_BITS);
			DigitT bits = (amount % DIGIT_BITS);
			
			_value.resize(_value.size() + (steps + 1));
			
			for (std::size_t i = (steps+1); i < _value.size(); i += 1) {
				std::size_t j = _value.size() - (i+1);
				
				IntermediateT result = (IntermediateT)_value[j] << bits;
				
				_value[j+steps+1] |= (result >> DIGIT_BITS);
				_value[j+steps] = result;
			}
			
			// Feed zeros onto the left hand side.
			for (std::size_t i = 0; i < steps; i += 1) {
				_value[i] = 0;
			}
			
			if (_value.back() == 0) {
				_value.pop_back();
			}
		}

		void Integer::shift_right(DigitT amount) {
			DigitT steps = (amount / DIGIT_BITS);
			DigitT bits = (amount % DIGIT_BITS);
			
			if (steps > _value.size()) {
				_value.resize(1);
				_value[0] = 0;
				
				return;
			}
			
			for (std::size_t i = steps; i < _value.size(); i += 1) {
				IntermediateT result = (IntermediateT)_value[i] << (DIGIT_BITS - bits);
				
				std::size_t s = i - steps;
				if (s != 0)
					_value[s-1] |= result;
				
				_value[s] = (result >> DIGIT_BITS);

			}
			
			// Truncate the right hand side
			_value.resize(_value.size() - steps);
			normalize();
		}

		void Integer::binary_and(const Integer & other) {
			std::size_t width = std::min(size(), other.size());
			
			std::size_t i = 0;
			
			for (; i < width; i += 1) {
				_value[i] &= other[i];
			}
			
			_value.resize(width);
		}

		void Integer::binary_or(const Integer & other) {
			std::size_t width = std::max(size(), other.size());
			_value.resize(width);
			
			std::size_t i = 0;
			
			for (; i < other.size(); i += 1) {
				_value[i] |= other[i];
			}
		}

		void Integer::binary_not() {
			for (std::size_t i = 0; i < _value.size(); i += 1) {
				_value[i] = ~_value[i];
			}
		}

		// Returns a large random number. Digits is in multiple of 32 bits.
		void Integer::generate_random_number (Integer::DigitT length) {
			static std::ifstream * randomDevice = NULL;
			
			if (!randomDevice) {
				randomDevice = new std::ifstream("/dev/urandom", std::ios::binary);
			}
			
			_value.resize(length);
			randomDevice->read((char*)&_value[0], (std::size_t)_value.size() * sizeof(DigitT));
		}

		// Returns a large random number between min and max.
		void Integer::generate_random_number (Integer min, Integer max) {
			generate_random_number(max.value().size());

			Integer diff = max;
			diff.subtract(min);
			
			this->modulus(diff);
			this->add(min);
			
			assert(*this < max);
			assert(*this > min);
		}

		// Returns the greatest common divisor of a and b.
		void Integer::calculate_greatest_common_divisor (Integer a, Integer b) {
			while (b != 0) {
				Integer tmp = a;
				tmp.modulus(b);
				
				a = b;
				b = tmp;
			}
			
			(*this) = a;
		}

		//	Computes inv = u^(-1) mod v */
		//	Ref: Knuth Algorithm X Vol 2 p 342
		//		ignoring u2, v2, t2
		//		and avoiding negative numbers.
		//		Returns non-zero if inverse undefined.
		void Integer::calculate_inverse (Integer u, Integer v) {
			Integer u1 = 1, u3 = u, v1 = 0, v3 = v;	
			bool odd = false;
			
			Integer q, t3, w, t1;
			while (v3 != 0) {
				// BigIntT q = u3 / v3, t3 = u3 % v3;
				q.set_fraction(u3, v3, t3);
				
				// BigIntT w = q * v1;
				w.set_product(q, v1);
				
				// BigIntT t1 = u1 + w;
				t1 = u1;
				t1.add(w);
				
				u1 = v1;
				v1 = t1;
				u3 = v3;
				v3 = t3;
				
				odd = !odd;
			}
			
			if (odd) {
				(*this) = v;
				this->subtract(u1);
			} else {
				(*this) = u1;
			}
		}

		// This function implements simple jacobi test.
		// We can expect compiler to perform tail-call optimisation.
		int jacobi (Integer m, Integer n) {
			int i = 1;
			Integer t;
			
			while (m > 1) {
				Integer j = 0;
				
				while ((m[0] & 1) == 0) {
					j.add(1);
					m.shift_right(1);
				}
				
				if ((j[0] & 1) == 1) {
					t = n[0] & 7;
					
					if ((t == 3) || (t == 5))
						i = -i;
				}
				
				if ((m[0] & 3) == 3 && (n[0] & 3) == 3) {
					i = -i;
				}
				
				t = n;
				
				t.modulus(m);
				n = m;
				m = t;
			}
			
			return i;
		}

		bool Integer::is_probably_prime (int tests) const {
			const Integer & p = *this;
			
			if (p == 2) {
				return true;
			}
			
			// Cache the reduction for better set_power.
			BarrettReduction br (p);
			
			while (tests-- > 0) {
				Integer a = 0;
				a.generate_random_number(2, p);

				Integer gcd = 0;
				gcd.calculate_greatest_common_divisor(a, p);

				if (gcd == 1) {
					//BigIntT l = calculatePower(a, (p-1)/2, p);
					Integer l = 0, e = 0, p1 = p;
					p1.subtract(1);
					e = p1;
					e.shift_right(1);
					
					l.set_power(a, e, br);
					
					//SBigIntT j = jacobi(a, p);
					int j = jacobi(a, p);
					
					if (((j == -1) && (l == p1)) || ((j == 1) && (l == 1))) {
						// So far so good...
					} else {
						// p is composite
						return false;
					}
				} else {
					// p is composite
					return false;
				}
			}
			
			return true;
		}

		void Integer::generate_prime (DigitT length) {
			while (true) {
				//std::cout << "." << std::flush;
				this->generate_random_number(length);
				this->_value[0] |= 1; // Ensure odd number
				
				if (is_probably_prime()) {
					//std::cout << std::endl;
					return;
				}
			}
		}

		void Integer::generate_prime (Integer min, Integer max) {
			while (true) {
				//std::cout << "." << std::flush;
				this->generate_random_number(min, max);
				this->_value[0] |= 1; // Ensure odd number
				
				if (is_probably_prime()) {
					//std::cout << std::endl;
					return;
				}
			}
		}

		bool Integer::find_prime_less_than (Integer max) {
			while (max > 0) {
				max.subtract(1);
				
				if (max.is_probably_prime()) {
					(*this) = max;
					return true;
				}
			}
			
			return false;
		}
		
		std::string Integer::to_hexadecimal(bool prefix) const {
			std::stringstream buffer;

			if (prefix)
				buffer << "0x";

			for (std::size_t i = 0; i < _value.size(); i += 1) {
				DigitT d = _value[_value.size() - (i+1)];

				for (unsigned n = 0; n < DIGIT_BITS; n += 4) {
					buffer << convert_to_character((d >> (DIGIT_BITS - (n+4))) & 0xF);
				}
			}

			return buffer.str();
		}

		std::string Integer::to_string(const Integer & base, bool prefix) const {
			std::stringstream buffer;
			Integer copy = *this;
			
			if (prefix)
				buffer << prefix_for_base(base.to_int64());
			
			while (!copy.is_zero()) {
				Integer result, remainder;
				
				result.set_fraction(copy, base, remainder);
				
				buffer << convert_to_character(remainder.to_int64());
				
				result.swap(copy);
			}
			
			return buffer.str();
		}

		void Integer::debug() {
			std::cerr << "Value = " << to_hexadecimal() << std::endl;
		}

		std::ostream & operator<< (std::ostream & output, const Integer & i) {
			output << i.to_string(16);
			
			return output;
		}
	}
}
