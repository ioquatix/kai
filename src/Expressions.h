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
#include "Object.h"
#include "Cell.h"
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
		
		ParseResult(Status status = FAILED);
		ParseResult(Parser::Token _token, Object * _value = NULL, Status status = OKAY);

		Parser::Token token;
		Ref<Object> value;
		Status status;
		
		bool isOkay() { return token && status == OKAY; }
		bool isIncomplete() { return status == INCOMPLETE; }
		bool isFailed() { return status == FAILED; }
	};
	
	class Expression : public Object {
	public:
		static const char * const NAME;
		
		virtual ~Expression();
		
		virtual ParseResult parse(Frame * frame, Expression * top, StringIteratorT begin, StringIteratorT end) abstract;
	};
	
	class Expressions : public Expression {
	protected:
		typedef std::list<Expression*> ExpressionsT;
		ExpressionsT _expressions;
					
	public:
		Expressions();
		virtual ~Expressions();
		
		virtual Ref<Symbol> identity(Frame * frame) const;
	
		virtual void mark(Memory::Traversal *) const;
		
		ParseResult parse(Frame * frame, const SourceCode & code, bool partial = false);
		
		virtual ParseResult parse(Frame * frame, Expression * top, StringIteratorT begin, StringIteratorT end);
		
		void add (Expression * expression);
		
		static Ref<Object> parse(Frame * frame);
		static Ref<Object> basic_expressions(Frame * frame);
		
		virtual void to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const;
		
		static void import(Frame * frame);
		
		static Expressions * fetch(Frame * frame);
	};
	
	class StringExpression : public Expression {
		public:
			virtual ~StringExpression();
			
			virtual ParseResult parse(Frame * frame, Expression * top, StringIteratorT begin, StringIteratorT end);
	};
	
	class SymbolExpression : public Expression {
		public:
			virtual ~SymbolExpression();
			
			virtual ParseResult parse(Frame * frame, Expression * top, StringIteratorT begin, StringIteratorT end);
	};
	
	class ScopeExpression : public Expression {
		protected:
			StringT _prefix;
			StringT _function;
			
		public:
			ScopeExpression (StringT prefix, StringT function);
			virtual ~ScopeExpression ();
			
			virtual ParseResult parse(Frame * frame, Expression * top, StringIteratorT begin, StringIteratorT end);
	};
	
	class OperatorExpression : public Expression {
		protected:
			Parser::OperatorParser _operators;
			
		public:
			OperatorExpression();
			virtual ~OperatorExpression();
			
			virtual ParseResult parse(Frame * frame, Expression * top, StringIteratorT begin, StringIteratorT end);
	};
	
	class NumberExpression : public Expression {
		public:
			virtual ~NumberExpression();
			
			virtual ParseResult parse(Frame * frame, Expression * top, StringIteratorT begin, StringIteratorT end);
	};
		
	class CellExpression : public Expression {
		protected:
			StringT _open, _close;
			bool _header;
			
			CellExpression(StringT open, StringT close);
			virtual Ref<Object> convert_to_result(Frame * frame, Cell * items);
			
			virtual ParseResult parse_header(Frame * frame, Expression * top, StringIteratorT begin, StringIteratorT end);
		public:
			CellExpression();
			virtual ~CellExpression();
			
			virtual ParseResult parse(Frame * frame, Expression * top, StringIteratorT begin, StringIteratorT end);
	};
	
	class ValueExpression : public Expression {
		public:
			virtual ~ValueExpression();
			
			virtual ParseResult parse(Frame * frame, Expression * top, StringIteratorT begin, StringIteratorT end);
	};
	
	class CallExpression : public CellExpression {
		protected:
			virtual Ref<Object> convert_to_result(Frame * frame, Cell * items);

		public:
			CallExpression();
			virtual ~CallExpression();
	};
	
	class BlockExpression : public CellExpression {
		protected:
			virtual Ref<Object> convert_to_result(Frame * frame, Cell * items);

		public:
			BlockExpression();
			virtual ~BlockExpression();
	};
	
	class LambdaExpression : public BlockExpression {
		protected:
			virtual Ref<Object> convert_to_result(Frame * frame, Cell * items);
			
			virtual ParseResult parse_header(Frame * frame, Expression * top, StringIteratorT begin, StringIteratorT end);

		public:
			LambdaExpression();
			virtual ~LambdaExpression();
	};
}

#endif
