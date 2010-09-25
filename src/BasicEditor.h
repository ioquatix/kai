/*
 *  BasicEditor.h
 *  Kai
 *
 *  Created by Samuel Williams on 25/09/10.
 *  Copyright 2010 Orion Transfer Ltd. All rights reserved.
 *
 */

#ifndef _KAI_BASIC_EDITOR_H
#define _KAI_BASIC_EDITOR_H

#include "Terminal.h"
#include "Expressions.h"

namespace Kai {
	
	class BasicEditor : virtual public IEditor
	{
		protected:
			Expressions * m_expressions;
			
		public:
			BasicEditor(Value * context);
			virtual ~BasicEditor();
			
			virtual StringT firstPrompt();
			virtual bool isComplete(const StringStreamT & buffer, StringT & prompt);
	};
	
}

#endif