/*
 *  Terminal.h
 *  This file is part of the "Kai" project, and is licensed under the GNU GPLv3.
 *
 *  Created by Samuel Williams on 1/03/10.
 *  Copyright 2010 Samuel Williams. All rights reserved.
 *
 */

#ifndef _KAI_TERMINAL_H
#define _KAI_TERMINAL_H

#include "Kai.h"

#include <sys/types.h>
#include <unistd.h>
#include <termios.h>
#include <string>
#include <iostream>

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
	struct TermInfo {
		std::set<StringT> capabilities;
		std::map<StringT, int> limits;
		std::map<StringT, StringT> functions;

		static TermInfo * parse(std::istream & buffer);
		static TermInfo * currentTerminal();
		static TermInfo * forTerminal(StringT name);
	};
	 */
	
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
			
			bool isTTY () const;
			
			void getCurrentSettings ();
			void updateTerminalSettings (int optional_actions = TCSANOW) const;
			
			std::string color(int foreground, int background, int attributes);
		
			void updateFlags (unsigned flag, bool state);
	};
	
	class IEditor
	{
		public:
			virtual ~IEditor();
			virtual StringT firstPrompt() = 0;
			virtual bool is_complete(const StringStreamT & buffer, StringT & prompt) = 0;
	};
	
	class TerminalEditor {
		protected:
			Terminal * _terminal;
			StringT _prompt;
			
		public:
			TerminalEditor(Terminal * terminal, const StringT & prompt);
			~TerminalEditor();
			
			bool readInput(StringT & buffer);
			bool readInput(StringT & buffer, StringT & prompt);
			void writeOutput(StringT);
			
			bool readInput (StringStreamT & buffer, IEditor & editor);
	};
	
	class Frame;
	class Expressions;
	
	class BasicEditor : virtual public IEditor
	{
	protected:
		Expressions * _expressions;
		Frame * _context;
		
	public:
		BasicEditor(Frame * context);
		virtual ~BasicEditor();
		
		virtual StringT firstPrompt();
		virtual bool is_complete(const StringStreamT & buffer, StringT & prompt);
	};
}

#endif
