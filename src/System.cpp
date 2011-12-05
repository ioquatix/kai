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

// getcwd, environ
#include <unistd.h>
extern char ** environ;

namespace Kai {
	
	System::System() {
		m_loadPaths = new Array();
		m_loadPaths->value().push_back(new String("./lib/"));
	}
	
	System::~System () {
		
	}
	
	Value * System::run (const PathT & path, Frame * frame) {
		SourceCode code(path);
			
		Expressions * expressions = Expressions::fetch(frame);
		Value * value = expressions->parse(code).value;
		Value * result = value->evaluate(frame);
		
		return result;
	}
	
	Value * System::compile (const PathT & path, Frame * frame) {
		Value * config = run(path, frame);
		
		if (!config) {
			throw Exception("Could not read configuration table", frame);
		}
		
		//config->call(sym('source-files'));
		
		return NULL;
	}
	
	bool pathExists (PathT path) {
		struct stat s;
				
		if (stat(path.c_str(), &s) == 0) {
			return true;
		}
		
		return false;
	}
	
	bool System::find (const PathT & subPath, PathT & path) {
		// Absolute path?
		if (subPath[0] == '/') {
			if (pathExists(subPath)) {
				path = subPath;
				return true;
			}
		}
		
		Array::ConstIteratorT begin = m_loadPaths->value().begin();
		Array::ConstIteratorT end = m_loadPaths->value().end();
		
		while (begin != end) {
			String * base = dynamic_cast<String*>(*begin);
			
			path = base->value() + subPath;
			
			if (pathExists(path)) {
				return true;
			}
			
			begin++;
		}
		
		return false;
	}
	
	void System::toCode(StringStreamT & buffer, MarkedT & marks, std::size_t indentation) {
		buffer << "(System@" << this << ")" << std::endl;
	}
	
	Value * System::require (Frame * frame) {
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
	
	Value * System::load (Frame * frame) {
		System * system = NULL;
		String * name = NULL;
		
		frame->extract()(system)(name);
		
		PathT path;
		if (system->find(name->value(), path)) {
			return system->run(path, frame);
		}
		
		throw Exception("Could not find file", name, frame);
	}
	
	Value * System::loadPaths (Frame * frame) {
		System * system = NULL;
		
		frame->extract()(system);
		
		return system->m_loadPaths;
	}
	
	Value * System::workingDirectory (Frame * frame) {
		char * buf = getcwd(NULL, 0);
		
		ensure(buf != NULL);
		
		String * path = new String(buf);
		
		free(buf);
		
		return path;
	}
	
	Value * System::environment (Frame * frame) {
		Table * env = new Table;
		
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
			
			env->update(sym(key), new String(value));
		}
		
		return env;
	}
	
	Value * System::prototype () {
		return globalPrototype();
	}
	
	Value * System::globalPrototype () {
		static Table * g_prototype = NULL;
		
		if (g_prototype == NULL) {
			g_prototype = new Table();
			
			g_prototype->update(sym("require"), KFunctionWrapper(System::require));
			g_prototype->update(sym("load"), KFunctionWrapper(System::load));
			g_prototype->update(sym("loadPaths"), KFunctionWrapper(System::loadPaths));
			g_prototype->update(sym("workingDirectory"), KFunctionWrapper(System::workingDirectory));
			g_prototype->update(sym("environment"), KFunctionWrapper(System::environment));
		}
		
		return g_prototype;
	}
	
	void System::import (Table * context) {
		System * system = new System();
		
		context->update(sym("system"), system);
		context->update(sym("System"), System::globalPrototype());
	}
	
}
