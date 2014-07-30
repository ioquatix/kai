//
//  InterpolationExpression.h
//  Kai
//
//  Created by Samuel Williams on 3/06/12.
//  Copyright (c) 2012 Orion Transfer Ltd. All rights reserved.
//

#ifndef _KAI_EXPRESSIONS_INTERPOLATION_EXPRESSION_H
#define _KAI_EXPRESSIONS_INTERPOLATION_EXPRESSION_H

#include "Expressions.hpp"

namespace Kai {
	namespace Parser {
		class InterpolationExpression : public Expression {			
		public:
			static void import(Frame * frame);
			
			InterpolationExpression();
			virtual ~InterpolationExpression();
			
			virtual ParseResult parse(Frame * Frame, const ParseState & state) const;
		};
	}
}

#endif
