//
//  main.cpp
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 24/06/09.
//  Copyright 2009 Samuel Williams. All rights reserved.
//
//

#include <iostream>
#include <stdio.h>
#include <signal.h>

#include "Reference.h"
#include "Memory/Collector.h"

#include "Object.h"
#include "Parser/Parser.h"
#include "Frame.h"
#include "Function.h"
#include "Exception.h"
#include "Terminal.h"
#include "SourceCode.h"
#include "Expressions.h"

#include "Lambda.h"
#include "Logic.h"
#include "Table.h"
#include "Number.h"
#include "Array.h"
#include "Symbol.h"
#include "String.h"
#include "System.h"

namespace {
	
	using namespace Kai;
	
	// Example:
	// ["Möbius Frequency" each (lambda `(chr) `[chr size])]
	
	Ref<Object> run_code (Frame * frame, SourceCode * code, int & status, Terminal * terminal) {
		Ref<Object> value = NULL, result = NULL;
		
		// Execution status
		status = 0;
		
		try {
			Ref<Expressions> expressions = Expressions::fetch(frame);
			
			value = expressions->parse(frame, code).value;
			
			if (value) {
				result = value->evaluate(frame);				
			}
			
			// Save the result of the expression into the special variable "_":
			frame->update(frame->sym("_"), result);
			
			// Run the garbage collector for the memory pool that contains value:
			Memory::Collector collector(frame->allocator());
			collector.collect();
			
			return result;
		} catch (Exception & ex) {
			// Execution failed
			status = 1;
			
			if (value) {
				std::cerr << "Executing : " << Object::to_string(frame, value) << std::endl;
			}
			
			std::cerr << "Exception : " << ex.what() << std::endl;
			
			ex.top()->debug();
		} catch (Parser::FatalParseFailure & ex) {
			// Print syntax error message
			
			//std::cerr << "\x1b[31;1m";
			ex.print_error(std::cerr, code);
			//std::cerr << "\x1b[0m";
			
			status = 2;
		}
		
		return NULL;
	}
	
	// ["Möbius Frequency" each (lambda `(chr) `[chr size])]
	
	Ref<Object> managed_memory_debug(Frame * frame) {
		frame->allocator()->debug();
		return NULL;
	}
	
	Ref<Frame> build_context () {
		// Create a garbage collected memory segment:
		Memory::PageAllocation * allocator = Memory::PageAllocation::create(128 * Memory::page_size());
		
		// Create a table for the global definitions and an initial stack frame:
		Table * global = new(allocator) Table();
		Frame * frame = new(allocator) Frame(global);
		
		// Bootstrap the table prototype:
		Object::import(frame);
		Table::import(frame);
		
		global->set_prototype(frame->lookup(frame->sym("Table")));
		
		Integer::import(frame);
		Number::import(frame);
		Frame::import(frame);
		Object::import(frame);
		Symbol::import(frame);
		Cell::import(frame);
		String::import(frame);
		StringBuffer::import(frame);
		Table::import(frame);
		Lambda::import(frame);
		Logic::import(frame);
		Expressions::import(frame);
		
		SourceCode::import(frame);
		SourceCodeIndex::import(frame);
		
		Array::import(frame);
		System::import(frame);
		
		global->update(frame->sym("gc-debug"), KAI_BUILTIN_FUNCTION(managed_memory_debug));
		
		Table * context = new(frame) Table;
		context->set_prototype(global);
		
		std::cerr << "Global: " << global << " Context: " << context << std::endl << std::flush;
		
		// Return a new stack frame one level down.
		return new(frame) Frame(context);
	}
}

void signal_hang (int) {
	puts("Segmentation Fault!\n");
	for (;;) sleep(1);
}

int main (int argc, const char * argv[]) {
	Time start;
	
	using namespace Kai;
	
	signal(SIGSEGV, signal_hang);
	
	int result = 0;
	Terminal console(STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO);
	Ref<Frame> context = build_context();
	
	BasicEditor editor(context);
	
	if (argc > 1) {
		if (StringT(argv[1]) == "-x") {
			Ref<SourceCode> code = new(context) SourceCode("<x>", argv[2]);
			run_code(context, code, result, &console);
		} else if (argc == 2) {
			Ref<SourceCode> code = new(context) SourceCode(argv[1]);
			run_code(context, code, result, &console);
		} else {
			std::cerr << "Unknown option: '" << argv[1] << "'" << std::endl;
			result = 10;
		}
	} else if (console.is_tty()) {
		// Running interactively
		TerminalEditor terminal_editor(&console, "kai> ");
		StringStreamT buffer;
		
		std::cerr << "Startup time = " << (Time() - start) << std::endl;
		
		while (terminal_editor.read_input(buffer, editor)) {	
			Ref<SourceCode> input = new(context) SourceCode("<stdin>", buffer.str());
			Ref<Object> value = run_code(context, input, result, &console);
			
			std::cout << Object::to_string(context, value) << std::endl;
			
			buffer.str("");
		}
	} else {
		StringStreamT buffer;
		buffer << std::cin.rdbuf();
		Ref<SourceCode> code = new(context) SourceCode("<stdin>", buffer.str());
		
		run_code(context, code, result, &console);			
	}
	
#ifdef KAI_TRACE
	// Dump trace statistics.
	std::cout << std::endl;
	Tracer::global_tracer()->dump(std::cout);
#endif
	
	return result;
}
