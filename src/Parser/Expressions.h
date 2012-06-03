//
//  Parser.h
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 11/09/10.
//  Copyright 2010 Orion Transfer Ltd. All rights reserved.
//
//

#ifndef _KAI_PARSER_EXPRESSIONS_H
#define _KAI_PARSER_EXPRESSIONS_H

#include "../Parser/Parser.h"
#include "../Object.h"
#include "../Cell.h"
#include "../SourceCode.h"

#include <list>

namespace Kai {
	namespace Parser {
	
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
			
			bool is_okay() { return token && status == OKAY; }
			bool is_incomplete() { return status == INCOMPLETE; }
			bool is_failed() { return status == FAILED; }
		};
		
		class Expression;
		
		struct ParseState {
			const SourceCode * code;
			
			const Expression * top;
			const StringIteratorT begin, current, end;
			
			ParseState next(StringIteratorT next) const {
				ParseState state = {code, top, begin, next, end};
				
				return state;
			}
			
			ParseState next(StringIteratorT next, Expression * expression) const {
				ParseState state = {code, expression, begin, next, end};
				
				return state;
			}
		};
		
		class Expression : public Object {
		public:
			static const char * const NAME;
			
			virtual ~Expression();
			
			virtual ParseResult parse(Frame * frame, const ParseState & state) const abstract;
		};
		
		class Expressions : public Expression {
		protected:
			typedef std::list<const Expression*> ExpressionsT;
			ExpressionsT _expressions;
			
			bool _ignore_whitespace;
			
		public:
			Expressions(bool ignore_whitespace = true);
			virtual ~Expressions();
			
			virtual Ref<Symbol> identity(Frame * frame) const;
			
			virtual void mark(Memory::Traversal *) const;
			
			ParseResult parse(Frame * frame, const SourceCode * code, bool partial = false);
			
			virtual ParseResult parse(Frame * frame, const ParseState & state) const;
			
			void add(const Expression * expression);
			
			static Ref<Object> parse(Frame * frame);
			static Ref<Object> basic_expressions(Frame * frame);
			
			virtual void to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const;
			
			static void import(Frame * frame);
			
			static Expressions * fetch(Frame * frame);
		};
		
		class StringExpression : public Expression {
		public:
			virtual ~StringExpression();
			
			virtual ParseResult parse(Frame * frame, const ParseState & state) const;
		};
		
		class SymbolExpression : public Expression {
		public:
			virtual ~SymbolExpression();
			
			virtual ParseResult parse(Frame * frame, const ParseState & state) const;
		};
		
		class ScopeExpression : public Expression {
		protected:
			StringT _prefix;
			StringT _function;
			
		public:
			ScopeExpression (StringT prefix, StringT function);
			virtual ~ScopeExpression ();
			
			virtual ParseResult parse(Frame * frame, const ParseState & state) const;
		};
		
		class OperatorExpression : public Expression {
		protected:
			Parser::OperatorParser _operators;
			
		public:
			OperatorExpression();
			virtual ~OperatorExpression();
			
			virtual ParseResult parse(Frame * frame, const ParseState & state) const;
		};
		
		class NumberExpression : public Expression {
		public:
			virtual ~NumberExpression();
			
			virtual ParseResult parse(Frame * frame, const ParseState & state) const;
		};
		
		class CellExpression : public Expression {
		protected:
			StringT _open, _close;
			bool _header;
			
			CellExpression(StringT open, StringT close);
			virtual Ref<Object> convert_to_result(Frame * frame, Cell * items) const;
			
			virtual ParseResult parse_header(Frame * frame, const ParseState & state) const;
		public:
			CellExpression();
			virtual ~CellExpression();
			
			virtual ParseResult parse(Frame * frame, const ParseState & state) const;
		};
		
		class ValueExpression : public Expression {
		public:
			virtual ~ValueExpression();
			
			virtual ParseResult parse(Frame * frame, const ParseState & state) const;
		};
		
		class CallExpression : public CellExpression {
		protected:
			virtual Ref<Object> convert_to_result(Frame * frame, Cell * items) const;
			
		public:
			CallExpression();
			virtual ~CallExpression();
		};
		
		class BlockExpression : public CellExpression {
		protected:
			virtual Ref<Object> convert_to_result(Frame * frame, Cell * items) const;
			
		public:
			BlockExpression();
			virtual ~BlockExpression();
		};
		
		class LambdaExpression : public BlockExpression {
		protected:
			virtual Ref<Object> convert_to_result(Frame * frame, Cell * items) const;
			
			virtual ParseResult parse_header(Frame * frame, const ParseState & state) const;
			
		public:
			LambdaExpression();
			virtual ~LambdaExpression();
		};
		
		/*
		 Example:
		 (trace `<-Godzilla
		 Hello,
		 World.
		 Godzilla)
		 */
		
		class HeredocExpression : public Expression {
		protected:
			
		public:
			HeredocExpression();
			virtual ~HeredocExpression();
			
			virtual ParseResult parse(Frame * frame, const ParseState & state) const;
		};
	}
}

#endif
