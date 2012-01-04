/*
 *  System.cpp
 *  Kai
 *
 *  Created by Samuel Williams on 26/11/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "System.h"

#include <sys/stat.h>
#include "Frame.h"
#include "Expressions.h"
#include "Function.h"
#include "Ensure.h"
#include "Table.h"
#include "Symbol.h"
#include "String.h"

// getcwd, environ
#include <unistd.h>
extern char ** environ;

namespace Kai {
	
	System::System(Frame * frame) {
		_load_paths = new(frame) Array();
		_load_paths->value().push_back(new(frame) String("./lib/"));
	}
	
	System::~System () {	
	}
	
	Ref<Symbol> System::identity(Frame * frame) const {
		return frame->sym("System");
	}
	
	Ref<Object> System::run(const PathT & path, Frame * frame) {
		std::cerr << "Running code at path: " << path << std::endl;
		
		SourceCode code(path);
		
		Expressions * expressions = Expressions::fetch(frame);
		std::cerr << "Expression : " << Object::to_string(frame, expressions) << std::endl;
		
		Ref<Object> value;
		
		try {
			value = expressions->parse(frame, code).value;
			std::cerr << "Value : " << Object::to_string(frame, value) << std::endl;
		} catch (Parser::FatalParseFailure & fatal_parse_failure) {
			// TODO: Need to implement better handling for this kind of failure - maybe convert to Kai::Exception and rethrow.
			fatal_parse_failure.print_error(std::cerr, code);
			
			return NULL;
		}
		
		Ref<Object> result = value->evaluate(frame);
		std::cerr << "Result : " << Object::to_string(frame, result) << std::endl;
		
		return result;
	}
	
	Ref<Object> System::compile(const PathT & path, Frame * frame) {
		Ref<Object> config = run(path, frame);
		
		if (!config) {
			throw Exception("Could not read configuration table", frame);
		}
		
		//config->call(frame->sym('source-files'));
		
		return NULL;
	}
	
	bool path_exists(PathT path) {
		struct stat s;
				
		if (stat(path.c_str(), &s) == 0) {
			return true;
		}
		
		return false;
	}
	
	bool System::find(const PathT & file_path, PathT & result) {
		// Absolute path?
		if (file_path[0] == '/') {
			if (path_exists(file_path)) {
				result = file_path;
				return true;
			}
		}
		
		Array::ConstIteratorT begin = _load_paths->value().begin();
		Array::ConstIteratorT end = _load_paths->value().end();
		
		while (begin != end) {
			const String * base = ptr(*begin).as<String>();
			
			result = base->value() + file_path;
			
			if (path_exists(result)) {
				return true;
			}
			
			begin++;
		}
		
		return false;
	}
	
	void System::to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const {
		buffer << "(System@" << this << ")" << std::endl;
	}
	
	Ref<Object> System::require(Frame * frame) {
		System * system = NULL;
		String * name = NULL;
		
		frame->extract()(system)(name);
		
		PathT path;
		if (system->find(name->value() + ".kai", path)) {
			return system->run(path, frame);
		}
		
		if (system->find(name->value() + ".kdep", path)) {
			return system->compile(path, frame);
		}
		
		throw Exception("Could not find dependency", name, frame);
	}
	
	Ref<Object> System::load(Frame * frame) {
		System * system = NULL;
		String * name = NULL;
		
		frame->extract()(system)(name);
		
		PathT path;
		if (system->find(name->value(), path)) {
			return system->run(path, frame);
		}
		
		throw Exception("Could not find file", name, frame);
	}
	
	Ref<Object> System::load_paths(Frame * frame) {
		System * system = NULL;
		
		frame->extract()(system);
		
		return system->_load_paths;
	}
	
	Ref<Object> System::working_directory(Frame * frame) {
		char * buf = getcwd(NULL, 0);
		
		KAI_ENSURE(buf != NULL);
		
		String * path = new(frame) String(buf);
		
		free(buf);
		
		return path;
	}
	
	Ref<Object> System::environment(Frame * frame) {
		Table * env = new(frame) Table;
		
		for (char **e = environ; *e; ++e) {
			// Pointer to start of key
			char * start = *e;
			// Pointer to equals sign
			char * end = start;
			
			// Find position of '='
			while (*end && *end != '=') {
				end++;
			}
			
			StringT key(start, end);
			
			end++; // Skip over '='
			start = end;
			
			// Find end of string
			while (*end) {
				end++;
			}
			
			StringT value(start, end);
			
			env->update(frame->sym(key.c_str()), new(frame) String(value));
		}
		
		return env;
	}
	
	void System::import(Frame * frame) {
		Table * prototype = new(frame) Table;
		
		prototype->update(frame->sym("require"), KAI_BUILTIN_FUNCTION(System::require));
		prototype->update(frame->sym("load"), KAI_BUILTIN_FUNCTION(System::load));
		prototype->update(frame->sym("load-paths"), KAI_BUILTIN_FUNCTION(System::load_paths));
		prototype->update(frame->sym("working-directory"), KAI_BUILTIN_FUNCTION(System::working_directory));
		prototype->update(frame->sym("environment"), KAI_BUILTIN_FUNCTION(System::environment));
		
		frame->update(frame->sym("System"), prototype);
		
		System * system = new(frame) System(frame);
		frame->update(frame->sym("system"), system);
	}
	
}
