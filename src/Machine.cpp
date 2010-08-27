/*
 *  Machine.cpp
 *  Kai
 *
 *  Created by Samuel Williams on 12/07/10.
 *  Copyright 2010 Samuel Williams. All rights reserved.
 *
 */

#include "Machine.h"

#include <list>
#include <iostream>
/*
namespace Kai {

	class Module {
		protected:
			static std::list<StringT> m_searchPaths;
			
		public:
			static Value * load (Frame * frame) {
				
			}
			
			static void import (Table * context) {
				context->update(new Symbol("load"), KFunctionWrapper(Module::load));
			}
	};

	class Machine {
		public:
			static Value * debug (Frame * frame) {
				std::cerr << "Debug: " << Kai::Value::toString(frame->operands()) << std::endl;
				return NULL;
			}
	};
	
}
*/

extern "C" {
	Kai::Value * _valueType () {
		return NULL;
	}
	
	Kai::Frame * _frameType () {
		return NULL;
	}

	void test () {
		std::cerr << "[Compiler Online]" << std::endl;
	}
	
	Kai::Value * debug (Kai::Frame * frame) {
		std::cerr << "Debug: " << Kai::Value::toString(frame->operands()) << std::endl;
		
		return NULL;
	}
/*	
	void kai_module_import (Kai::Table * module, Kai::Frame * frame) {
		using namespace Kai;
		
		module->update("debug", KFunctionWrapper(Machine::debug));
	}
*/
};
