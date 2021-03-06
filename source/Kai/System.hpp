//
//  System.h
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 26/11/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//
//

#ifndef _KAI_SYSTEM_H
#define _KAI_SYSTEM_H

#include "Array.hpp"
#include "SourceCode.hpp"

namespace Kai {

	typedef Ref<Object> (KaiLoadFn)(Frame * context);
	
	class System : public Object {
	protected:
		Ref<Array> _load_paths;
		Ref<Array> _arguments;
		
		// Run a script at the given path
		Ref<Object> run(const PathT & path, Frame * frame);
		
		// Load an obeject file at the given path
		Ref<Object> compile(const PathT & path, Frame * frame);
		
	public:
		static const char * const NAME;
		
		System(Frame * frame);
		virtual ~System();
		
		void set_arguments(std::size_t argc, const char * argv[]);
		
		virtual Ref<Symbol> identity(Frame * frame) const;
		
		// Returns true if the file_path can be found (either an absolute path or as a sub-path of one of _load_paths), and puts the full path in result.
		bool find(const PathT & file_path, PathT & result);
		
		virtual void to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const;
		
		static Ref<Object> load(Frame * frame);
		static Ref<Object> require(Frame * frame);
		static Ref<Object> load_paths(Frame * frame);
		static Ref<Object> working_directory(Frame * frame);
		static Ref<Object> environment(Frame * frame);
		static Ref<Object> arguments(Frame * frame);
		
		static void import(Frame * frame);
	};
	
}

#endif
