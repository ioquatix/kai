/*
 *  Parser.cpp
 *  Kai
 *
 *  Created by Samuel Williams on 11/09/10.
 *  Copyright 2010 Orion Transfer Ltd. All rights reserved.
 *
 */

#include "Expressions.h"
#include "Frame.h"
#include "Function.h"
#include "Lambda.h"
#include "Table.h"
#include "String.h"
#include "Symbol.h"
#include "Number.h"

namespace Kai {
	
	ParseResult::ParseResult(Status _status) : value(NULL), status(_status) {
	}
	
	ParseResult::ParseResult (Parser::Token _token, Object * _value, Status _status) : token(_token), value(_value), status(_status) {
	}
	
	Expression::~Expression () {
	}
	
	Expressions::Expressions () {
	}
	
	Expressions::~Expressions () {
	}
	
	Ref<Symbol> Expressions::identity(Frame * frame) const {
		return frame->sym("Expressions");
	}
	
	void Expressions::mark(Memory::Traversal * traversal) const {
		for (Expression * expression : _expressions) {
			traversal->traverse(expression);
		}
	}
	
	Ref<Object> Expressions::basic_expressions(Frame * frame) {
		Expressions * self = new(frame) Expressions;

		self->add(new(frame) NumberExpression);
		
		self->add(new(frame) StringExpression);
		self->add(new(frame) SymbolExpression);
		self->add(new(frame) OperatorExpression);
		self->add(new(frame) CellExpression);
		
		self->add(new(frame) ScopeExpression("$", "global-scope"));
		self->add(new(frame) ScopeExpression("@", "instance-scope"));
		
		self->add(new(frame) ValueExpression);
		self->add(new(frame) CallExpression);
		self->add(new(frame) LambdaExpression);
		self->add(new(frame) BlockExpression);
				
		return self;
	}
	
	ParseResult Expressions::parse (Frame * frame, const SourceCode & code, bool partial) {
		StringT::const_iterator begin = code.begin();
		StringT::const_iterator end = code.end();
		
		// Parse value
		ParseResult result = parse(frame, this, begin, end);
		
		if (!partial) {
			if (!result.token) {
				throw Parser::FatalParseFailure(Parser::Token(begin, end), "Could not parse source code");
			}
			
			if (result.status == ParseResult::FAILED) {
				// A more meaningful error would be better.
				throw Parser::FatalParseFailure(result.token, "Invalid Syntax");
			}
			
			// Chomp any remaining whitespace
			begin = Parser::parse_whitespace(result.token.end(), end).end();
			
			if (begin != end) {
				throw Parser::FatalParseFailure(Parser::Token(begin, end), "Could not parse entire source code!");
			}
		}
		
		return result;
	}
	
	ParseResult Expressions::parse(Frame * frame, Expression * top, StringIteratorT begin, StringIteratorT end) {		
		ParseResult result;
		
		// We start off with the assumption that nothing has parsed correctly.
		ParseResult::Status status = ParseResult::FAILED;
		Parser::Token failedToken;
		
		// Eat whitespace
		begin = Parser::parse_whitespace(begin, end).end();
		
		if (begin < end) {
			for (ExpressionsT::iterator iter = _expressions.begin(); iter != _expressions.end(); iter++) {				
				result = (*iter)->parse(frame, this, begin, end);
				status |= result.status;
								
				if (result.isOkay()) {
					// We have successfully parsed the expression.
					status = ParseResult::OKAY;
					break;
				} else if (result.isFailed()) {
					if (!failedToken)
						failedToken = result.token;
				} else if (result.token.end() == end) {
					// We have parsed something but it wasn't successful and parsed all data up to the end.
					// This clause details with expressions which don't return INCOMPLETE.
					status |= ParseResult::INCOMPLETE;
				}
			}
		} else {
			// We are at the end of the current input.
			status |= ParseResult::INCOMPLETE;
		}
		
		result.status = status;
		
		if (result.status == ParseResult::FAILED) {
			result.token = failedToken;
		}
		
		return result;
	}
	
	void Expressions::add (Expression * expression) {
		_expressions.push_back(expression);
	}
	
#pragma mark -

	Ref<Object> Expressions::parse (Frame * frame) {
		Expressions * self;
		String * codeString;
		
		frame->extract()[self][codeString];
		
		try {
			SourceCode code("<program>", codeString->value());
			
			return self->parse(frame, code).value;
		} catch (Parser::FatalParseFailure & failure) {
			throw Exception("Could not parse input", codeString, frame);
		}
	}

