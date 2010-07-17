/*
 *  Compiler.cpp
 *  Kai
 *
 *  Created by Samuel Williams on 11/07/10.
 *  Copyright 2010 Samuel Williams. All rights reserved.
 *
 */

#include "Compiler.h"

#include "Frame.h"
#include "Function.h"

#include <string>
#include <memory>
#include <limits>

// llvm::InitializeNativeTarget()
#include <llvm/Target/TargetSelect.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Module.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/ExecutionEngine/JIT.h>

namespace Kai {
	
	Compiler::Compiler ()
	{
		using std::auto_ptr;
		
		static bool s_initialized = false;		
		
		if (!s_initialized) {
			llvm::InitializeNativeTarget();
			llvm::llvm_start_multithreaded();
			
			s_initialized = true;
		}
		
		llvm::LLVMContext & context = llvm::getGlobalContext();
		
		StringT error;
		auto_ptr<llvm::MemoryBuffer> buffer(llvm::MemoryBuffer::getFile("machine.o"));
		
		if (!buffer.get()) {
			std::cerr << "Could not load machine bitcode!" << std::endl;
			
			return;
		}
		
		auto_ptr<llvm::Module> module(llvm::ParseBitcodeFile(buffer.get(), context, &error));
		buffer.release();
		
		if (!module.get()) {
			std::cerr << "Could not create machine module!" << std::endl;
			std::cerr << "*** " << error << std::endl;
			
			return;
		}
		
		m_engine = llvm::ExecutionEngine::createJIT(module.get(), &error);
		module.release();
		
		if (!m_engine) {
			std::cerr << "Could not create execution engine!" << std::endl;
			std::cerr << "*** " << error << std::endl;
		}
	}
	
	Compiler::~Compiler () {
		std::cout << "Freeing execution engine" << std::endl;
		
		if (m_engine)
			delete m_engine;
	}
	
	// Returns a compiled function corresponding to the given arguments.
	Value * Compiler::evaluate (Frame * frame) {
		String * name;
		Cell * arguments;
		Value * body;
		
		frame->extract()[name][arguments][body];
		
		std::vector<StringT> values;
		std::vector<llvm::Type*> signature;
		
		Cell * cur = arguments;
		
		while (cur != NULL) {
			
			
			cur = cur->tailAs<Cell>();
		}
		
		return NULL;
	}

	Value * Compiler::lookup (Symbol * identifier) {
		StringT functionName = identifier->value();
		
		llvm::Function* f = m_engine->FindFunctionNamed(functionName.c_str());
		
		if (!f) {
			return NULL;
		}
		
		return new CompiledFunction(reinterpret_cast<EvaluateFunctionT>(m_engine->getPointerToFunction(f)));
	}
			
	Value * Compiler::prototype () {
		return Value::prototype();
	}
			
	void Compiler::toCode (StringStreamT & buffer) {
		buffer << "(compiler)";
	}
	
	void Compiler::import (Table * context) {
		context->update(new Symbol("compiler"), new Compiler);
	}
	
}