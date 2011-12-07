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
#include <stdio.h>
#include <signal.h>

#include "Reference.h"

#include "Value.h"
#include "Parser/Parser.h"
#include "Frame.h"
#include "Function.h"
#include "Exception.h"
#include "Terminal.h"
#include "SourceCode.h"
#include "Expressions.h"
#include "BasicEditor.h"

#include "Lambda.h"
#include "Array.h"
#include "System.h"

namespace {

	using namespace Kai;
	
	Ret<Value> runCode (Table * context, SourceCode & code, int & status) {
		Ref<Value> value = NULL, result = NULL;

		// Execution status
		status = 0;
	
		try {
			Ref<Frame> frame = new Frame(context);
			Ref<Expressions> expressions = Expressions::fetch(frame);
			
			value = expressions->parse(code).value;
			
			if (value) {
				result = value->evaluate(frame);
				
				if (result)
					context->update(sym("_"), result);
				
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
		String::import(global);
		Table::import(global);
		Lambda::import(global);
		Logic::import(global);
		Expressions::import(global);
		
		Array::import(global);
		System::import(global);
		
		Table * context = new Table;
		context->setPrototype(global);
		
		return context;
	}
}

void signalHang (int) {
	puts("Segmentation Fault!\n");
	for (;;) sleep(1);
}

int main (int argc, const char * argv[]) {
	Time start;
	
	using namespace Kai;
	
	signal(SIGSEGV, signalHang);
	
	int result = 0;
	Terminal console(STDIN_FILENO);
	Table * context = buildContext();

	BasicEditor editor(context);
	
	if (argc == 3) {
		if (StringT(argv[1]) == "-x") {
			SourceCode code("<cmd>", argv[2]);
			runCode(context, code, result);
		} else if (StringT(argv[1]) == "-f") {
			SourceCode code(argv[2]);
			runCode(context, code, result);
		} else {
			std::cerr << "Unknown option: '" << argv[1] << "'" << std::endl;
			result = 10;
		}
	} else if (console.isTTY()) {
		// Running interactively
		TerminalEditor terminalEditor("kai> ");
		
		StringStreamT buffer;
		
		std::cerr << "Startup time = " << (Time() - start) << std::endl;
		
		while (terminalEditor.readInput(buffer, editor)) {
			Ref<Value> value;
							
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
	
#ifdef KAI_TRACE
	// Dump trace statistics.
	std::cout << std::endl;
	Tracer::globalTracer()->dump(std::cout);
#endif
	
	return result;
}
