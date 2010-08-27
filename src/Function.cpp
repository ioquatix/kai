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
#include "Compiler.h"

#include <iostream>
#include <llvm/LLVMContext.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Function.h>
#include <llvm/Support/raw_os_ostream.h>

#include <llvm/Module.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/Target/TargetData.h>
#include <llvm/Analysis/Verifier.h>

namespace Kai {

#pragma mark -

	llvm::Value * buildTrampoline (std::string name, EvaluateFunctionT function, Frame * frame) {
		Compiler * c = frame->lookupAs<Compiler>(new Symbol("compiler"));
		
		std::vector<const llvm::Type*> argumentTypes;
		llvm::FunctionType * trampolineFunctionType = llvm::FunctionType::get(
			c->valuePointerType(),
			argumentTypes,
			false
		);
		
		argumentTypes.push_back(c->framePointerType());
		
		llvm::FunctionType * baseFunctionType = llvm::FunctionType::get(
			c->valuePointerType(),
			argumentTypes,
			false
		);
		
		llvm::Function * base = dynamic_cast<llvm::Function*>(
			c->module()->getOrInsertFunction(name, baseFunctionType)
		);
		
		ensure(base);
				
		llvm::Function * trampoline = llvm::Function::Create(
			trampolineFunctionType,
			llvm::GlobalValue::PrivateLinkage,
			name + "_trampoline",
			c->module()
		);
		
		// Register the function in the execution engine - this process might be better if we could optimise it.
		// used to be addGlobalMapping(...)
		c->engine()->updateGlobalMapping(base, (void*)function);
		
		// Build the trampoline
		llvm::IRBuilder<> builder(llvm::getGlobalContext());
		llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", trampoline);
		builder.SetInsertPoint(entryBlock);
		
		// http://old.nabble.com/Creating-Pointer-Constants-td22401381.html
		const llvm::Type * intPointerType = c->engine()->getTargetData()->getIntPtrType(llvm::getGlobalContext());
		llvm::Value * framePointer = llvm::ConstantExpr::getIntToPtr(
			llvm::ConstantInt::get(intPointerType, (intptr_t)frame),
			// This could be replaced with the correct type by looking into machine.o
			c->framePointerType()
		);
		
		std::vector<llvm::Value*> arguments;
		arguments.push_back(framePointer);
		llvm::Value * result = builder.CreateCall(base, arguments.begin(), arguments.end(), "bounce");
		
		builder.CreateRet(result);
		
		llvm::verifyFunction(*trampoline);
		
		return llvm::CallInst::Create(trampoline);
	}

	DynamicFunction::DynamicFunction (EvaluateFunctionT function) : m_function(function) {
		
	}
	
	DynamicFunction::~DynamicFunction () {
		
	}

	Value * DynamicFunction::evaluate (Frame * frame) {
		return m_function(frame);
	}

	void DynamicFunction::toCode (StringStreamT & buffer) {
		buffer << "(dynamic-function " << m_function << ")";
	}
}
