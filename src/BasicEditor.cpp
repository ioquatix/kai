/*
 *  BasicEditor.cpp
 *  Kai
 *
 *  Created by Samuel Williams on 25/09/10.
 *  Copyright 2010 Orion Transfer Ltd. All rights reserved.
 *
 */

#include "BasicEditor.h"
#include "Value.h"
#include "Frame.h"
#include "Ensure.h"

namespace Kai {

	BasicEditor::BasicEditor(Value * context)
	{
		Frame * frame = new Frame(context);
		m_expressions = Expressions::fetch(frame);
		
		ensure(m_expressions != NULL);
	}
	
	BasicEditor::~BasicEditor()
	{
	}
	
	StringT BasicEditor::firstPrompt()
	{
		return "kai> ";
	}
	
	bool BasicEditor::isComplete(const StringStreamT & buffer, StringT & prompt)
	{
		ParseResult result;
		
		SourceCode code("<editor>", buffer.str());
		prompt = "";
		
		try {
			result = m_expressions->parse(code, true);
		} catch (Parser::FatalParseFailure & ex) {
			return false;
		}
		
		if (result.isIncomplete()) {
			return false;
		} else {
			// FAILED or OKAY
			return true;
		}
	}

}
