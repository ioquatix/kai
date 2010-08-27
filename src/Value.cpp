/*
 *  Value.cpp
 *  This file is part of the "Kai" project, and is licensed under the GNU GPLv3.
 *
 *  Created by Samuel Williams on 10/04/10.
 *  Copyright 2010 Samuel Williams. All rights reserved.
 *
 */

#include "Value.h"
#include <assert.h>
#include <gc/gc_cpp.h>
#include <iostream>

#include "Frame.h"
#include "Exception.h"
#include "Function.h"
#include "Compiler.h"
#include "Ensure.h"

// For String constructor
#include "Parser/Strings.h"

#include <llvm/LLVMContext.h>
#include <llvm/Type.h>
#include <llvm/Constants.h>

namespace Kai {

	void debug (Value * value) {
		std::cerr << Value::toString(value) << std::endl;
	}

#pragma mark -
#pragma mark Value

	template <typename ThisT>
	inline static int derivedCompare (ThisT * lhs, Value * rhs) {
		ThisT * other = dynamic_cast<ThisT *>(rhs);

		if (other) {
			return lhs->compare(other);
		} else {
			throw InvalidComparison();
		}
	}

	inline int clampComparison (int result) {
		if (result < 0) {
			return -1;
		} else if (result > 0) {
			return 1;
		} else {
			return 0;
		}
	}

	Value::~Value () {

	}
	
	Value * Value::lookup (Symbol * identifier) {
		Value * proto = prototype();
		
		if (proto) {
			return proto->lookup(identifier);
		}
		
		return NULL;
	}
	
	Value * Value::prototype () {
		return globalPrototype();
	}

	int Value::compare (Value * other) {
		throw InvalidComparison();
	}

	void Value::debug () {
		std::cerr << toString(this) << std::endl;
	}

	Value * Value::evaluate (Frame * frame) {
		return this;
	}
	
	llvm::Value * Value::compile (Frame * frame) {
		// Return a new trampoline which will do the default evaluation of this.
		Cell * message = Cell::create()(new Symbol("block"))(this);
		
		// Build stack frame
		Frame * next = new Frame(NULL, message, frame);
		
		// Get the function called block
		Value * function = frame->lookup(new Symbol("block"));
		ensure(function != NULL);
		
		// This should return an appropriate trampoline
		return function->compile(next);
	}

	StringT Value::toString (Value * value) {
		if (value) {
			StringStreamT buffer;
			value->toCode(buffer);
			return buffer.str();
		} else {
			return "nil";
		}
	}
	
	bool Value::toBoolean (Value * value) {
		if (value) {
			return true;
		}
		
		return false;
	}
	
	int Value::compare (Value * lhs, Value * rhs) {
		if (lhs && rhs) {
			// Both values are non-NULL
			return lhs->compare(rhs);
		} else {
			// One or both values are NULL
			if (lhs == rhs) {
				return COMPARISON_EQUAL;
			} else if (lhs) {
				return COMPARISON_DESCENDING;
			} else if (rhs) {
				return COMPARISON_ASCENDING;
			}
		}
		
		// We should never get here
		throw InvalidComparison();
	}

#pragma mark Builtin Functions

	void Value::import (Table * context) {
		context->update(new Symbol("Value"), globalPrototype());
	}
	
	Value * Value::globalPrototype () {
		static Table * g_prototype = NULL;
		
		if (!g_prototype) {
			g_prototype = new Table;
			
			g_prototype->update(new Symbol("toString"), KFunctionWrapper(Value::toString));
			g_prototype->update(new Symbol("toBoolean"), KFunctionWrapper(Value::toBoolean));
			g_prototype->update(new Symbol("compare"), KFunctionWrapper(Value::compare));
			g_prototype->update(new Symbol("equal"), KFunctionWrapper(Value::equal));
			g_prototype->update(new Symbol("prototype"), KFunctionWrapper(Value::prototype));
			g_prototype->update(new Symbol("value"), KFunctionWrapper(Value::value));
					
			g_prototype->update(new Symbol("lookup"), KFunctionWrapper(Value::lookup));
			g_prototype->update(new Symbol("call"), KFunctionWrapper(Value::call));
		}
		
		return g_prototype;
	}
	
