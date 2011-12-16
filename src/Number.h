//
//  Number.h
//  Kai
//
//  Created by Samuel Williams on 16/12/11.
//  Copyright (c) 2011 Orion Transfer Ltd. All rights reserved.
//

#ifndef Kai_Integer_h
#define Kai_Integer_h

#include "Value.h"
#include "Math/Integer.h"

namespace Kai {
	class Integer : public Value {
	public:
		typedef Math::Integer ValueT;
		
	protected:
		ValueT m_value;
		
	public:
		Integer (ValueT value);
		virtual ~Integer ();
		
		ValueT & value () { return m_value; }
		
		virtual Ref<Value> prototype ();
		
		virtual int compare (const Value * other) const;
		int compare (const Integer * other) const;
		
		virtual void toCode(StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const;
		
		static Ref<Value> sum (Frame * frame);
		static Ref<Value> product (Frame * frame);
		static Ref<Value> subtract (Frame * frame);
		static Ref<Value> modulus (Frame * frame);
		
		static Ref<Value> greatest_common_divisor(Frame * frame);
		static Ref<Value> generate_prime(Frame * frame);
		
		static Ref<Value> to_string(Frame * frame);
		
		static Ref<Value> globalPrototype ();
		static void import (Table * context);
	};
}

#endif
