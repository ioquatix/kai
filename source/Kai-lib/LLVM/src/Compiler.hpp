//
//  Compiler.h
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 11/07/10.
//  Copyright 2010 Samuel Williams. All rights reserved.
//
//
 
#include "Value.hpp"

#include <llvm/LLVMContext.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/Support/StandardPasses.h>

namespace Kai {
	
	class DynamicFunction;
	class CompiledFunction;
	
	class Compiler : public Value {
		protected:
			llvm::ExecutionEngine * _engine;
			llvm::Module * _module;
			llvm::IRBuilder<> * _builder;
			
			llvm::FunctionPassManager * _functionOptimizer;
			llvm::PassManager * _moduleOptimizer;
			
			const llvm::Type * _frameType;
			const llvm::Type * _valueType;			
		public:
			Compiler ();
			virtual ~Compiler ();
			
			// Returns a compiled function corresponding to the given arguments.
			virtual Object * evaluate (Frame * frame);
			
			// Lookup a compiled function with a given name
			virtual Object * lookup (Symbol * identifier);
			
			// A prototype specifies the behaviour of the current value.
			virtual Object * prototype ();
			
			virtual void to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const;
			
			virtual DynamicFunction * resolve (CompiledFunction *);
			
			llvm::ExecutionEngine * engine ();
			llvm::Module * module ();
			llvm::IRBuilder<> * builder ();
			llvm::FunctionPassManager * functionOptimizer ();
			llvm::PassManager * moduleOptimizer ();
			
			const llvm::Type * framePointerType ();
			const llvm::Type * valuePointerType ();
			
			static void import (Table * context);			
	};
		
	class CompiledValue : public Value {
		protected:
			llvm::Object * _value;
		
		public:
			CompiledValue (llvm::Object * value);
			virtual ~CompiledValue ();
			
			// Returns the contained value.
			llvm::Object * compile (Frame * frame);
			
			virtual void to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const;
			
			virtual Object * prototype ();
			
			static Object * globalPrototype ();
			static void import (Table * context);
	};
	
	class CompiledFunction : public Value {
		public:
			typedef std::vector<llvm::Type*> TypeSignatureT;

		protected:
			friend class Compiler;
			llvm::Function * _code;
					
		public:
			CompiledFunction(llvm::Function * code);
			virtual ~CompiledFunction ();
						
			virtual void to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const;
			
			virtual Object * prototype ();
			
			virtual Object * evaluate (Frame * frame);
			
			virtual llvm::Object * compile (Frame * frame);
			virtual llvm::Object * compiledValue (Frame * frame);
			
			static Object * code(Frame * frame);
			static Object * optimize(Frame * frame);
			static Object * resolve(Frame * frame);
			
			static void import (Table * context);
				
			static Object * globalPrototype ();
	};
	
	class CompiledType : public Value {
		protected:
			const llvm::Type * _type;
		
		public:
			CompiledType (const llvm::Type *);
			virtual ~CompiledType ();
			
			const llvm::Type * value () const;
						
			static Object * globalIntegerPrototype ();
			
			virtual void to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const;
			
			static Object * voidType (Frame * frame);
			
			static Object * intType (Frame * frame);
			static Object * floatType (Frame * frame);
			static Object * functionType (Frame * frame);
			
			static Object * structType (Frame * frame);
			
			static Object * arrayType (Frame * frame);
			static Object * pointerType (Frame * frame);
			static Object * vectorType (Frame * frame);
			
			static void import (Table * context);
	};

}

