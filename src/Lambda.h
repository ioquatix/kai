//
//  Lambda.h
//  Kai
//
//  Created by Samuel Williams on 27/09/11.
//  Copyright 2011 Orion Transfer Ltd. All rights reserved.
//

/*
 *  Value.h
 *  This file is part of the "Kai" project, and is licensed under the GNU GPLv3.
 *
 *  Created by Samuel Williams on 10/04/10.
 *  Copyright 2010 Samuel Williams. All rights reserved.
 *
 */
 
#ifndef _KLAMBDA_H
#define _KLAMBDA_H

#include "Value.h"

namespace Kai {

#pragma mark -
#pragma mark Lambda

	class Lambda : public Value {
		protected:
			Ref<Frame> m_scope;
			Ref<Cell> m_arguments;
			Ref<Cell> m_code;
			
		public:
			Lambda (Frame * scope, Cell * arguments, Cell * code);
			virtual ~Lambda ();
			
			virtual Ref<Value> evaluate (Frame * frame);
			
			virtual void toCode(StringStreamT & buffer, MarkedT & marks, std::size_t indentation);
			
			static Ref<Value> lambda (Frame * frame);
			static void import (Table *);
	};

}

#endif
