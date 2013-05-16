//
//  Compiler.cpp
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 11/07/10.
//  Copyright 2010 Samuel Williams. All rights reserved.
//
//

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
#include <llvm/Assembly/PrintModulePass.h>

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
			//llvm::llv_start_multithreaded();
			
			s_initialized = true;
		}
		
		llvm::LLVMContext & context = llvm::getGlobalContext();
		
		StringT error;
		auto_ptr<llvm::MemoryBuffer> buffer(llvm::MemoryBuffer::getFile("machine.o"));
		
		if (!buffer.get()) {
			std::cerr << "Could not load machine bitcode!" << std::endl;
			_module = new llvm::Module("machine", context);
		} else {
			_module = llvm::ParseBitcodeFile(buffer.get(), context, &error);
			buffer.release();
			
			// Extract some useful type information.
			_frameType = _module->getFunction("_frameType")->getReturnType();
			_valueType = _module->getFunction("_valueType")->getReturnType();
		}
		
		if (!_module) {
			std::cerr << "Could not create machine module!" << std::endl;
			std::cerr << "*** " << error << std::endl;
			
			return;
		}
		
		_engine = llvm::ExecutionEngine::createJIT(_module, &error);
		
		if (!_engine) {
			std::cerr << "Could not create execution engine!" << std::endl;
			std::cerr << "*** " << error << std::endl;
		}
		
		_builder = new llvm::IRBuilder<>(llvm::getGlobalContext());
		
		// Create the function optimiser, optimisation level 3
		_functionOptimizer = new llvm::FunctionPassManager(_module);
		llvm::createStandardFunctionPasses(_functionOptimizer, 3);
		_functionOptimizer->doInitialization();
		
		// Create the module optimiser, optimisation level 3
		_moduleOptimizer = new llvm::PassManager;
		llvm::createStandardModulePasses(_moduleOptimizer,
			1, false, true, true, true, true, NULL
		);
	}
		
	Compiler::~Compiler () {
		std::cout << "Freeing execution engine" << std::endl;
		
		if (_module)
			delete _module;
		
		if (_engine)
			delete _engine;
		
		if (_builder)
			delete _builder;
	}
	
	// Returns a compiled function corresponding to the given arguments.
	Object * Compiler::evaluate (Frame * frame) {
		Compiler * c = frame->lookupAs<Compiler>(frame->sym("compiler"));

		Symbol * name;	
		CompiledType * signature;
		Cell * argumentNames;
		Object * body;
		
		frame->extract()(name)(signature)[argumentNames](body);
		
		const llvm::FunctionType * funcType = dynamic_cast<const llvm::FunctionType*>(signature->value());
		
		if (!funcType) {
			throw Exception("Invalid function signature!", signature, frame);
		}
		
		llvm::Function * func = llvm::Function::Create(
			funcType,
			llvm::Function::ExternalLinkage,
			name->value(),
			c->module()
		);
		
		//func->setCallingConv(llvm::CallingConv::Fast);
		//func->setVisibility(llvm::GlobalObject::HiddenVisibility);
		
		Table * arguments = new Table;

		// For recursion
		arguments->update(name, new CompiledFunction(func));
		
		for (llvm::Function::arg_iterator arg = func->arg_begin(); arg != func->arg_end(); arg++) {
			Symbol * name = argumentNames->head().as<Symbol>();
			
			arg->setName(name->value());
			
			arguments->update(name, new CompiledValue(&*arg));
			
			argumentNames = argumentNames->tail().as<Cell>();
		}
		
		llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", func);
		c->builder()->SetInsertPoint(entryBlock);
		
		Frame * next = new Frame(arguments, frame);
		//std::cout << "Compiling: " << Object::to_string(body) << std::endl;
		llvm::Object * result = body->compile(next);
		
		// Automatically return the last variable
		//if (!c->builder()->GetInsertBlock()->getTerminator() && result) {
		//	c->builder()->CreateRet(result);
		//}
		
		// Check the function is okay
		llvm::verifyFunction(*func);
		
		return new CompiledFunction(func);
	}

	Object * Compiler::lookup (Symbol * identifier) {
		StringT functionName = identifier->value();
		
		llvm::Function* code = _engine->FindFunctionNamed(functionName.c_str());
		
		if (!code) {
			return NULL;
		}
		
		return new CompiledFunction(code);
	}
			
	Object * Compiler::prototype () {
		return Object::prototype();
	}
	
	llvm::ExecutionEngine * Compiler::engine () {
		return _engine;
	}
	
	llvm::Module * Compiler::module () {
		return _module;
	}
	
	llvm::IRBuilder<> * Compiler::builder () {
		return _builder;
	}
	
	llvm::FunctionPassManager * Compiler::functionOptimizer () {
		return _functionOptimizer;
	}
	
	llvm::PassManager * Compiler::moduleOptimizer () {
		return _moduleOptimizer;
	}
	
	const llvm::Type * Compiler::framePointerType () {
		return _frameType;
	}
	
	const llvm::Type * Compiler::valuePointerType () {
		return _valueType;
	}
	
	void Compiler::to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const {
		buffer << "(compiler)";
	}
	
	DynamicFunction * Compiler::resolve (CompiledFunction * compiledFunction) {
		void * p = _engine->getPointerToFunction(compiledFunction->_code);
		
		if (!p) {
			return NULL;
		}
		
		EvaluateFunctionT function = reinterpret_cast<EvaluateFunctionT>(p);
		
		return new DynamicFunction(function);
	}
	
	void Compiler::import (Table * context) {
		Compiler * c = new Compiler;
		
		context->update(frame->sym("compiler"), c);
		
		if (c->valuePointerType())
			context->update(frame->sym("Value"), new CompiledType(c->valuePointerType()));
		
		if (c->framePointerType())
			context->update(frame->sym("Frame"), new CompiledType(c->framePointerType()));
		
		CompiledObject::import(context);
	}

