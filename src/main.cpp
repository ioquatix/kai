/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <gc/gc.h>
#include <stdio.h>

#include "Cell.h"
#include "Parser.h"
#include "Frame.h"
#include "Function.h"
#include "Exception.h"
#include "Terminal.h"
#include "SourceCode.h"

namespace {

	using namespace Kai;

	Value * runCode (Table * context, SourceCode & code, int & result) {
		try {
			Frame frame(context);
		
			Cell * value = Parser::parse(code.buffer());
		
			Value * result = value->evaluate(&frame);
			
			return result;
		} catch (Exception & ex) {
			std::cerr << "Exception : " << ex.what() << std::endl;
			
			ex.top()->debug();
		}
	
		return NULL;
	}
	
	Table * buildContext () {
		Table * context = new Table;
		
		context->update(new Symbol("trace"), KFunctionWrapper(Builtins::trace));
		context->update(new Symbol("value"), KFunctionWrapper(Builtins::value));

		context->update(new Symbol("wrap"), KFunctionWrapper(Builtins::wrap));
		context->update(new Symbol("unwrap"), KFunctionWrapper(Builtins::unwrap));
		
		context->update(new Symbol("update"), KFunctionWrapper(Builtins::update));
		context->update(new Symbol("head"), KFunctionWrapper(Builtins::head));
		context->update(new Symbol("tail"), KFunctionWrapper(Builtins::tail));

		context->update(new Symbol("caller"), KFunctionWrapper(Builtins::caller));

		context->update(new Symbol("and"), KFunctionWrapper(Builtins::logicalAnd));
		context->update(new Symbol("or"), KFunctionWrapper(Builtins::logicalOr));
		context->update(new Symbol("not"), KFunctionWrapper(Builtins::logicalNot));
		
		context->update(new Symbol("with"), KFunctionWrapper(Builtins::with));
		
		context->update(new Symbol("Table"), Table::metaclass());
		
		return context;
	}
}

int main (int argc, const char * argv[]) {
	using namespace Kai;

	GC_init();
	
	int result = 0;
	Terminal console(STDIN_FILENO);
	Table * context = buildContext();

	if (argc == 2) {
		SourceCode code(argv[1]);
		
		runCode(context, code, result);
	} else if (console.isTTY()) {
		TerminalEditor terminalEditor("kai> ");
		
		StringT input;
		
		while (terminalEditor.readInput(input)) {
			Value * value;
							
			SourceCode currentLine("<stdin>", input);
			
			value = runCode(context, currentLine, result);
			
			if (value)
				std::cout << Value::toString(value) << std::endl;
		}
	} else {
		StringStreamT buffer;
		buffer << std::cin.rdbuf();
		SourceCode code ("<stdin>", buffer.str());
		
		runCode(context, code, result);			
	}
	
	return result;
}