	Value * Value::call (Frame * frame) {
		/*
			((lookup 'Table 'set) (this) 'call (lambda '(object function) '(block
				((cell
					(wrap (lookup object (head function)))
					(cell object (tail function))
				))
			)))
		*/
		
		Value * self;
		Cell * body;
		
		frame->extract()(self)(body);
		
		// Wrap self so we can pass it to other functions
		self = Cell::create()(new Symbol("value"))(self);
		
		Symbol * functionName = body->headAs<Symbol>();
		
		//std::cerr << "Calling " << Value::toString(functionName) << " for " << Value::toString(self) << std::endl;
		
		Cell * dispatch = Cell::create()
			(new Symbol("lookup"))
			(self)
			(functionName);
		
		Cell * arguments = new Cell(self, body->tail());
		
		Cell * call = new Cell(dispatch, arguments);
		
		return call->evaluate(frame);
	}
	
	Value * Value::lookup (Frame * frame) {		
		Cell * cur = frame->operands();
		Value * value = NULL;
		
		while (cur != NULL) {
			if (cur->head() == NULL) {
				throw Exception("Invalid Name", cur, frame);
			}
			
			value = cur->head()->evaluate(frame);
			
			Cell * tail = cur->tailAs<Cell>();
			if (!tail) break;
			
			if (value == NULL) {
				throw Exception("Null Scope", cur->head(), frame);
			}
			
			frame = new Frame(value, frame);
			cur = tail;
		}
		
		return value;
	}
	
	Value * Value::toString (Frame * frame) {
		Value * value;
		
		frame->extract()[value];
		
		return new String(Value::toString(value));
	}
	
	Value * Value::toBoolean (Frame * frame) {
		Value * value;
		
		frame->extract()[value];
		
		if (Value::toBoolean(value)) {
			return Symbol::trueSymbol();
		} else {
			return Symbol::falseSymbol();
		}
	}
	
	Value * Value::compare (Frame * frame) {
		Value * lhs, * rhs;
		
		frame->extract()[lhs][rhs];
		
		ComparisonResult c = COMPARISON_EQUAL;
		
		try {
			c = Value::compare(lhs, rhs);
		} catch (InvalidComparison) {
			throw Exception("Invalid Comparison", frame);
		}
		
		return new Integer(c);
	}
	
	Value * Value::equal (Frame * frame) {
		Value * lhs, * rhs;
		
		frame->extract()[lhs][rhs];
		
		ComparisonResult c = COMPARISON_EQUAL;
		
		try {
			c = Value::compare(lhs, rhs);
		} catch (InvalidComparison) {
			throw Exception("Invalid Comparison", frame);
		}
		
		if (c == COMPARISON_EQUAL) {
			return new Symbol("true");
		} else {
			return NULL;
		}
	}
	
	Value * Value::prototype (Frame * frame) {
		Value * value = NULL;
		
		frame->extract()[value];
		
		return value->prototype();
	}
	
	Value * Value::value (Frame * frame) {
		if (frame->operands()) {
			return frame->operands()->head();
		}
		
		return NULL;
	}

#pragma mark -
#pragma mark Cell

	Cell::Cell (Value * head, Value * tail) : m_head(head), m_tail(tail) {
		
	}

	Cell::~Cell () {

	}

	Cell * Cell::insert (Value * value) {
		Cell * next = new Cell(value, this->m_tail);
		
		m_tail = next;
		
		return next;
	}

	Cell * Cell::append (Value * value) {
		Cell * end = this;
		
		while (end != NULL) {
			Cell * next = dynamic_cast<Cell*>(end->m_tail);
			
			if (next)
				end = next;
			else
				break;
		}
		
		assert(end != NULL);
		
		return end->insert(value);
	}

	unsigned Cell::count () {
		Cell * end = this;
		
		unsigned result = 0;
		
		while (end != NULL) {
			end = dynamic_cast<Cell*>(end->m_tail);
			
			result += 1;
		}
		
		return result;
	}

	int Cell::compare (Value * other) {
		return derivedCompare(this, other);
	}

	int Cell::compare (Cell * other) {
		int result = Value::compare(m_head, other->m_head);
		
		if (result == 0) {
			return Value::compare(m_tail, other->m_tail);
		} else {
			return result;
		}
	}

