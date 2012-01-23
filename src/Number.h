//
//  Number.h
//  Kai
//
//  Created by Samuel Williams on 16/12/11.
//  Copyright (c) 2011 Orion Transfer Ltd. All rights reserved.
//

#ifndef Kai_Integer_h
#define Kai_Integer_h

#include "Object.h"
#include "Math/Integer.h"
#include "Math/Number.h"

namespace Kai {
	class Integral {
	public:
		static const char * const NAME;
		
		virtual ~Integral();
		
		virtual Math::Integer to_integer() const abstract;
	};
	
	class Integer : public Object, virtual public Integral {
	public:
		typedef Math::Integer ValueT;
		
		enum { DEFAULT_RADIX = 16 };
		
	protected:
		ValueT _value;
		
	public:
		static const char * const NAME;
		
		Integer (ValueT value);
		virtual ~Integer ();
		
		virtual Ref<Symbol> identity(Frame * frame) const;
		
		virtual Math::Integer to_integer() const;
		
		ValueT & value() { return _value; }
				
		virtual ComparisonResult compare(const Object * other) const;
		ComparisonResult compare(const Integer * other) const;
		
		virtual void to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const;
		
		static Ref<Object> sum(Frame * frame);
		static Ref<Object> product(Frame * frame);
		static Ref<Object> subtract(Frame * frame);
		static Ref<Object> modulus(Frame * frame);
		static Ref<Object> power(Frame * frame);
		
		static Ref<Object> fractional_part(Frame * frame);
		
		static Ref<Object> greatest_common_divisor(Frame * frame);
		static Ref<Object> generate_prime(Frame * frame);
		
		static Ref<Object> from_string(Frame * frame);
		static Ref<Object> to_string(Frame * frame);
		
		static Ref<Object> to_number(Frame * frame);
		
		static void import(Frame * frame);
	};
	
	class Number : public Object, virtual public Integral {
	public:
		typedef Math::Number ValueT;
		
	protected:
		ValueT _value;
	
	public:
		static const char * const NAME;
		
		Number(ValueT value);
		virtual ~Number();
		
		virtual Ref<Symbol> identity(Frame * frame) const;
		
		virtual Math::Integer to_integer() const;
		
		ValueT & value() { return _value; }
				
		virtual ComparisonResult compare(const Object * other) const;
		ComparisonResult compare(const Number * other) const;
		
		virtual void to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const;
		
		static Ref<Object> product(Frame * frame);

		static void import(Frame * frame);
	};
}

#endif
