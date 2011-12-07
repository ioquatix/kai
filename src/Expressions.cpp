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

namespace Kai {
	
	ParseResult::ParseResult()
		: value(NULL), status(FAILED)
	{
	
	}
	
	ParseResult::ParseResult (Parser::Token _token, Value * _value, Status _status)
		: token(_token), value(_value), status(_status)
	{
	
	}
	
	IExpressions::~IExpressions () {
	
	}
		
	Expressions::Expressions () {
	
	}
	
	Expressions::~Expressions () {
	
	}
	
	Expressions * Expressions::basicExpressions () {
		Expressions * self = new Expressions;

		self->add(new IntegerExpression);		
		self->add(new StringExpression);
		self->add(new SymbolExpression);
		self->add(new OperatorExpression);
		self->add(new CellExpression);
		
		self->add(new ScopeExpression("$", "global-scope"));
		self->add(new ScopeExpression("@", "instance-scope"));
		
		self->add(new ValueExpression);
		self->add(new CallExpression);
		self->add(new BlockExpression);
		
		return self;
	}
	
	ParseResult Expressions::parse (const SourceCode & code, bool partial) {
		StringT::const_iterator begin = code.begin();
		StringT::const_iterator end = code.end();
		
		// Parse value
		ParseResult result = parse(this, begin, end);
		
		if (!partial) {
			if (!result.token) {
				throw Parser::FatalParseFailure(Parser::Token(begin, end), "Could not parse source code");
			}
			
			if (result.status == ParseResult::FAILED) {
				// A more meaningful error would be better.
				throw Parser::FatalParseFailure(result.token, "Invalid Syntax");
			}
			
			// Chomp any remaining whitespace
			begin = Parser::parseWhitespace(result.token.end(), end).end();
			
			if (begin != end) {
				throw Parser::FatalParseFailure(Parser::Token(begin, end), "Could not parse entire source code!");
			}
		}
		
		return result;
	}
	