	void Expressions::to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const {
		buffer << "(Expressions@" << this << ")";
	}
	
	void Expressions::import (Frame * frame) {
		Table * prototype = new(frame) Table;
		
		prototype->update(frame->sym("parse"), KAI_BUILTIN_FUNCTION(Expressions::parse));
		
		frame->update(frame->sym("Expressions"), prototype);
		
		frame->update(frame->sym("expressions"), basic_expressions(frame));
	}
	
	Expressions * Expressions::fetch (Frame * frame) {
		return frame->lookup(frame->sym("expressions")).as<Expressions>();
	}

#pragma mark -

	StringExpression::~StringExpression() {
	
	}
			
	ParseResult StringExpression::parse(Frame * frame, Expression * top, StringIteratorT begin, StringIteratorT end) {
		Parser::Token token = Parser::parse_string(begin, end);
		
		if (token) {
			return ParseResult(token, new(frame) String(token.value(), true));
		} else {
			return ParseResult(token);
		}
	}

#pragma mark -

	SymbolExpression::~SymbolExpression() {
	
	}
	
	ParseResult SymbolExpression::parse(Frame * frame, Expression * top, StringIteratorT begin, StringIteratorT end) {
		Parser::Token token = Parser::parse_identifier(begin, end);
		
		if (token) {
			return ParseResult(token, frame->sym(token.value().c_str()));
		} else {
			return ParseResult(token);
		}
	}
	
#pragma mark -

	ScopeExpression::ScopeExpression(StringT prefix, StringT function)
		: _prefix(prefix), _function(function)
	{
	
	}
	
	ScopeExpression::~ScopeExpression() {
	
	}
	
	ParseResult ScopeExpression::parse(Frame * frame, Expression * top, StringIteratorT begin, StringIteratorT end) {
		Parser::Token token = Parser::parse_constant(begin, end, _prefix);
		
		if (token) {
			Parser::Token identifier = Parser::parse_identifier(token.end(), end);
			
			if (identifier) {
				token &= identifier;
				
				return ParseResult(token, 
					Cell::create(frame)(frame->sym(_function.c_str()))(frame->sym(identifier.value().c_str())->as_value(frame))
				);
			}
		}

		return ParseResult(token);
	}
	
#pragma mark -

	OperatorExpression::OperatorExpression() {
		// Operators must be in longest to shortest order
		_operators << "==" << "<=>";
		_operators << "<=" << ">=";
		_operators << "<<" << ">>";
		_operators << "<" << ">";
		_operators << "+" << "-" << "*" << "/" << "%" << "^";
		_operators << "=";
	}

	OperatorExpression::~OperatorExpression() {
	
	}
	
	ParseResult OperatorExpression::parse(Frame * frame, Expression * top, StringIteratorT begin, StringIteratorT end) {
		Parser::Token token = _operators(begin, end);
		
		if (token) {
			return ParseResult(token, frame->sym(token.value().c_str()));
		} else {
			return ParseResult(token);
		}
	}
	
#pragma mark -

	NumberExpression::~NumberExpression() {
	
	}
	
	ParseResult NumberExpression::parse(Frame * frame, Expression * top, StringIteratorT begin, StringIteratorT end) {
		Parser::Token token;
		
		token = Parser::parse_hexadecimal(begin, end);
		
		if (token) {
			Math::Integer value(token.value(), 16);
			
			return ParseResult(token, new(frame) Integer(value));
		}
		
		token = Parser::parse_decimal(begin, end);
		
		if (token) {
			Math::Number value(token.value());
			
			return ParseResult(token, new(frame) Number(value));
		}
		
		/*
		token = Parser::parse_integer(begin, end);
		
		if (token) {
			Math::Integer value(token.value(), 10);
			
			return ParseResult(token, new(frame) Integer(value));
		}
		 */
					
		return ParseResult(token);
	}
	
#pragma mark -

	CellExpression::CellExpression(StringT open, StringT close) : _open(open), _close(close), _header(false) {
	
	}
			
	CellExpression::CellExpression() : _open("("), _close(")"), _header(false) {
	
	}
	
	Ref<Object> CellExpression::convert_to_result(Frame * frame, Cell * items) {
		return items;
	}

	CellExpression::~CellExpression() {
	
	}
	
	ParseResult CellExpression::parse_header(Frame * frame, Expression * top, StringIteratorT begin, StringIteratorT end) {
		return ParseResult(ParseResult::FAILED);
	}
	
