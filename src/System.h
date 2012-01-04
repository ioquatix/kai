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
	
	class System : public Object {
	protected:
		Ref<Array> _load_paths;
		
		// Run a script at the given path
		Ref<Object> run(const PathT & path, Frame * frame);
		
		// Load an obeject file at the given path
		Ref<Object> compile(const PathT & path, Frame * frame);
		
	public:
		System(Frame * frame);
		virtual ~System();
		
		virtual Ref<Symbol> identity(Frame * frame) const;
		
		bool find(const StringT & subPath, StringT & fullPath);
		
		virtual void to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const;
		
		static Ref<Object> load(Frame * frame);
		static Ref<Object> require(Frame * frame);
		static Ref<Object> load_paths(Frame * frame);
		static Ref<Object> working_directory(Frame * frame);
		static Ref<Object> environment(Frame * frame);
		
		static void import(Frame * frame);
	};
	
}

#endif
