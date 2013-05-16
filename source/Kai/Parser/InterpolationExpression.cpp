//
//  InterpolationExpression.cpp
//  Kai
//
//  Created by Samuel Williams on 3/06/12.
//  Copyright (c) 2012 Orion Transfer Ltd. All rights reserved.
//

#include "InterpolationExpression.h"

#include "../String.h"

namespace Kai {
	namespace Parser {
		
		// [interpolate parse "Hello <% foo %> World"]
		void InterpolationExpression::import(Frame * frame) {
			Ref<Expressions> interpolation = new(frame) Expressions(false);

			Ref<Expression> inner = frame->lookup(frame->sym("expressions"));
			
			interpolation->add(new(frame) InterpolationExpression);
			
			frame->update(frame->sym("interpolate"), interpolation);
		}
		
		InterpolationExpression::InterpolationExpression() {
			
		}
		
		InterpolationExpression::~InterpolationExpression() {
			
		}
		
		static bool is_not_body_character(Unicode::CodePointT code_point) {
			return code_point != '<' && code_point != '%';
		}
		
		static Parser::Token parse_body(StringIteratorT begin, StringIteratorT end) {
			Token segment(begin);
			StringIteratorT current = begin;
			
			while (1) {
				// Check if we got to the end:
				if (current == end) {
					Token body(begin, current, Parser::TEXT_BLOCK);
					
					if (body.length() > 0)
						segment << body;
					
					return segment;
				}
				
				{
					// Check for %(expressions):
					Parser::Token marker = Parser::parse_constant(current, end, "%");
					
					if (marker) {
						Token body(begin, current, Parser::TEXT_BLOCK);
						marker.set_identity(Parser::EXPRESSION_MARKER);
						
						segment << body;
						segment << marker;
						
						return segment;
					}
				}

				{
					// Check for <% code %> expressions:
					Parser::Token marker = Parser::parse_constant(current, end, "<%");
					
					if (marker) {
						Token body(begin, current, Parser::TEXT_BLOCK);
						marker.set_identity(Parser::BODY_MARKER);
						
						if (body.length() > 0)
							segment << body;
						
						segment << marker;
						
						return segment;
					}
				}
				
				// Move to the next character
				Unicode::next(current, end);
			}
		}
		
		// e.g. [interpolate parse "foo %bar %baz bog"]
		ParseResult InterpolationExpression::parse(Frame * frame, const ParseState & state) const {
			// We are parsing text starting from the initial character - while parsing text we can encounter two types of symbols:
			//		  text-expression := (^ | '%>') text-body* ('<%' | $)
			//              text-body := '%' expression / text-character
			
			Parser::Token top(state.begin);
			Ref<Expression> inner = frame->lookup(frame->sym("expressions"));

			// Code generation:
			Ref<Cell> first, list;
			
			if (state.current == state.begin || (top += Parser::parse_constant(top.end(), state.end, "%>"))) {
				StringIteratorT current = top.end();
				
				while (current != state.end) {
					Parser::Token token = parse_body(current, state.end);
					KAI_ENSURE(token.length() != 0);
					
					//token.debug();
					
					for (auto & child : token.children()) {
						top << child;
						
						switch (child.identity()) {
							case Parser::TEXT_BLOCK: {
								Ref<String> text_string = new(frame) String(child.value());
								list = Cell::append(frame, list, text_string, first);
								
								std::cerr << "Text Block: " << child.value() << std::endl;
							} break;
							
							case Parser::EXPRESSION_MARKER: {
								ParseResult inner_result = inner->parse(frame, state.next(child.end(), inner));
								
								if (!inner_result.is_okay())
									return inner_result;
								
								list = Cell::append(frame, list, inner_result.value, first);
								
								std::cerr << "Expression: " << Object::to_string(frame, inner_result.value) << std::endl;
								
								top << inner_result.token;
								
								current = inner_result.token.end();
							} break;
							
							case Parser::BODY_MARKER: {
								// If we get to <% we have now finished parsing the "interpolation text" and need to start parsing expressions.
								return ParseResult(top);
							} break;
							
							default: {
								std::cerr << "Error while parsing interpolation:";
								child.debug();
							} break;
						}
					}
					
					current = top.end();
				}
				
				// If we got to the end, we can successfully parse this interpolation
				if (current == state.end) {
					return ParseResult(top, first);
				} else {
					return ParseResult(top, first, ParseResult::INCOMPLETE);
				}
			}
			
			return ParseResult();
		}	
	}
}
