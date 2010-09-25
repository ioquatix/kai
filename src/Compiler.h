/*
 *  Compiler.h
 *  Kai
 *
 *  Created by Samuel Williams on 11/07/10.
 *  Copyright 2010 Samuel Williams. All rights reserved.
 *
 */
 
#include "Value.h"

#include <llvm/LLVMContext.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/Support/StandardPasses.h>

namespace Kai {
	
	class DynamicFunction;
	class CompiledFunction;
	
	class Compiler : public Value {
		protected:
			llvm::ExecutionEngine * m_engine;
			llvm::Module * m_module;
			llvm::IRBuilder<> * m_builder;
			
			llvm::FunctionPassManager * m_functionOptimizer;
			llvm::PassManager * m_moduleOptimizer;
			
			const llvm::Type * m_frameType;
			const llvm::Type * m_valueType;			
		public:
			Compiler ();
			virtual ~Compiler ();
			
			// Returns a compiled function corresponding to the given arguments.
			virtual Value * evaluate (Frame * frame);
			
			// Lookup a compiled function with a given name
			virtual Value * lookup (Symbol * identifier);
			
			// A prototype specifies the behaviour of the current value.
			virtual Value * prototype ();
			
			virtual void toCode(StringStreamT & buffer, MarkedT & marks, std::size_t indentation);
			
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
			llvm::Value * m_value;
		
		public:
			CompiledValue (llvm::Value * value);
			virtual ~CompiledValue ();
			
			// Returns the contained value.
			llvm::Value * compile (Frame * frame);
			
			virtual void toCode(StringStreamT & buffer, MarkedT & marks, std::size_t indentation);
			
			virtual Value * prototype ();
			
			static Value * globalPrototype ();
			static void import (Table * context);
	};
	
	class CompiledFunction : public Value {
		public:
			typedef std::vector<llvm::Type*> TypeSignatureT;

		protected:
			friend class Compiler;
			llvm::Function * m_code;
					
		public:
			CompiledFunction(llvm::Function * code);
			virtual ~CompiledFunction ();
						
			virtual void toCode(StringStreamT & buffer, MarkedT & marks, std::size_t indentation);
			
			virtual Value * prototype ();
			
			virtual Value * evaluate (Frame * frame);
			
			virtual llvm::Value * compile (Frame * frame);
			virtual llvm::Value * compiledValue (Frame * frame);
			
			static Value * code(Frame * frame);
			static Value * optimize(Frame * frame);
			static Value * resolve(Frame * frame);
			
			static void import (Table * context);
				
			static Value * globalPrototype ();
	};
	
	class CompiledType : public Value {
		protected:
			const llvm::Type * m_type;
		
		public:
			CompiledType (const llvm::Type *);
			virtual ~CompiledType ();
			
			const llvm::Type * value () const;
						
			static Value * globalIntegerPrototype ();
			
			virtual void toCode(StringStreamT & buffer, MarkedT & marks, std::size_t indentation);
			
			static Value * voidType (Frame * frame);
			
			static Value * intType (Frame * frame);
			static Value * floatType (Frame * frame);
			static Value * functionType (Frame * frame);
			
			static Value * structType (Frame * frame);
			static Value * unionType (Frame * frame);
			
			static Value * arrayType (Frame * frame);
			static Value * pointerType (Frame * frame);
			static Value * vectorType (Frame * frame);
			
			static void import (Table * context);
	};

}

