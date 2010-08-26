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

#include <llvm/DerivedTypes.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Support/IRBuilder.h>


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
		CompiledType * signature;
		Cell * names;
		Value * body;
		
		frame->extract()[signature][names][body];
		
		const llvm::FunctionType * funcType = dynamic_cast<const llvm::FunctionType*>(signature->value());
		
		if (!funcType) {
			throw Exception("Invalid function signature!", signature, frame);
		}
		
		using namespace llvm;
		llvm::Function * func = llvm::Function::Create(
			funcType,
			llvm::GlobalValue::PrivateLinkage,
			"anonymous",
			NULL
		);
		
		for (llvm::Function::arg_iterator i = func->arg_begin(); i != func->arg_end(); i++) {
			Symbol * name = names->headAs<Symbol>();
			
			i->setName(name->value());
			
			names = names->tailAs<Cell>();
		}
		
		llvm::IRBuilder<> builder(llvm::getGlobalContext());
		llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", func);
		builder.SetInsertPoint(entryBlock);
		
		llvm::Value * result = body->compile(&llvm::getGlobalContext());
		
		builder.CreateRet(result);
		
		// Check the function is okay
		llvm::verifyFunction(*func);
		
		return new CompiledFunction(func);
	}

	Value * Compiler::lookup (Symbol * identifier) {
		StringT functionName = identifier->value();
		
		llvm::Function* code = m_engine->FindFunctionNamed(functionName.c_str());
		
		if (!code) {
			return NULL;
		}
		
		return new CompiledFunction(code);
	}
			
	Value * Compiler::prototype () {
		return Value::prototype();
	}
			
	void Compiler::toCode (StringStreamT & buffer) {
		buffer << "(compiler)";
	}
	
	DynamicFunction * Compiler::resolve (CompiledFunction * compiledFunction) {
		void * p = m_engine->getPointerToFunction(compiledFunction->m_code);
		
		if (!p) {
			return NULL;
		}
		
		EvaluateFunctionT function = reinterpret_cast<EvaluateFunctionT>(p);
		
		return new DynamicFunction(function);
	}
	
	void Compiler::import (Table * context) {
		context->update(new Symbol("compiler"), new Compiler);
	}
	
}