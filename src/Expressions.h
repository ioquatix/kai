/*
 *  Parser.h
 *  Kai
 *
 *  Created by Samuel Williams on 11/09/10.
 *  Copyright 2010 Orion Transfer Ltd. All rights reserved.
 *
 */

#ifndef _KAI_INTERPRETER_PARSER_H
#define _KAI_INTERPRETER_PARSER_H

#include "Parser/Parser.h"
#include "Value.h"
#include "SourceCode.h"

#include <list>

namespace Kai {
	
	struct ParseResult {
		enum {
			OKAY = 0,
			FAILED = 1,
			INCOMPLETE = 3
		};
		
		typedef int Status;
		
		ParseResult();
		ParseResult(Parser::Token _token, Value * _value = NULL, Status status = OKAY);

		Parser::Token token;
		Ref<Value> value;
		Status status;
		
		bool isOkay() { return token && status == OKAY; }
		bool isIncomplete() { return status == INCOMPLETE; }
		bool isFailed() { return status == FAILED; }
	};
		
	class IExpressions {
		public:
			virtual ~IExpressions ();
			
			virtual ParseResult parse (IExpressions * top, StringIteratorT begin, StringIteratorT end) abstract;
	};
	
	class Expressions : public Value, public virtual IExpressions {
		protected:
			//typedef traceable_allocator<IExpressions*> AllocatorT;
			typedef std::list<IExpressions*> ParsersT;
			ParsersT m_parsers;
						
		public:
			Expressions ();
			virtual ~Expressions ();
		
			virtual void mark();
			
			static Expressions * basicExpressions ();
			
			ParseResult parse (const SourceCode & code, bool partial = false);
			
			virtual ParseResult parse (IExpressions * top, StringIteratorT begin, StringIteratorT end);
			
			void add (IExpressions * parser);
			
			static Ref<Value> parse (Frame * frame);
			static Ref<Value> globalPrototype ();
			static void import (Table * context);
			virtual Ref<Value> prototype ();
			
			static Expressions * fetch(Frame * frame);
	};
	
	class StringExpression : public Value, virtual public IExpressions {
		public:
			virtual ~StringExpression();
			
			virtual ParseResult parse (IExpressions * top, StringIteratorT begin, StringIteratorT end);
	};
	
	class SymbolExpression : public Value, virtual public IExpressions {
		public:
			virtual ~SymbolExpression();
			
			virtual ParseResult parse (IExpressions * top, StringIteratorT begin, StringIteratorT end);
	};
	
	class ScopeExpression : public Value, virtual public IExpressions {
		protected:
			StringT m_prefix;
			StringT m_function;
			
		public:
			ScopeExpression (StringT prefix, StringT function);
			virtual ~ScopeExpression ();
			
			virtual ParseResult parse (IExpressions * top, StringIteratorT begin, StringIteratorT end);
	};
	
	class OperatorExpression : public Value, virtual public IExpressions {
		protected:
			Parser::OperatorParser m_operators;
			
		public:
			OperatorExpression();
			virtual ~OperatorExpression();
			
			virtual ParseResult parse (IExpressions * top, StringIteratorT begin, StringIteratorT end);
	};
	
	class IntegerExpression : public Value, virtual public IExpressions {
		public:
			virtual ~IntegerExpression();
			
			virtual ParseResult parse (IExpressions * top, StringIteratorT begin, StringIteratorT end);
	};
		
	class CellExpression : public Value, virtual public IExpressions {
		protected:
			StringT m_open, m_close;
			
			CellExpression(StringT open, StringT close);
			virtual Ref<Value> convertToResult (Cell * items);
			
		public:
			CellExpression();
			virtual ~CellExpression();
			
			virtual ParseResult parse (IExpressions * top, StringIteratorT begin, StringIteratorT end);
	};
	
	class ValueExpression : public Value, virtual public IExpressions {
		public:
			virtual ~ValueExpression();
			
			virtual ParseResult parse (IExpressions * top, StringIteratorT begin, StringIteratorT end);
	};
	
	class CallExpression : public CellExpression {
		protected:
			virtual Ref<Value> convertToResult (Cell * items);

		public:
			CallExpression();
			virtual ~CallExpression();
	};
	
	class BlockExpression : public CellExpression {
		protected:
			virtual Ref<Value> convertToResult (Cell * items);

		public:
			BlockExpression();
			virtual ~BlockExpression();
	};
}

#endif
