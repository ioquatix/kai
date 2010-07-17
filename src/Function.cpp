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
#include <llvm/LLVMContext.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Support/raw_os_ostream.h>

namespace Kai {

	CompiledFunction::CompiledFunction (EvaluateFunctionT function) : m_function(function) {
		
	}
	
	CompiledFunction::~CompiledFunction () {
		
	}

	Value * CompiledFunction::evaluate (Frame * frame) {
		return m_function(frame);
	}

	void CompiledFunction::toCode (StringStreamT & buffer) {
		buffer << "(compiled-function " << m_function << ")";
	}
	
	CompiledType::CompiledType (const llvm::Type * ty) : m_type(ty) {
	
	}
	
	CompiledType::~CompiledType () {
	
	}
	
	const llvm::Type * CompiledType::value () const {
		return m_type;
	}
	
	void CompiledType::toCode (StringStreamT & buffer) {
		buffer << "(compiled-type ";
		
		llvm::raw_os_ostream llvmBuffer(buffer);
		m_type->print(llvmBuffer);
		llvmBuffer.flush();
		
		buffer << ")";
	}
	
	Value * CompiledType::voidType (Frame * frame) {
		return new CompiledType(llvm::Type::getVoidTy(llvm::getGlobalContext()));
	}
	
	Value * CompiledType::intType (Frame * frame) {
		Integer * bitSize;
		
		frame->extract()(bitSize);
		
		switch (bitSize->value()) {
			case 1:
				return new CompiledType(llvm::Type::getInt1Ty(llvm::getGlobalContext()));
			case 8:
				return new CompiledType(llvm::Type::getInt8Ty(llvm::getGlobalContext()));
			case 16:
				return new CompiledType(llvm::Type::getInt16Ty(llvm::getGlobalContext()));
			case 32:
				return new CompiledType(llvm::Type::getInt32Ty(llvm::getGlobalContext()));
			case 64:
				return new CompiledType(llvm::Type::getInt64Ty(llvm::getGlobalContext()));
		}

		throw Exception("Invalid primative integer width!", bitSize, frame);
	}
	
	Value * CompiledType::floatType (Frame * frame) {
		Integer * bitSize;
		
		frame->extract()(bitSize);
		
		switch (bitSize->value()) {
			case 32:
				return new CompiledType(llvm::Type::getFloatTy(llvm::getGlobalContext()));
			case 64:
				return new CompiledType(llvm::Type::getDoubleTy(llvm::getGlobalContext()));
			case 128:
				return new CompiledType(llvm::Type::getFP128Ty(llvm::getGlobalContext()));
		}

		throw Exception("Invalid primative float width!", bitSize, frame);
	}
	
	Value * CompiledType::pointerType (Frame * frame) {
		CompiledType * to;
		
		frame->extract()(to);
		
		return new CompiledType(to->value()->getPointerTo());
	}
	
	void CompiledType::import (Table * context) {
		context->update(new Symbol("void"), KFunctionWrapper(CompiledType::voidType));
		context->update(new Symbol("int"), KFunctionWrapper(CompiledType::intType));
		context->update(new Symbol("float"), KFunctionWrapper(CompiledType::floatType));
		context->update(new Symbol("pointer"), KFunctionWrapper(CompiledType::pointerType));
	}

}
