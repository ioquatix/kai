/*
 *  System.h
 *  Kai
 *
 *  Created by Samuel Williams on 26/11/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _KAI_SYSTEM_H
#define _KAI_SYSTEM_H

#include "Array.h"
#include "SourceCode.h"

namespace Kai {
	
	class System : public Value {
	protected:
		Array * m_loadPaths;
		
		// Run a script at the given path
		Ref<Value> run (const PathT & path, Frame * frame);
		
		// Load an obeject file at the given path
		Ref<Value> compile (const PathT & path, Frame * frame);
		
	public:
		System();
		virtual ~System();
		
		bool find (const StringT & subPath, StringT & fullPath);
		
		virtual void toCode(StringStreamT & buffer, MarkedT & marks, std::size_t indentation);
		
		static Ref<Value> load (Frame * frame);
		static Ref<Value> require (Frame * frame);
		static Ref<Value> loadPaths (Frame * frame);
		static Ref<Value> workingDirectory (Frame * frame);
		static Ref<Value> environment (Frame * frame);
		
		virtual Ref<Value> prototype ();
		static Ref<Value> globalPrototype ();
		static void import (Table * context);
	};
	
}

#endif
