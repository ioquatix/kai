//
//  Terminal.h
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 1/03/10.
//  Copyright 2010 Samuel Williams. All rights reserved.
//
//

#ifndef _KAI_TERMINAL_H
#define _KAI_TERMINAL_H

#include "Kai.h"

#include <sys/types.h>
#include <unistd.h>
#include <termios.h>
#include <string>
#include <iostream>

#include "Parser/Expressions.h"

namespace Kai {
	
	typedef int FileDescriptorT;
	typedef int ProcessIdentifierT;
	typedef int ProcessStatusT;
	
	class Process {
	protected:
		FileDescriptorT _in, _out, _error;
		ProcessIdentifierT _pid;
		
	public:
		Process(StringT path, std::vector<StringT> & arguments);
		~Process();
		
		FileDescriptorT in() { return _in; }
		FileDescriptorT out() { return _out; }
		FileDescriptorT error() { return _error; }
		
		ProcessStatusT wait(bool dump = true);
		void kill(int signal);
	};
	
	/*
	 To provide advanced editing facilities, the Terminal class encapsulates all the features required for line editing, output and text colouring.
	 */
	class Terminal {
	protected:
		FileDescriptorT _in, _out, _error;
		struct termios _settings;
		
	public:
		Terminal(FileDescriptorT in, FileDescriptorT out, FileDescriptorT error);
		virtual ~Terminal();
		
		bool is_tty () const;
		
		void current_settings ();
		void update_settings (int optional_actions = TCSANOW) const;
		
		void update_flags (unsigned flag, bool state);
	};
	
	class IEditor
	{
	public:
		virtual ~IEditor();
		virtual StringT first_prompt() = 0;
		virtual bool is_complete(const StringStreamT & buffer, StringT & prompt) = 0;
	};
	
	class TerminalEditor {
	protected:
		Terminal * _terminal;
		StringT _prompt;
		
	public:
		TerminalEditor(Terminal * terminal, const StringT & prompt);
		~TerminalEditor();
		
		bool read_input(StringT & buffer);
		bool read_input(StringT & buffer, StringT & prompt);			
		bool read_input (StringStreamT & buffer, IEditor & editor);
	};
	
	class Frame;
	
	class BasicEditor : virtual public IEditor
	{
	protected:
		Parser::Expressions * _expressions;
		Frame * _context;
		
	public:
		BasicEditor(Frame * context);
		virtual ~BasicEditor();
		
		virtual StringT first_prompt();
		virtual bool is_complete(const StringStreamT & buffer, StringT & prompt);
	};
}

#endif
