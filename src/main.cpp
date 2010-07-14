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

#include "Value.h"
#include "Parser/Parser.h"
#include "Frame.h"
#include "Function.h"
#include "Exception.h"
#include "Terminal.h"
#include "SourceCode.h"
#include "Compiler.h"

namespace {

	using namespace Kai;

	Value * runCode (Table * context, SourceCode & code, int & status) {
		Value * value = NULL, * result = NULL;

		// Execution status
		status = 0;
	
		try {
			Frame * frame = new Frame(context);
			
			value = Parser::parse(code);
			
			if (value) {
				result = value->evaluate(frame);
							
				return result;
			} else {
				return NULL;
			}
		} catch (Exception & ex) {
			// Execution failed
			status = 1;
			
			if (value) {
				std::cerr << "Executing : " << Value::toString(value) << std::endl;
			}
			
			std::cerr << "Exception : " << ex.what() << std::endl;
			
			ex.top()->debug();
		} catch (Parser::FatalParseFailure & ex) {
			status = 2;
		}
	
		return NULL;
	}
	
	Table * buildContext () {
		Table * global = new Table;
		global->setPrototype(Table::globalPrototype());
				
		Integer::import(global);
		Frame::import(global);
		Value::import(global);
		Cell::import(global);
		Table::import(global);
		Lambda::import(global);
		Logic::import(global);
		
		Table * context = new Table;
		context->setPrototype(global);
		
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
