//
//  Machine.cpp
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 12/07/10.
//  Copyright 2010 Samuel Williams. All rights reserved.
//
//

#include "Machine.hpp"

#include <list>
#include <iostream>
//
namespace Kai {

	class Module {
		protected:
			static std::list<StringT> _searchPaths;
			
		public:
			static Object * load (Frame * frame) {
				
			}
			
			static void import (Table * context) {
				context->update(frame->sym("load"), KAI_BUILTIN_FUNCTION(Module::load));
			}
	};

	class Machine {
		public:
			static Object * debug (Frame * frame) {
				std::cerr << "Debug: " << Kai::Object::to_string(frame->operands()) << std::endl;
				return NULL;
			}
	};
	
}
*/

extern "C" {
	Kai::Object * _valueType () {
		return NULL;
	}
	
	Kai::Frame * _frameType () {
		return NULL;
	}

	void test () {
		std::cerr << "[Compiler Online]" << std::endl;
	}
	
	Kai::Object * debug (Kai::Frame * frame) {
		std::cerr << "Debug: " << Kai::Object::to_string(frame->operands()) << std::endl;
		
		return NULL;
	}
//	
	void kai_module_import (Kai::Table * module, Kai::Frame * frame) {
		using namespace Kai;
		
		module->update("debug", KAI_BUILTIN_FUNCTION(Machine::debug));
	}
*/
};