	ParseResult CellExpression::parse(Frame * frame, Expression * top, StringIteratorT begin, StringIteratorT end) {
		Parser::Token t(begin, Parser::CELL);
		Ref<Cell> first, list;
		
		ParseResult::Status status = ParseResult::OKAY;
		
		Parser::Token failedToken;
		
		if (t &= Parser::parse_constant(t.end(), end, _open)) {
			bool header = _header;
			
			while (true) {
				ParseResult result;
				
				if (header) {
					result = parse_header(frame, top, t.end(), end);
					header = false;
				} else {
					// Eat any whitespace before the expression because otherwise if the expression fails to parse we might
					// be left with erroneous whitespace..
					t += Parser::parse_whitespace(t.end(), end);
					
					result = top->parse(frame, top, t.end(), end);
				}
				
				status |= result.status;
				
				if (result.isOkay()) {
					list = Cell::append(frame, list, result.value, first);
					t << result.token;
				} else {
					if (result.isFailed()) {
						if (result.token)
							// Take the token from the embedded expression.
							failedToken = result.token;
						else
							// Build a token from the failed expression.
							failedToken = Parser::Token(t.end(), end);
					}
				
					break;
				}
				
				t += Parser::parse_whitespace(t.end(), end);
			}
			
			t &= Parser::parse_constant(t.end(), end, _close);
			
			// If the list is parsed correctly, token will be valid.
			// Otherwise, it is either failed or incomplete.
			if (t || status == ParseResult::OKAY) {
				return ParseResult(t, convert_to_result(frame, first), ParseResult::OKAY);
			} else {
				return ParseResult(failedToken, NULL, status);
			}
		}
		
		return ParseResult();
	}

#pragma mark -

	ValueExpression::~ValueExpression() {
	
	}
	
	ParseResult ValueExpression::parse(Frame * frame, Expression * top, StringIteratorT begin, StringIteratorT end) {
		Parser::Token token = Parser::parse_constant(begin, end, "`");
		if (!token) return ParseResult(token);
		
		ParseResult body = top->parse(frame, top, token.end(), end);
			
		if (body.isOkay()) {
			return ParseResult(body.token, Cell::create(frame)(frame->sym("value"))(body.value));
		}
		
		return body;
	}
	
#pragma mark -

	CallExpression::CallExpression()
		: CellExpression("[", "]")
	{
	
	}

	CallExpression::~CallExpression() {
	
	}

	Ref<Object> CallExpression::convert_to_result(Frame * frame, Cell * items) {
		return Cell::create(frame)
			(frame->sym("call"))
			(items->head())
			(Cell::create(frame)
				(frame->sym("value"))
				(items->tail())
			);
	}
	
#pragma mark -

	BlockExpression::BlockExpression()
		: CellExpression("{", "}")
	{
	
	}

	BlockExpression::~BlockExpression()
	{
	
	}
			
	Ref<Object> BlockExpression::convert_to_result(Frame * frame, Cell * items)
	{
		return new(frame) Cell(frame->sym("block"), items);
	}

#pragma mark -
	
	LambdaExpression::LambdaExpression() {
		_header = true;
	}
	
	LambdaExpression::~LambdaExpression() {
		
	}
	
	ParseResult LambdaExpression::parse_header(Frame * frame, Expression * top, StringIteratorT begin, StringIteratorT end) {
		SymbolExpression symbol_expression;
		
		Parser::Token t(begin, Parser::CELL);
		Ref<Cell> first, list;
		
		if (t &= Parser::parse_constant(t.end(), end, "|")) {
			while (1) {
				Parser::Token q = Parser::parse_constant(t.end(), end, "|");
				
				if (q) {
					t &= q;
					
					return ParseResult(t, first);
				} else {
					ParseResult result = symbol_expression.parse(frame, top, t.end(), end);
					
					if (result.isOkay()) {
						list = Cell::append(frame, list, result.value, first);
						t << result.token;
					} else {
						// Is this the correct behaviour to return a partial failure?
						result.status = ParseResult::FAILED;
						
						return result;
					}
				}
				
				t += Parser::parse_whitespace(t.end(), end);
			}
		} else {
			return ParseResult();
		}
	}
	
	Ref<Object> LambdaExpression::convert_to_result(Frame * frame, Cell * items) {
		KAI_ENSURE(items);
		
		Object * arguments = items->head();
		Object * body = items->tail();
		
		if (arguments != NULL) {
			arguments = arguments->as_value(frame);
		}
		
		if (body != NULL) {
			body = (new(frame) Cell(frame->sym("block"), body))->as_value(frame);
		}
		
		return Cell::create(frame)(frame->sym("lambda"))(arguments)(body);
	}
}