	void Cell::toCode (StringStreamT & buffer) {
		buffer << '(';
		
		Cell * cur = this;
		
		while (cur) {
			if (cur->m_head)
				cur->m_head->toCode(buffer);
			else
				buffer << "nil";
			
			if (cur->m_tail) {
				buffer << ' ';
				
				Cell * next = dynamic_cast<Cell*>(cur->m_tail);
				
				if (next) {
					cur = next;
				} else {
					cur->m_tail->toCode(buffer);
					cur = NULL;
				}
			} else {
				break;
			}
		}
		
		buffer << ')';
	}

	Value * Cell::evaluate (Frame * frame) {
		return frame->call(NULL, this);
	}

	Value * Cell::_new (Frame * frame) {
		Value * head = NULL;
		Value * tail = NULL;
		
		frame->extract()[head][tail];
		
		return new Cell(head, tail);
	}
	
	Value * Cell::head (Frame * frame) {
		Cell * cell;
		
		frame->extract()[cell];
		
		if (!cell) {
			throw Exception("Invalid Argument", frame);
		}
		
		return cell->head();
	}
	
	Value * Cell::tail (Frame * frame) {
		Cell * cell;
		
		frame->extract()[cell];
		
		if (!cell) {
			throw Exception("Invalid Argument", frame);
		}
		
		return cell->tail();
	}
	
	Value * Cell::prototype () {
		return globalPrototype();
	}
	
	Value * Cell::each (Frame * frame) {
		Cell * cell;
		Value * callback;
		
		frame->extract()[cell][callback];
		
		while (cell != NULL) {
			Cell * message = Cell::create()(callback)(cell);
			message->evaluate(frame);
			
			cell = cell->tailAs<Cell>();
		}
		
		return NULL;
	}
	
	Value * Cell::globalPrototype () {
		static Table * g_prototype = NULL;
		
		if (!g_prototype) {
			g_prototype = new Table;
			
			g_prototype->update(new Symbol("new"), KFunctionWrapper(Cell::_new));
			g_prototype->update(new Symbol("each"), KFunctionWrapper(Cell::each));
			g_prototype->update(new Symbol("head"), KFunctionWrapper(Cell::head));
			g_prototype->update(new Symbol("tail"), KFunctionWrapper(Cell::tail));
		}
		
		return g_prototype;
	}
	
	void Cell::import (Table * context) {
		context->update(new Symbol("Cell"), globalPrototype());
	}

#pragma mark -
#pragma mark String

	String::String (const StringT & value, bool unescape) : m_value(value) {
		if (unescape) {
			m_value = Parser::unescapeString(m_value);
		}
	}

	String::~String () {

	}

	int String::compare (Value * other) {
		return derivedCompare(this, other);
	}

	int String::compare (String * other) {
		return m_value.compare(other->m_value);
	}

	void String::toCode (StringStreamT & buffer) {
		buffer << '"';
		
		buffer << m_value;
		
		buffer << '"';
	}

#pragma mark -
#pragma mark Symbol

	int Symbol::calculateHash (const StringT & string) {
		int sum = 0;
		
		StringT::const_iterator c = string.begin();
		const StringT::const_iterator end = string.end();
		
		while (c != end) {
			sum += *c;
			
			++c;
		}
		
		return sum;
	}

	Symbol::Symbol (const StringT & value) : m_value(value), m_hash(calculateHash(value)) {

	}

	Symbol::~Symbol () {

	}

	int Symbol::compare (Value * other) {
		return derivedCompare(this, other);
	}

	int Symbol::compare (Symbol * other) {
		int result = clampComparison(m_hash - other->m_hash);
		
		if (result == 0) {
			return m_value.compare(other->m_value);
		}		
		
		return result;
	}

	void Symbol::toCode (StringStreamT & buffer) {
		buffer << m_value;
	}

	Value * Symbol::evaluate (Frame * frame) {
		if (m_value[0] != ':') {
			return frame->lookup(this);
		} else {
			return this;
		}
	}

	Symbol * Symbol::nilSymbol () {
		return new Symbol("nil");
	}
	
	Symbol * Symbol::falseSymbol () {
		return new Symbol("false");
	}
	
	Symbol * Symbol::trueSymbol () {
		return new Symbol("true");
	}

#pragma mark -
#pragma mark Integer

	Integer::Integer (int value) : m_value(value) {
	}

	Integer::~Integer () {
	}
	
	llvm::Value * Integer::compile (Frame * context) {
		return llvm::ConstantInt::get(
			(const llvm::Type*)llvm::Type::getInt32Ty(llvm::getGlobalContext()), 
			llvm::APInt(sizeof(m_value) * 8, m_value, true)
		);
	}
	
