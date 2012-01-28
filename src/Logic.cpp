//
//  Logic.cpp
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 16/12/11.
//  Copyright (c) 2011 Orion Transfer Ltd. All rights reserved.
//

#include "Logic.h"
#include "Cell.h"
#include "Frame.h"
#include "Table.h"
#include "Symbol.h"
#include "Function.h"

namespace Kai {
	// Builtin Logical Operations
	Ref<Object> Logic::or_ (Frame * frame) {
		Cell * cur = frame->operands();
		
		while (cur != NULL) {
			Ref<Object> value = cur->head()->evaluate(frame);
			
			if (Object::to_boolean(frame, value)) {
				return value;
			}
			
			cur = cur->tail().as<Cell>();
		}
		
		return Symbol::false_symbol(frame);
	}
	
	Ref<Object> Logic::and_ (Frame * frame) {
		Cell * cur = frame->operands();
		
		while (cur != NULL) {
			Ref<Object> value = cur->head()->evaluate(frame);
			
			if (!Object::to_boolean(frame, value)) {
				return Symbol::false_symbol(frame);					
			}
			
			cur = cur->tail().as<Cell>();
		}
		
		return Symbol::true_symbol(frame);		
	}
	
	Ref<Object> Logic::not_ (Frame * frame) {
		Cell * cur = frame->unwrap();
		
		if (cur == NULL)
			throw Exception("Invalid Argument", frame);
		
		if (Object::to_boolean(frame, cur->head())) {
			return Symbol::false_symbol(frame);
		}
		
		return Symbol::true_symbol(frame);
	}
	
	Ref<Object> Logic::when (Frame * frame) {
		ArgumentExtractor args(frame, frame->operands());
		
		Object * value;
		args = args[value];
		
		value = value->evaluate(frame);
		
		while (args) {
			Object * condition, * code;
			
			args[condition][code];
			
			condition = condition->evaluate(frame);
			
			if (Object::compare(value, condition) == EQUAL) {
				return code->evaluate(frame);
			}
		}
		
		return NULL;
	}
	
	Ref<Object> Logic::if_ (Frame * frame) {
		Object * condition, * true_clause, * false_clause;
		
		frame->extract(false)[condition][true_clause][false_clause];
		
		if (Object::to_boolean(frame, condition->evaluate(frame))) {
			return true_clause->evaluate(frame);
		} else {
			if (false_clause)
				return false_clause->evaluate(frame);
			else
				return NULL;
		}
	}
	
	/*
	Ref<Object> Logic::anything_value () {
		return frame->sym("anything");
	}
	
	Ref<Object> Logic::nothing_value () {
		return frame->sym("nothing");
	}
	 
	 */
	
	struct ReturnValue {
		Ref<Object> value;
	};
	
	Ref<Object> Logic::return_ (Frame * frame) {
		Object * result = NULL;
		
		frame->extract()[result];
		
		ReturnValue r = {result};
		
		throw r;
	}
	
	Ref<Object> Logic::block (Frame * frame) {
		Ref<Object> result = NULL;
		
		try {
			Cell * statements = frame->operands();
			
			while (statements != NULL) {
				result = statements->head()->evaluate(frame);
				
				statements = statements->tail().as<Cell>();
			}
		} catch (ReturnValue r) {
			return r.value;
		}
		
		return result;
	}
	
	void Logic::import (Frame * frame) {
		frame->update(frame->sym("or"), KAI_BUILTIN_FUNCTION(Logic::or_));
		frame->update(frame->sym("and"), KAI_BUILTIN_FUNCTION(Logic::and_));
		frame->update(frame->sym("not"), KAI_BUILTIN_FUNCTION(Logic::not_));
		
		frame->update(frame->sym("block"), KAI_BUILTIN_FUNCTION(Logic::block));
		frame->update(frame->sym("return"), KAI_BUILTIN_FUNCTION(Logic::return_));
		
		frame->update(frame->sym("when"), KAI_BUILTIN_FUNCTION(Logic::when));
		frame->update(frame->sym("if"), KAI_BUILTIN_FUNCTION(Logic::if_));
		
		//frame->update(frame->sym("true"), Logic::true_value());
		//frame->update(frame->sym("false"), Logic::false_value());
		
		//frame->update(frame->sym("anything"), Logic::anything_value());
		//frame->update(frame->sym("nothing"), Logic::nothing_value());
	}
}