// MARK: -
	
	CompiledFunction::CompiledFunction (llvm::Function * code) : _code(code) {
		
	}
	
	CompiledFunction::~CompiledFunction () {
	
	}
	
	void CompiledFunction::to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const {
		buffer << "(compiled-function ";
		buffer << _code->getNameStr();
		buffer << ")";
	}

	Object * CompiledFunction::prototype () {
		return globalPrototype();
	}
	
	Object * CompiledFunction::evaluate (Frame * frame) {
		Compiler * compiler = dynamic_cast<Kai::Compiler *>(frame->lookup(frame->sym("compiler")));
		typedef int (FuncT)(int, int);
		FuncT * f = (FuncT*)compiler->engine()->getPointerToFunction(_code);
		
		Integer * f1, * f2;
		frame->extract()[f1][f2];
		
		int r = f(f1->value(), f2->value());
		
		return new Integer(r);
	}
	
	llvm::Object * CompiledFunction::compile (Frame * frame)
	{
		Compiler * compiler = dynamic_cast<Kai::Compiler *>(frame->lookup(frame->sym("compiler")));

		std::vector<llvm::Value*> operands;
		Cell * arguments = frame->unwrap();
		
		//std::cerr << "Compiling function " << Object::to_string(arguments) << std::endl;
		
		while (arguments != NULL) {
			llvm::Object * operand = arguments->head()->compile(frame);
			assert(operand != NULL);
			
			operands.push_back(operand);
			
			arguments = arguments->tail().as<Cell>();
		}
		
		llvm::CallInst * call = compiler->builder()->CreateCall(_code, operands.begin(), operands.end());
		//call->setCallingConv(llvm::CallingConv::Fast);
		
		return call;
	}
	
	llvm::Object * CompiledFunction::compiledValue (Frame * frame)
	{
		return _code;
	}
	
	Object * CompiledFunction::code(Frame * frame) {
		CompiledFunction * function = NULL;
		frame->extract()(function);
		
		StringStreamT buffer;
		llvm::raw_os_ostream llvmBuffer(buffer);
		function->compiledValue(frame)->print(llvmBuffer);
		llvmBuffer.flush();
		
		return new String(buffer.str());
	}
	
	Object * CompiledFunction::optimize (Frame * frame) {
		Compiler * compiler = dynamic_cast<Kai::Compiler *>(frame->lookup(frame->sym("compiler")));
		CompiledFunction * function = NULL;
		
		frame->extract()(function);
		
		compiler->functionOptimizer()->run(*(function->_code));
		compiler->moduleOptimizer()->run(*(compiler->module()));
		
		return function;
	}
	
	Object * CompiledFunction::resolve (Frame * frame) {
		CompiledFunction * self = NULL;
		Compiler * compiler = dynamic_cast<Kai::Compiler *>(frame->lookup(frame->sym("compiler")));
		
		frame->extract()(self);
		
		if (!compiler) {
			throw Exception("Could not find compiler!", frame);
		}
		
		return compiler->resolve(self);
	}
	
	void CompiledFunction::import (Table * context) {
		context->update(frame->sym("CompiledFunction"), globalPrototype());
	}
	
	Object * CompiledFunction::globalPrototype () {
		static Ref<Table> g_prototype;
		
		if (!g_prototype) {
			g_prototype = new Table;
			
			g_prototype->update(frame->sym("code"), KAI_BUILTIN_FUNCTION(CompiledFunction::code));
			g_prototype->update(frame->sym("resolve"), KAI_BUILTIN_FUNCTION(CompiledFunction::resolve));
			g_prototype->update(frame->sym("optimize"), KAI_BUILTIN_FUNCTION(CompiledFunction::optimize));
		}
		
		return g_prototype;
	}