	Value * Integer::sum (Frame * frame) {
		int total = 0;
		
		Cell * args = frame->unwrap();
		
		while (args != NULL) {
			Integer * integer = args->headAs<Integer>();
			
			if (integer) {
				total += integer->value();
			} else {
				throw Exception("Invalid Integer Value", frame);
			}
			
			args = args->tailAs<Cell>();
		}
		
		return new Integer(total);
	}
	
	Value * Integer::product (Frame * frame) {
		int total = 1;
		
		Cell * args = frame->unwrap();
		
		while (args != NULL) {
			Integer * integer = args->headAs<Integer>();
			
			if (integer) {
				total *= integer->value();
			} else {
				throw Exception("Invalid Integer Value", frame);
			}
			
			args = args->tailAs<Cell>();
		}
		
		return new Integer(total);
	}
	
	Value * Integer::modulus (Frame * frame) {
		Integer * number, * base;
		
		frame->extract()[number][base];
		
		return new Integer(number->value() / base->value());
	}
	
	Value * Integer::globalPrototype () {
		static Table * g_prototype = NULL;
		
		if (!g_prototype) {
			g_prototype = new Table;
			
			g_prototype->update(new Symbol("+"), KFunctionWrapper(Integer::sum));
			g_prototype->update(new Symbol("*"), KFunctionWrapper(Integer::product));
			g_prototype->update(new Symbol("%"), KFunctionWrapper(Integer::modulus));
		}
		
		return g_prototype;
	}
	
	Value * Integer::prototype () {		
		return globalPrototype();
	}

	int Integer::compare (Value * other) {
		return derivedCompare(this, other);
	}

	int Integer::compare (Integer * other) {
		return clampComparison(other->m_value - m_value);
	}

	void Integer::toCode (StringStreamT & buffer) {
		buffer << m_value;
	}
	
	void Integer::import (Table * context) {
		context->update(new Symbol("Integer"), globalPrototype());
	}

#pragma mark -
#pragma mark Table

	Table::Table (int size) : m_size(size) {
		assert(size >= 1);
		
		m_bins = (Bin**)GC_malloc(sizeof(Bin*) * size);
	}

	Table::~Table () {

	}
	
	Table::Table (int size, bool allocate) {
		assert(size >= 1);
		
		m_bins = (Bin**)(this + sizeof(Table));
	}

	Table * Table::allocate (int size) {	
		Table * table = (Table*)GC_malloc(sizeof(Table) + sizeof(Bin*) * size);
		
		return new(table) Table(size, false);
	}

	Table::Bin * Table::find (Symbol * key) {
		assert(key != NULL);
		
		Bin * bin = m_bins[key->hash() % m_size];
		
		while (bin != NULL) {
			if (key->compare(bin->key) == 0) {
				return bin;
			}
			
			bin = bin->next;
		}

		return NULL;
	}

	Value * Table::update (Symbol * key, Value * value) {
		assert(key != NULL);
		assert(value != NULL);
			
		unsigned index = key->hash() % m_size;
		
		Bin * bin = m_bins[index];
		
		if (bin) {
			while (1) {
				if (key->compare(bin->key) == 0) {
					Value * old = bin->value;
					
					bin->value = value;
					
					return old;
				}
				
				if (bin->next == NULL)
					break;
				
				bin = bin->next;
			}
		}
		
		Bin * next = new Bin;
		
		next->next = NULL;
		next->key = key;
		next->value = value;
		
		if (bin) {
			bin->next = next;
		} else {
			m_bins[index] = next;
		}
		
		return NULL;
	}

	Value * Table::remove (Symbol * key) {
		assert(key != NULL);
		
		// The place where we should write the pointer to the next value
		Bin ** next = &m_bins[key->hash() % m_size];
		Bin * bin = *next;
		
		while (bin != NULL) {
			if (key->compare(bin->key) == 0) {
				*next = bin->next;
			}
			
			next = &bin->next;
			bin = bin->next;
		}

		return NULL;
	}

	int Table::compare (Value * other) {
		return derivedCompare(this, other);
	}

	int Table::compare (Table * other) {
		throw InvalidComparison();
	}

