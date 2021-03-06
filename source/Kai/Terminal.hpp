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

#include "Kai.hpp"

#include <sys/types.h>
#include <unistd.h>
#include <termios.h>
#include <string>
#include <iostream>

#include "Object.hpp"
#include "Parser/Expressions.hpp"

namespace Kai {
	
	typedef int FileDescriptorT;
	typedef int ProcessIdentifierT;
	typedef int ProcessStatusT;

	class Terminal;

	Ref<Object> run_code (Frame * frame, SourceCode * code, int & status, Terminal * terminal);
	
	class Process {
	protected:
		FileDescriptorT _input_file, _output_file, _error_file;
		ProcessIdentifierT _pid;
		
	public:
		Process(StringT path, std::vector<StringT> & arguments);
		~Process();
		
		FileDescriptorT in() { return _input_file; }
		FileDescriptorT out() { return _output_file; }
		FileDescriptorT error() { return _error_file; }
		
		ProcessStatusT wait(bool dump = true);
		void kill(int signal);
	};
	
	/*
	 To provide advanced editing facilities, the Terminal class encapsulates all the features required for line editing, output and text colouring.
	 */
	class Terminal : public Object {
	protected:
		FileDescriptorT _input_file, _output_file, _error_file;
		struct termios _settings, _nominal_settings;
		
		bool _raw_mode;

		void fetch_current_settings();
		void set_raw_mode(bool enabled);

	public:
		static const char * const NAME;
		
		Terminal(FileDescriptorT in, FileDescriptorT out, FileDescriptorT error);
		virtual ~Terminal();

		class RawModeLock {
			Terminal * _terminal;

		public:
			RawModeLock(Terminal * terminal) : _terminal(terminal) {
				_terminal->set_raw_mode(true);
			}

			~RawModeLock() {
				_terminal->set_raw_mode(false);
			}
		};
		
		FileDescriptorT input_file() { return _input_file; }
		FileDescriptorT output_file() { return _output_file; }
		FileDescriptorT error_file() { return _error_file; }
		
		virtual Ref<Symbol> identity(Frame * frame) const;
		
		bool is_tty() const;
		
		std::string terminal_name();
		
		void update_settings(int optional_actions = TCSANOW) const;
		void update_flags(unsigned flag, bool state);
		
		struct Size {
			Size(unsigned short width, unsigned short height) : width(width), height(height) {}
			
			unsigned short width, height;
		};
		
		Size size() const;
		
		static Ref<Object> size(Frame * frame);
		static Ref<Object> is_tty(Frame * frame);

		static Ref<Object> run(Frame * frame);

		static void import(Frame * frame);
	};
	
	class ICommandLineEditor : virtual public Object {
	public:
		virtual ~ICommandLineEditor();
		virtual StringT first_prompt() = 0;
		virtual bool is_complete(const StringT & buffer, StringT & prompt) = 0;
	};
	
	class XTerminalSession : public Object {
	protected:
		Terminal * _terminal;

		// A history of single lines which have been entered by the user:
		std::vector<StringT> _history;

		// The current input buffer which contains all the text entered by the user as individual lines:
		StringStreamT _buffer;
		StringT _current_line;

		std::size_t _history_position;
		std::size_t _cursor_position;
		
		void refresh_prompt (const StringT & prompt, const StringT & buffer);

		void clear_buffer();
		void set_buffer(const StringT & buffer);
		StringT & current_line();
		std::size_t column_offset();

		void begin_edit();

		bool read_input(StringT & prompt);

	public:
		XTerminalSession(Terminal * terminal);
		virtual ~XTerminalSession();
		
		// TODO:
		virtual void mark(Memory::Traversal * traversal) const;

		bool read_input(StringT & buffer, ICommandLineEditor & editor);
		
		struct OutputMode {
			enum Attributes {
				NORMAL = 0,
				BOLD = 1,
				FAINT = 2,
				ITALIC = 3,
				UNDERLINE = 4
			};
			
			enum Colors {
				BLACK = 0,
				RED = 1,
				GREEN = 2,
				YELLOW = 3,
				BLUE = 4,
				MAGENTA = 5,
				CYAN = 6,
				WHITE = 7
			};
			
			static const int UNSPECIFIED = -1;
			
			OutputMode(int attributes = UNSPECIFIED, int foreground_color = UNSPECIFIED, int background_color = UNSPECIFIED) : attributes(attributes), foreground_color(foreground_color), background_color(background_color) {}
			
			int attributes;
			int foreground_color, background_color;
		};
	};
	
	class Frame;
	
	class CommandLineEditor : virtual public ICommandLineEditor
	{
	protected:
		Parser::Expressions * _expressions;
		Frame * _context;
		
	public:
		CommandLineEditor(Frame * context);
		virtual ~CommandLineEditor();
		
		virtual StringT first_prompt();
		virtual bool is_complete(const StringT & buffer, StringT & prompt);
	};
}

#endif
