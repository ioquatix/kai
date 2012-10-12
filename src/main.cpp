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
#include "Parser/Expressions.h"
#include "Parser/InterpolationExpression.h"

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
	// (trace ["Möbius Frequency" each (lambda `(chr) `[chr size])])
	
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
		
		// Hook up the global prototype:
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
		
		Parser::Expressions::import(frame);
		Parser::InterpolationExpression::import(frame);
		
		SourceCode::import(frame);
		SourceCodeIndex::import(frame);
		
		Array::import(frame);
		System::import(frame);
		
		// Console manipulation:
		Terminal::import(frame);
		
		Ref<Terminal> terminal = new(frame) Terminal(STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO);
		frame->update(frame->sym("terminal"), terminal);
		
		// Garbage collection debugging:
		global->update(frame->sym("gc-debug"), KAI_BUILTIN_FUNCTION(managed_memory_debug));
		
		Table * context = new(frame) Table;
		context->set_prototype(global);
		
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
	Ref<Frame> context = build_context();

	// Used later for setting up system arguments:
	Ref<System> system = context->lookup(context->sym("system"));
	Ref<Terminal> terminal = context->lookup(context->sym("terminal"));

	Ref<SourceCode> code;

	if (argc >= 2) {
		// First argument is a file path
		system->set_arguments(argc - 1, argv + 1);

		code = new(context) SourceCode(argv[1]);
	} else {
		// Source code expected from stdin
		system->set_arguments(argc, argv);

		StringStreamT buffer;
		buffer << std::cin.rdbuf();

		code = new(context) SourceCode("<stdin>", buffer.str());
	}

	try {
		run_code(context, code, result, terminal);
	} catch (std::exception & error) {
		std::cerr << "Fatal Error: " << error.what() << std::endl;
	}
	
#ifdef KAI_TRACE
	// Dump trace statistics.
	std::cout << std::endl;
	Tracer::global_tracer()->dump(std::cout);
#endif
	
	return result;
}