	void Table::toCode (StringStreamT & buffer) {
		buffer << "(table";
		
		for (unsigned i = 0; i < m_size; i += 1) {
			Bin * bin = m_bins[i];
			
			while (bin != NULL) {
				buffer << " '";
				bin->key->toCode(buffer);
				buffer << " ";
				bin->value->toCode(buffer);
				
				bin = bin->next;
			}
		}
		
		buffer << ")";
	}

	Value * Table::lookup (Symbol * key) {
		Bin * bin = find(key);
		
		if (bin) {
			return bin->value;
		}
		
		if (m_prototype)
			return m_prototype->lookup(key);

		return NULL;
	}
	
	void Table::setPrototype (Value * prototype) {
		m_prototype = prototype;
	}
	
	Value * Table::prototype () {
		return m_prototype;
	}
	
#pragma mark Builtins
	
	Value * Table::table (Frame * frame) {
		Table * table = new Table;
		Cell * args = frame->unwrap();
		
		while (args) {
			Symbol * key = NULL;
			Value * value = NULL;
			
			args = args->extract(frame)[key][value];
			
			if (key == NULL) {
				throw Exception("Invalid Key", frame);
			}
			
			table->update(key, value);
		}
		
		return table;
	}

	Value * Table::update (Frame * frame) {
		Table * table = NULL;
		Symbol * key = NULL;
		Value * value = NULL;
		
		frame->extract()[table][key][value];
		
		if (table == NULL) {
			throw Exception("Invalid Target", frame);
		}
		
		if (key == NULL) {
			throw Exception("Invalid Key", frame);
		}
		
		std::cerr << "Updating " << Value::toString(key) << " to " << Value::toString(value) << std::endl;
		
		if (value == NULL)
			return table->remove(key);
		else
			return table->update(key, value);
	}
	
	Value * Table::lookup (Frame * frame) {
		Table * table = NULL;
		Symbol * key = NULL;
		
		frame->extract()[table][key];
		
		if (table == NULL) {
			throw Exception("Invalid Target", frame);
		}
		
		if (key == NULL) {
			throw Exception("Invalid Key", frame);
		}
		
		return table->lookup(key);
	}
	
	Value * Table::each (Frame * frame) {
		Table * table;
		Value * callback;
		
		frame->extract()[table][callback];
		
		for (unsigned i = 0; i < table->m_size; i += 1) {
			Bin * cur = table->m_bins[i];
			
			while (cur != NULL) {
				Cell * message = Cell::create()(callback)(cur->key)(cur->value);
				message->evaluate(frame);
				
				cur = cur->next;
			}
		}
		
		return NULL;
	}
	
	Value * Table::setPrototype (Frame * frame) {
		Table * table = NULL;
		Value * prototype = NULL;
		
		frame->extract()(table)(prototype);
		
		table->setPrototype(prototype);
		
		return NULL;
	}
	
	Value * Table::globalPrototype () {
		static Table * g_prototype = NULL;
		
		if (!g_prototype) {
			g_prototype = new Table;
			g_prototype->setPrototype(Value::globalPrototype());
			
			g_prototype->update(new Symbol("new"), KFunctionWrapper(Table::table));
			g_prototype->update(new Symbol("set"), KFunctionWrapper(Table::update));
			g_prototype->update(new Symbol("get"), KFunctionWrapper(Table::lookup));
			g_prototype->update(new Symbol("each"), KFunctionWrapper(Table::each));
			g_prototype->update(new Symbol("prototype="), KFunctionWrapper(Table::setPrototype));
		}
		
		return g_prototype;
	}
	
	void Table::import (Table * context) {
		context->update(new Symbol("Table"), Table::globalPrototype());
	}
	
#pragma mark -
#pragma mark Lambda

	Lambda::Lambda (Value * scope, Cell * arguments, Cell * code)
		: m_scope(scope), m_arguments(arguments), m_code(code) {
		
	}
	
	Lambda::~Lambda () {
		
	}
	
	Value * Lambda::evaluate (Frame * frame) {
		Table * locals = new Table;
		locals->setPrototype(m_scope);
		
		Cell * names = m_arguments;
		Cell * values = frame->unwrap();
		while (names != NULL) {
			if (values == NULL) {
				throw Exception("Lambda Arity Mismatch", frame);
			}
			
			locals->update(names->headAs<Symbol>(), values->head());
			
			names = names->tailAs<Cell>();
			values = values->tailAs<Cell>();
		}
		
		Frame * next = new Frame(locals, frame);
		
		return m_code->evaluate(next);
	}
	