	ParseResult Expressions::parse (IExpressions * top, StringIteratorT begin, StringIteratorT end) {		
		ParseResult result;
		
		// We start off with the assumption that nothing has parsed correctly.
		ParseResult::Status status = ParseResult::FAILED;
		Parser::Token failedToken;
		
		// Eat whitespace
		begin = Parser::parseWhitespace(begin, end).end();
		
		if (begin < end) {
			for (ParsersT::iterator iter = m_parsers.begin(); iter != m_parsers.end(); iter++) {
				result = (*iter)->parse(this, begin, end);
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
	
	void Expressions::add (IExpressions * parser) {
		m_parsers.push_back(parser);
	}
	
#pragma mark -

	Ref<Value> Expressions::parse (Frame * frame) {
		Expressions * self;
		String * codeString;
		
		frame->extract()[self][codeString];
		
		try {
			SourceCode code("<program>", codeString->value());
			
			return self->parse(code).value;
		} catch (Parser::FatalParseFailure & failure) {
			throw Exception("Could not parse input", codeString, frame);
		}
	}
		
	Ref<Value> Expressions::prototype () {
		return globalPrototype();
	}
	
	Ref<Value> Expressions::globalPrototype () {
		static Table * g_prototype = NULL;
		
		if (!g_prototype) {
			g_prototype = new Table;
			
			g_prototype->update(sym("parse"), KFunctionWrapper(Expressions::parse));
		}
		
		return g_prototype;
	}
	
	void Expressions::import (Table * context) {
		context->update(sym("Expressions"), globalPrototype());
		context->update(sym("expressions"), basicExpressions());
	}
	
	Expressions * Expressions::fetch (Frame * frame) {
		return frame->lookupAs<Expressions>(sym("expressions"));
	}

#pragma mark -

	StringExpression::~StringExpression() {
	
	}
			
	ParseResult StringExpression::parse (IExpressions * top, StringIteratorT begin, StringIteratorT end) {
		Parser::Token token = Parser::parseString(begin, end);
		
		if (token) {
			return ParseResult(token, new String(token.value(), true));
		} else {
			return ParseResult(token);
		}
	}

#pragma mark -

	SymbolExpression::~SymbolExpression() {
	
	}
	
	ParseResult SymbolExpression::parse (IExpressions * top, StringIteratorT begin, StringIteratorT end) {
		Parser::Token token = Parser::parseIdentifier(begin, end);
		
		if (token) {
			return ParseResult(token, sym(token.value()));
		} else {
			return ParseResult(token);
		}
	}
	
#pragma mark -

	ScopeExpression::ScopeExpression(StringT prefix, StringT function)
		: m_prefix(prefix), m_function(function)
	{
	
	}
	
	ScopeExpression::~ScopeExpression() {
	
	}
	
	ParseResult ScopeExpression::parse (IExpressions * top, StringIteratorT begin, StringIteratorT end) {
		Parser::Token token = Parser::parseConstant(begin, end, m_prefix);
		
		if (token) {
			Parser::Token identifier = Parser::parseIdentifier(token.end(), end);
			
			if (identifier) {
				token &= identifier;
				
				return ParseResult(token, Cell::create
					(sym(m_function))
					(sym(identifier.value())->asValue())
				);
			}
		}

		return ParseResult(token);
	}
	
#pragma mark -

	OperatorExpression::OperatorExpression() {
		// Operators must be in longest to shortest order
		m_operators << "==" << "<=>";
		m_operators << "<=" << ">=";
		m_operators << "<<" << ">>";
		m_operators << "<" << ">";
		m_operators << "+" << "-" << "*" << "/" << "%";
		m_operators << "=";
	}

	OperatorExpression::~OperatorExpression() {
	
	}
	
	ParseResult OperatorExpression::parse (IExpressions * top, StringIteratorT begin, StringIteratorT end) {
		Parser::Token token = m_operators(begin, end);
		
		if (token) {
			return ParseResult(token, sym(token.value()));
		} else {
			return ParseResult(token);
		}
	}
	
#pragma mark -

	IntegerExpression::~IntegerExpression() {
	
	}
	
	ParseResult IntegerExpression::parse (IExpressions * top, StringIteratorT begin, StringIteratorT end) {
		Parser::Token token = Parser::parseNumber(begin, end);
		
		if (token) {
			return ParseResult(token, new Integer(Parser::convert<int>(token.value())));
		} else {
			return ParseResult(token);
		}
	}

#pragma mark -

	CellExpression::CellExpression(StringT open, StringT close)
		: m_open(open), m_close(close)
	{
	
	}
			
	CellExpression::CellExpression()
		: m_open("("), m_close(")")
	{
	
	}
	
	Ref<Value> CellExpression::convertToResult (Cell * items)
	{
		return items;
	}

	CellExpression::~CellExpression() {
	
	}
			
	ParseResult CellExpression::parse (IExpressions * top, StringIteratorT begin, StringIteratorT end) {
		Parser::Token t(begin, Parser::CELL);
		Cell * first = NULL, * list = NULL;
		ParseResult::Status status = ParseResult::OKAY;
		
		Parser::Token failedToken;
		
		if (t &= Parser::parseConstant(t.end(), end, m_open)) {
			while (true) {
				// Eat any whitespace before the expression because otherwise if the expression fails to parse we might
				// be left with erroneous whitespace..
				t += Parser::parseWhitespace(t.end(), end);

				ParseResult result = top->parse(top, t.end(), end);
				status |= result.status;
				
				if (result.isOkay()) {
					list = Cell::append(list, result.value, first);
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
				
				t += Parser::parseWhitespace(t.end(), end);
			}
			
			t &= Parser::parseConstant(t.end(), end, m_close);
			
			// If the list is parsed correctly, token will be valid.
			// Otherwise, it is either failed or incomplete.
			if (t)
				status = ParseResult::OKAY;
			
			if (failedToken && status == ParseResult::FAILED) {
				t = failedToken;
			}
			
			return ParseResult(t, convertToResult(first), status);
		}
		
		return ParseResult();
	}

#pragma mark -

	ValueExpression::~ValueExpression() {
	
	}
	
	ParseResult ValueExpression::parse (IExpressions * top, StringIteratorT begin, StringIteratorT end) {
		Parser::Token token = Parser::parseConstant(begin, end, "`");
		if (!token) return ParseResult(token);
		
		ParseResult body = top->parse(top, token.end(), end);
			
		if (body.isOkay()) {
			return ParseResult(body.token, Cell::create(sym("value"))(body.value));
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

	Ref<Value> CallExpression::convertToResult (Cell * items) {
		return Cell::create
			(sym("call"))
			(items->head())
			(Cell::create
				(sym("value"))
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
			
	Ref<Value> BlockExpression::convertToResult (Cell * items)
	{
		return (new Cell(sym("block"), items))->asValue();
	}
}
