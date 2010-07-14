/*
 *  Compiler.cpp
 *  Kai
 *
 *  Created by Samuel Williams on 11/07/10.
 *  Copyright 2010 Samuel Williams. All rights reserved.
 *
 */

#include "Compiler.h"

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
		
		auto_ptr<llvm::ExecutionEngine> engine(llvm::ExecutionEngine::createJIT(module.get(), &error));
		module.release();
		
		if (!engine.get()) {
			std::cerr << "Could not create execution engine!" << std::endl;
			std::cerr << "*** " << error << std::endl;
			
			return;
		}

		llvm::Function* func = engine->FindFunctionNamed("test");
		
		if (!func) {
			std::cerr << "Could not load test function!" << std::endl;
			
			return;
		}

		typedef void (*TestFunc)();
		TestFunc testFunc = reinterpret_cast<TestFunc>(engine->getPointerToFunction(func));
		testFunc();
	}
	
	Compiler::~Compiler () {
	
	}
	
	// Returns a compiled function corresponding to the given arguments.
	Value * Compiler::evaluate (Frame * frame) {
		
	}	

	Value * Compiler::lookup (Symbol * identifier) {
		return NULL;
	}
			
	Value * Compiler::prototype () {
		return Value::prototype();
	}
			
	void Compiler::toCode (StringStreamT & buffer) {
		buffer << "(compiler)";
	}
	
}