	void Lambda::toCode (StringStreamT & buffer) {
		buffer << "(lambda ";
		
		m_arguments->toCode(buffer);
		
		buffer << " ";
		
		m_code->toCode(buffer);
		
		buffer << ")";
	}
	
	Value * Lambda::lambda (Frame * frame) {
		Cell * arguments, * code;
		
		frame->extract()[arguments][code];
		
		if (arguments == NULL) {
			throw Exception("Invalid Argument List", frame);
		}
		
		if (code == NULL) {
			throw Exception("Invalid Lambda Body", frame);
		}
		
		return new Lambda(frame->scope(), arguments, code);
	}
	
	void Lambda::import (Table * context) {
		context->update(new Symbol("lambda"), KFunctionWrapper(Lambda::lambda));
	}
	
#pragma mark -
#pragma mark Logic

	// Builtin Logical Operations
	Value * Logic::or_ (Frame * frame) {
		Cell * cur = frame->operands();
		
		while (cur != NULL) {
			Value * value = cur->head()->evaluate(frame);
			
			if (Value::toBoolean(value)) {
				return value;
			}
			
			cur = cur->tailAs<Cell>();
		}
		
		return Symbol::falseSymbol();
	}
	
	Value * Logic::and_ (Frame * frame) {
		Cell * cur = frame->operands();
		
		while (cur != NULL) {
			Value * value = cur->head()->evaluate(frame);
			
			if (!Value::toBoolean(value)) {
				return Symbol::falseSymbol();					
			}
			
			cur = cur->tailAs<Cell>();
		}
		
		return Symbol::trueSymbol();		
	}
	
	Value * Logic::not_ (Frame * frame) {
		Cell * cur = frame->unwrap();
		
		if (cur == NULL)
			throw Exception("Invalid Argument", frame);
		
		if (Value::toBoolean(cur->head())) {
			return Symbol::falseSymbol();
		}
		
		return Symbol::trueSymbol();
	}

	Value * Logic::when (Frame * frame) {
		Cell::ArgumentExtractor args(frame, frame->operands());
		
		Value * value;
		args = args[value];
		
		value = value->evaluate(frame);
		
		while (args) {
			Value * condition, * code;
			
			args[condition][code];
			
			condition = condition->evaluate(frame);
			
			if (Value::compare(value, condition) == COMPARISON_EQUAL) {
				return code->evaluate(frame);
			}
		}
		
		return NULL;
	}
	
	Value * Logic::if_ (Frame * frame) {
		Value * condition, * trueClause, * falseClause;
		
		frame->extract(false)[condition][trueClause][falseClause];
		
		if (Value::toBoolean(condition->evaluate(frame))) {
			return trueClause->evaluate(frame);
		} else {
			if (falseClause)
				return falseClause->evaluate(frame);
			else
				return NULL;
		}
	}
	
	Value * Logic::trueValue () {
		return NULL;
	}
	
	Value * Logic::falseValue () {
		return NULL;
	}
	
	Value * Logic::anythingValue () {
		return NULL;
	}
	
	Value * Logic::nothingValue () {
		return NULL;
	}
	
	struct ReturnValue {
		Value * value;
	};
	
	Value * Logic::block (Frame * frame) {
		Value * result = NULL;
		
		try {
			Cell * statements = frame->operands();
			
			while (statements != NULL) {
				result = statements->head()->evaluate(frame);
				
				statements = statements->tailAs<Cell>();
			}
		} catch (ReturnValue r) {
			return r.value;
		}
		
		return result;
	}
	
	Value * Logic::return_ (Frame * frame) {
		ReturnValue r = {frame->unwrap()};
		
		throw r;
	}
	
	void Logic::import (Table * context) {
		context->update(new Symbol("or"), KFunctionWrapper(Logic::or_));
		context->update(new Symbol("and"), KFunctionWrapper(Logic::and_));
		context->update(new Symbol("not"), KFunctionWrapper(Logic::not_));

		context->update(new Symbol("block"), KFunctionWrapper(Logic::block));
		context->update(new Symbol("return"), KFunctionWrapper(Logic::return_));

		context->update(new Symbol("when"), KFunctionWrapper(Logic::when));
		context->update(new Symbol("if"), KFunctionWrapper(Logic::if_));
	}
}