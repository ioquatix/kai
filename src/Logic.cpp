//
//  Logic.cpp
//  Kai
//
//  Created by Samuel Williams on 16/12/11.
//  Copyright (c) 2011 Orion Transfer Ltd. All rights reserved.
//

#include "Logic.h"
#include "Frame.h"
#include "Function.h"

namespace Kai {
	// Builtin Logical Operations
	Ref<Value> Logic::or_ (Frame * frame) {
		Cell * cur = frame->operands();
		
		while (cur != NULL) {
			Ref<Value> value = cur->head()->evaluate(frame);
			
			if (Value::toBoolean(value)) {
				return value;
			}
			
			cur = cur->tailAs<Cell>();
		}
		
		return Symbol::falseSymbol();
	}
	
	Ref<Value> Logic::and_ (Frame * frame) {
		Cell * cur = frame->operands();
		
		while (cur != NULL) {
			Ref<Value> value = cur->head()->evaluate(frame);
			
			if (!Value::toBoolean(value)) {
				return Symbol::falseSymbol();					
			}
			
			cur = cur->tailAs<Cell>();
		}
		
		return Symbol::trueSymbol();		
	}
	
	Ref<Value> Logic::not_ (Frame * frame) {
		Cell * cur = frame->unwrap();
		
		if (cur == NULL)
			throw Exception("Invalid Argument", frame);
		
		if (Value::toBoolean(cur->head())) {
			return Symbol::falseSymbol();
		}
		
		return Symbol::trueSymbol();
	}
	
	Ref<Value> Logic::when (Frame * frame) {
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
	
	Ref<Value> Logic::if_ (Frame * frame) {
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
	
	Ref<Value> Logic::trueValue () {
		return NULL;
	}
	
	Ref<Value> Logic::falseValue () {
		return NULL;
	}
	
	Ref<Value> Logic::anythingValue () {
		return NULL;
	}
	
	Ref<Value> Logic::nothingValue () {
		return NULL;
	}
	
	struct ReturnValue {
		Ref<Value> value;
	};
	
	Ref<Value> Logic::return_ (Frame * frame) {
		Value * result = NULL;
		
		frame->extract()[result];
		
		ReturnValue r = {result};
		
		throw r;
	}
	
	Ref<Value> Logic::block (Frame * frame) {
		Ref<Value> result = NULL;
		
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
	
	void Logic::import (Table * context) {
		context->update(sym("or"), KFunctionWrapper(Logic::or_));
		context->update(sym("and"), KFunctionWrapper(Logic::and_));
		context->update(sym("not"), KFunctionWrapper(Logic::not_));
		
		context->update(sym("block"), KFunctionWrapper(Logic::block));
		context->update(sym("return"), KFunctionWrapper(Logic::return_));
		
		context->update(sym("when"), KFunctionWrapper(Logic::when));
		context->update(sym("if"), KFunctionWrapper(Logic::if_));
	}
}
