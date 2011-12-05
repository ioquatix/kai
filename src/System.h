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
		Value * run (const PathT & path, Frame * frame);
		
		// Load an obeject file at the given path
		Value * compile (const PathT & path, Frame * frame);
		
	public:
		System();
		virtual ~System();
		
		bool find (const StringT & subPath, StringT & fullPath);
		
		virtual void toCode(StringStreamT & buffer, MarkedT & marks, std::size_t indentation);
		
		static Value * load (Frame * frame);
		static Value * require (Frame * frame);
		static Value * loadPaths (Frame * frame);
		static Value * workingDirectory (Frame * frame);
		static Value * environment (Frame * frame);
		
		virtual Value * prototype ();
		static Value * globalPrototype ();
		static void import (Table * context);
	};
	
}

#endif
