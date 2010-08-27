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
#include <llvm/Support/raw_os_ostream.h>

namespace Kai {
	
	Compiler::Compiler ()
	{
		using std::auto_ptr;
		
		static bool s_initialized = false;		
		
		if (!s_initialized) {
			llvm::InitializeNativeTarget();
			//llvm::llvm_start_multithreaded();
			
			s_initialized = true;
		}
		
		llvm::LLVMContext & context = llvm::getGlobalContext();
		
		StringT error;
		auto_ptr<llvm::MemoryBuffer> buffer(llvm::MemoryBuffer::getFile("machine.o"));
		
		if (!buffer.get()) {
			std::cerr << "Could not load machine bitcode!" << std::endl;
			
			return;
		}
		
		m_module = llvm::ParseBitcodeFile(buffer.get(), context, &error);
		buffer.release();
		
		if (!m_module) {
			std::cerr << "Could not create machine module!" << std::endl;
			std::cerr << "*** " << error << std::endl;
			
			return;
		}
		
		m_engine = llvm::ExecutionEngine::createJIT(m_module, &error);
		
		if (!m_engine) {
			std::cerr << "Could not create execution engine!" << std::endl;
			std::cerr << "*** " << error << std::endl;
		}
		
		// Extract some useful type information.
		m_frameType = m_module->getFunction("_frameType")->getReturnType();
		m_valueType = m_module->getFunction("_valueType")->getReturnType();
	}
	
	Compiler::~Compiler () {
		std::cout << "Freeing execution engine" << std::endl;
		
		if (m_engine)
			delete m_engine;
	}
	
	// Returns a compiled function corresponding to the given arguments.
	Value * Compiler::evaluate (Frame * frame) {
		Compiler * c = frame->lookupAs<Compiler>(new Symbol("compiler"));

		Symbol * name;	
		CompiledType * signature;
		Cell * argumentNames;
		Value * body;
		
		frame->extract()(name)(signature)[argumentNames](body);
		
		const llvm::FunctionType * funcType = dynamic_cast<const llvm::FunctionType*>(signature->value());
		
		if (!funcType) {
			throw Exception("Invalid function signature!", signature, frame);
		}
		
		using namespace llvm;
		llvm::Function * func = llvm::Function::Create(
			funcType,
			llvm::GlobalValue::PrivateLinkage,
			name->value(),
			c->module()
		);
		
		Table * arguments = new Table;

		// For recursion
		arguments->update(name, new CompiledValue(func));
		
		for (llvm::Function::arg_iterator arg = func->arg_begin(); arg != func->arg_end(); arg++) {
			Symbol * name = argumentNames->headAs<Symbol>();
			
			arg->setName(name->value());
			
			arguments->update(name, new CompiledValue(&*arg));
			
			argumentNames = argumentNames->tailAs<Cell>();
		}
		
		llvm::IRBuilder<> builder(llvm::getGlobalContext());
		llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", func);
		builder.SetInsertPoint(entryBlock);
		
		Frame * next = new Frame(arguments, frame);
		llvm::Value * result = builder.Insert((llvm::Instruction*)body->compile(next));
		
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
	
	llvm::ExecutionEngine * Compiler::engine () {
		return m_engine;
	}
	
	llvm::Module * Compiler::module () {
		return m_module;
	}
	
	const llvm::Type * Compiler::framePointerType () {
		return m_frameType;
	}
	
	const llvm::Type * Compiler::valuePointerType () {
		return m_valueType;
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
		Compiler * c = new Compiler;
		
		context->update(new Symbol("compiler"), c);
		context->update(new Symbol("Value"), new CompiledType(c->valuePointerType()));
		context->update(new Symbol("Frame"), new CompiledType(c->framePointerType()));
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

#pragma mark -	
	
	CompiledValue::CompiledValue (llvm::Value * value) {
	
	}
	
	CompiledValue::~CompiledValue () {
	
	}

	// Returns the contained value.
	llvm::Value * CompiledValue::compile (Frame * frame) {
		return m_value;
	}
	
	void CompiledValue::toCode (StringStreamT & buffer) {
		buffer << "(compiled-value ";
		
		llvm::raw_os_ostream llvmBuffer(buffer);
		m_value->print(llvmBuffer);
		llvmBuffer.flush();
		
		buffer << ")";
	}

}