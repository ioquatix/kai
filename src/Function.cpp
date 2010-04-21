/*
 *  Function.cpp
 *  This file is part of the "Kai" project, and is licensed under the GNU GPLv3.
 *
 *  Created by Samuel Williams on 13/04/10.
 *  Copyright 2010 Samuel Williams. All rights reserved.
 *
 */

#include "Function.h"
#include "Frame.h"
#include "Exception.h"
#include "Ensure.h"

#include <iostream>

namespace Kai {
	namespace Builtins {

		Value * trace (Frame * frame) {
			Cell * arguments = frame->unwrap();
			
			return new String(Value::toString(arguments));
		}
		
		Value * value (Frame * frame) {
			return frame->operands()->head();
		}
		
		class KWrapper : public Value {
			private:
				Value * m_value;
			
			public:
				KWrapper (Value * value)
					: m_value(value)
				{
				
				}
				
				virtual Value * evaluate (Frame * frame) {
					return m_value;
				}
				
				virtual void toCode (StringStreamT & buffer) {
					buffer << "(wrap ";
					m_value->toCode(buffer);
					buffer << ')';
				}
		};
		
		Value * wrap (Frame * frame) {
			return new KWrapper(frame->operands());
		}

		Cell * unwrap (Frame * frame) {
			return frame->unwrap();
		}
		
		Value * update (Frame * frame) {
			Cell * arguments = unwrap(frame);
			ensure(arguments != NULL);
			
			Symbol * key = dynamic_cast<Symbol*>(arguments->head());
			Cell * next = dynamic_cast<Cell*>(arguments->tail());
			Value * value;
			
			if (next)
				value = next->head();
			else
				throw Exception("Expected Value", frame);
		
			//std::cerr << "Updating " << Value::toString(key) << " to " << Value::toString(value) << std::endl;
		
			Table * table = dynamic_cast<Table*>(frame->caller());
			
			if (!key) {
				StringStreamT s; s << "Non-Symbolic Key: " << Value::toString(arguments->head());
				throw Exception(s.str(), frame);
			}
			
			if (!table) {
				throw Exception("Invalid Table", frame);
			}
			
			if (value == NULL)
				return table->remove(key);
			else
				return table->update(key, value);
		}

		Symbol * compare (Frame * frame) {
			Cell * arguments = unwrap(frame);

			if (arguments == NULL || arguments->head() == NULL || arguments->tail() == NULL) {
				throw Exception("Invalid Comparison", frame);
			}

			int result = arguments->head()->compare(arguments->tail());
			
			if (result < 0)
				return new Symbol("descending");
			else if (result > 0)
				return new Symbol("ascending");

			return new Symbol("equal");
		}
		
		Value * head (Frame * frame) {
			Cell * arguments = unwrap(frame);
			
			return arguments->head();
		}
		
		Value * tail (Frame * frame) {
			Cell * arguments = unwrap(frame);
			
			return arguments->tail();
		}
	}
}