// MARK: -
	
	CompiledType::CompiledType (const llvm::Type * ty) : _type(ty) {
	
	}
	
	CompiledType::~CompiledType () {
	
	}
	
	class IntegerEquality : public Value {
		public:
			virtual void to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const {
				buffer << "(builtin-integer-equality)";
			}
			
			virtual Object * evaluate (Frame * frame) {
				return new CompiledValue(this->compile(frame));
			}
			
			virtual llvm::Object * compile (Frame * frame) {
				Compiler * c = frame->lookupAs<Compiler>(frame->sym("compiler"));
				
				Object * lhs, * rhs;
				frame->extract()(lhs)(rhs);

				llvm::Object * result = c->builder()->CreateICmpEQ(lhs->compile(frame), rhs->compile(frame));
				
				return result;
			}
	};
	
	class IntegerModulus : public Value {
		public:
			virtual void to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) {
				buffer << "(builtin-integer-modulus)";
			}
			
			virtual Object * evaluate (Frame * frame) {
				return new CompiledValue(this->compile(frame));
			}
			
			virtual llvm::Object * compile (Frame * frame) {
				// Find the current compiler.
				Compiler * c = frame->lookupAs<Compiler>(frame->sym("compiler"));
				
				// Extract the arguments to the operator.
				Object * lhs, * rhs;
				frame->extract()(lhs)(rhs);
				
				// Return the LLVM remainder instruction with the two arguments.
				llvm::Object * result = c->builder()->CreateURem(lhs->compile(frame), rhs->compile(frame));
				
				return result;
			}
	};
	
	Object * CompiledType::globalIntegerPrototype () {
		static Ref<Table> g_prototype;
		
		if (!g_prototype) {
			g_prototype = new Table;
			g_prototype->set_prototype(Object::globalPrototype());
			
			g_prototype->update(frame->sym("=="), new IntegerEquality);
			g_prototype->update(frame->sym("%"), new IntegerModulus);
		}
		
		return g_prototype;
	}
	
	const llvm::Type * CompiledType::value () const {
		return _type;
	}
	
	void CompiledType::to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const {
		buffer << "(compiled-type ";
		
		llvm::raw_os_ostream llvmBuffer(buffer);
		_type->print(llvmBuffer);
		llvmBuffer.flush();
		
		buffer << ")";
	}
	
	Object * CompiledType::voidType (Frame * frame) {
		return new CompiledType(llvm::Type::getVoidTy(llvm::getGlobalContext()));
	}
	
	Object * CompiledType::intType (Frame * frame) {
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
	
	Object * CompiledType::floatType (Frame * frame) {
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
	
	Object * CompiledType::functionType (Frame * frame) {
		Cell * args = frame->unwrap();
		
		const llvm::Type* returnType = NULL;
		std::vector<const llvm::Type*> argumentTypes;
		bool varargs = false;
		
		while (args != NULL) {
			CompiledType * element = args->head().as<CompiledType>();
			
			if (returnType == NULL)
				returnType = element->value();
			else
				argumentTypes.push_back(element->value());

			args = args->tail().as<Cell>();
		}
		
		if (returnType == NULL) {
			throw Exception("Invalid return type!", frame);
		}
		
		return new CompiledType(llvm::FunctionType::get(returnType, argumentTypes, varargs));
	}
	
	Object * CompiledType::structType (Frame * frame) {
		Cell * args = frame->unwrap();
		std::vector<const llvm::Type*> types;
		bool packed = false;
		
		while (args != NULL) {
			CompiledType * element = args->head().as<CompiledType>();
			
			types.push_back(element->value());
			
			args = args->tail().as<Cell>();
		}
		
		return new CompiledType(llvm::StructType::get(llvm::getGlobalContext(), types, packed));
	}
	
	Object * CompiledType::arrayType (Frame * frame) {
		CompiledType * elementType;
		Integer * numberOfElements;
		
		frame->extract()(elementType)(numberOfElements);
		
		return new CompiledType(llvm::ArrayType::get(elementType->value(), numberOfElements->value()));
	}
	
	Object * CompiledType::pointerType (Frame * frame) {
		CompiledType * to;
		
		frame->extract()(to);
		
		return new CompiledType(to->value()->getPointerTo());
	}
	
	Object * CompiledType::vectorType (Frame * frame) {
		CompiledType * elementType;
		Integer * numberOfElements;
		
		frame->extract()(elementType)(numberOfElements);
		
		return new CompiledType(llvm::VectorType::get(elementType->value(), numberOfElements->value()));
	}
	
	void CompiledType::import (Table * context) {
		context->update(frame->sym("void"), KAI_BUILTIN_FUNCTION(CompiledType::voidType));
		
		context->update(frame->sym("int"), KAI_BUILTIN_FUNCTION(CompiledType::intType));
		context->update(frame->sym("float"), KAI_BUILTIN_FUNCTION(CompiledType::floatType));
		context->update(frame->sym("function"), KAI_BUILTIN_FUNCTION(CompiledType::functionType));
		
		context->update(frame->sym("struct"), KAI_BUILTIN_FUNCTION(CompiledType::structType));
		
		context->update(frame->sym("array"), KAI_BUILTIN_FUNCTION(CompiledType::arrayType));
		context->update(frame->sym("pointer"), KAI_BUILTIN_FUNCTION(CompiledType::pointerType));
		context->update(frame->sym("vector"), KAI_BUILTIN_FUNCTION(CompiledType::vectorType));
	}

// MARK: -
	
	CompiledObject::CompiledValue (llvm::Object * value) : _value(value)  {
	
	}
	
	CompiledObject::~CompiledValue () {
	
	}

	// Returns the contained value.
	llvm::Object * CompiledObject::compile (Frame * frame) {
		return _value;
	}
	
	void CompiledObject::to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const {
		buffer << "(compiled-value ";
		
		llvm::raw_os_ostream llvmBuffer(buffer);
		_value->print(llvmBuffer);
		llvmBuffer.flush();
		
		buffer << ")";
	}
	
	Object * CompiledObject::prototype () {
		return globalPrototype();
	}
	
	class BuiltinLoad : public Value {
	public:
		virtual void to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const {
			buffer << "(builtin-load)";
		}
		
		virtual Object * evaluate (Frame * frame) {
			return new CompiledValue(this->compile(frame));
		}
		
		virtual llvm::Object * compile (Frame * frame) {
			Compiler * c = frame->lookupAs<Compiler>(frame->sym("compiler"));
			
			Object * varExpr;
			frame->extract()(varExpr);
			
			llvm::Object * var = varExpr->compile(frame);
			
			return c->builder()->CreateLoad(var);
		}
	};
	
	class BuiltinStore : public Value {
	public:
		virtual void to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const {
			buffer << "(builtin-store)";
		}
		
		virtual Object * evaluate (Frame * frame) {
			return new CompiledValue(this->compile(frame));
		}
		
		virtual llvm::Object * compile (Frame * frame) {
			Compiler * c = frame->lookupAs<Compiler>(frame->sym("compiler"));
			
			Object * varExpr;
			Object * valueExpr;
			
			frame->extract()(varExpr)(valueExpr);
			
			llvm::Object * var = varExpr->compile(frame);
			llvm::Object * value = valueExpr->compile(frame);
			
			return c->builder()->CreateStore(value, var);
		}
	};
	
	Object * CompiledObject::globalPrototype () {
		static Ref<Table> g_prototype;
		
		if (!g_prototype) {
			g_prototype = new Table;
			g_prototype->set_prototype(Object::globalPrototype());
			
			g_prototype->update(frame->sym("load"), new BuiltinLoad);
			g_prototype->update(frame->sym("store"), new BuiltinStore);
			g_prototype->update(frame->sym("=="), new IntegerEquality);
			g_prototype->update(frame->sym("%"), new IntegerModulus);
		}
		
		return g_prototype;
	}
	
	void CompiledObject::import (Table * context) {
		context->update(frame->sym("CompiledValue"), CompiledObject::globalPrototype());
	}

}
