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
#include "Expressions.h"
#include "BasicEditor.h"

namespace {

	using namespace Kai;
	
	Value * runCode (Table * context, SourceCode & code, int & status) {
		Value * value = NULL, * result = NULL;

		// Execution status
		status = 0;
	
		try {
			Frame * frame = new Frame(context);
			Expressions * expressions = frame->lookupAs<Expressions>(new Symbol("expr"));
			
			value = expressions->parse(code).value;
			
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
			// Print syntax error message
			ex.printError(std::cerr, code);
			
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
		Symbol::import(global);
		Cell::import(global);
		Table::import(global);
		Lambda::import(global);
		Logic::import(global);
		Expressions::import(global);
		
		Compiler::import(global);
		CompiledType::import(global);
		
		Table * context = new Table;
		context->setPrototype(global);
		
		return context;
	}
}

int main (int argc, const char * argv[]) {
	Time start;
	
	using namespace Kai;

	GC_init();
	
	int result = 0;
	Terminal console(STDIN_FILENO);
	Table * context = buildContext();

	BasicEditor editor(context);

	if (argc == 2) {
		SourceCode code(argv[1]);
		
		runCode(context, code, result);
	} else if (console.isTTY()) {
		// Running interactively
		TerminalEditor terminalEditor("kai> ");
		
		StringStreamT buffer;
		
		std::cerr << "Startup time = " << (Time() - start) << std::endl;
		
		while (terminalEditor.readInput(buffer, editor)) {
			Value * value;
							
			SourceCode currentLine("<stdin>", buffer.str());
						
			value = runCode(context, currentLine, result);
			
			std::cout << Value::toString(value) << std::endl;
			
			buffer.str("");
		}
	} else {
		StringStreamT buffer;
		buffer << std::cin.rdbuf();
		SourceCode code ("<stdin>", buffer.str());
		
		runCode(context, code, result);			
	}
	
	// Dump trace statistics.
	std::cout << std::endl;
	Tracer::globalTracer()->dump(std::cout);
	
	return result;
}
