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
 
#ifndef _KAI_LAMBDA_H
#define _KAI_LAMBDA_H

#include "Object.h"
#include "Cell.h"

namespace Kai {

#pragma mark -
#pragma mark Lambda

	class Lambda : public Object {
	protected:
		Frame * _scope;
		Cell * _arguments;
		Cell * _code;
		
		bool _macro;
	
	public:
		Lambda(Frame * scope, Cell * arguments, Cell * code);
		virtual ~Lambda();
	
		virtual Ref<Symbol> identity(Frame * frame) const;
	
		bool is_macro() const { return _macro; }
		void set_macro(bool macro) { _macro = macro; }
	
		virtual void mark(Memory::Traversal * traversal) const;
		
		virtual Ref<Object> evaluate(Frame * frame);
		
		virtual void to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const;
		
		static Ref<Object> to_macro(Frame * frame);
		
		static Ref<Object> is_macro(Frame * frame);
		static Ref<Object> is_function(Frame * frame);
		
		static Ref<Object> lambda(Frame * frame);
		static Ref<Object> macro(Frame * frame);
	
		virtual Ref<Object> prototype(Frame * frame);
		
		static void import(Frame *);
	};

}

#endif
