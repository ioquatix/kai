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

namespace Kai {

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
	
#pragma mark -
	
	CompiledFunction::CompiledFunction (llvm::Function * code) : m_code(code) {
		
	}
	
	CompiledFunction::~CompiledFunction () {
	
	}
	
	void CompiledFunction::toCode (StringStreamT & buffer) {
		buffer << "(compiled-function ";
		
		llvm::raw_os_ostream llvmBuffer(buffer);
		m_code->print(llvmBuffer);
		llvmBuffer.flush();
		
		//buffer << m_code->getNameStr();
		
		buffer << ")";
	}

	Value * CompiledFunction::prototype () {
		return globalPrototype();
	}
	
	Value * CompiledFunction::resolve (Frame * frame) {
		CompiledFunction * self = NULL;
		Compiler * compiler = dynamic_cast<Kai::Compiler *>(frame->lookup(new Symbol("compiler")));
		
		frame->extract()(self);
		
		if (!compiler) {
			throw Exception("Could not find compiler!", frame);
		}
		
		return compiler->resolve(self);
	}
	
	void CompiledFunction::import (Table * context) {
		context->update(new Symbol("CompiledFunction"), globalPrototype());
	}
	
	Value * CompiledFunction::globalPrototype () {
		static Table * g_prototype = NULL;
		
		if (!g_prototype) {
			g_prototype = new Table;
			
			g_prototype->update(new Symbol("resolve"), KFunctionWrapper(CompiledFunction::resolve));
		}
		
		return g_prototype;
	}

#pragma mark -
	
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
		
		unsigned nbits = bitSize->value();
		
		if (nbits >= llvm::IntegerType::MIN_INT_BITS && nbits <= llvm::IntegerType::MAX_INT_BITS) {
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
				default:
					return new CompiledType(llvm::IntegerType::get(llvm::getGlobalContext(), bitSize->value()));
			}
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
	
	Value * CompiledType::functionType (Frame * frame) {
		Cell * args = frame->unwrap();
		
		const llvm::Type* returnType = NULL;
		std::vector<const llvm::Type*> argumentTypes;
		bool varargs = false;
		
		while (args != NULL) {
			CompiledType * element = args->headAs<CompiledType>();
			
			if (returnType == NULL)
				returnType = element->value();
			else
				argumentTypes.push_back(element->value());

			args = args->tailAs<Cell>();
		}
		
		if (returnType == NULL) {
			throw Exception("Invalid return type!", frame);
		}
		
		return new CompiledType(llvm::FunctionType::get(returnType, argumentTypes, varargs));
	}
	
	Value * CompiledType::structType (Frame * frame) {
		Cell * args = frame->unwrap();
		std::vector<const llvm::Type*> types;
		bool packed = false;
		
		while (args != NULL) {
			CompiledType * element = args->headAs<CompiledType>();
			
			types.push_back(element->value());
			
			args = args->tailAs<Cell>();
		}
		
		return new CompiledType(llvm::StructType::get(llvm::getGlobalContext(), types, packed));
	}
	
	Value * CompiledType::unionType (Frame * frame) {
		Cell * args = frame->unwrap();
		std::vector<const llvm::Type*> types;
		
		while (args != NULL) {
			CompiledType * element = args->headAs<CompiledType>();
			
			types.push_back(element->value());
			
			args = args->tailAs<Cell>();
		}
		
		return new CompiledType(llvm::UnionType::get(&types[0], types.size()));
	}
	
	Value * CompiledType::arrayType (Frame * frame) {
		CompiledType * elementType;
		Integer * numberOfElements;
		
		frame->extract()(elementType)(numberOfElements);
		
		return new CompiledType(llvm::ArrayType::get(elementType->value(), numberOfElements->value()));
	}
	
	Value * CompiledType::pointerType (Frame * frame) {
		CompiledType * to;
		
		frame->extract()(to);
		
		return new CompiledType(to->value()->getPointerTo());
	}
	
	Value * CompiledType::vectorType (Frame * frame) {
		CompiledType * elementType;
		Integer * numberOfElements;
		
		frame->extract()(elementType)(numberOfElements);
		
		return new CompiledType(llvm::VectorType::get(elementType->value(), numberOfElements->value()));
	}
	
	void CompiledType::import (Table * context) {
		context->update(new Symbol("void"), KFunctionWrapper(CompiledType::voidType));
		
		context->update(new Symbol("int"), KFunctionWrapper(CompiledType::intType));
		context->update(new Symbol("float"), KFunctionWrapper(CompiledType::floatType));
		context->update(new Symbol("function"), KFunctionWrapper(CompiledType::functionType));
		
		context->update(new Symbol("struct"), KFunctionWrapper(CompiledType::structType));
		context->update(new Symbol("union"), KFunctionWrapper(CompiledType::unionType));
		
		context->update(new Symbol("array"), KFunctionWrapper(CompiledType::arrayType));
		context->update(new Symbol("pointer"), KFunctionWrapper(CompiledType::pointerType));
		context->update(new Symbol("vector"), KFunctionWrapper(CompiledType::vectorType));
	}

}
