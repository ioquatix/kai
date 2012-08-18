//
//  Terminal.cpp
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 1/03/10.
//  Copyright 2010 Samuel Williams. All rights reserved.
//
//

#include "Terminal.h"
#include "Ensure.h"

#include "Object.h"
#include "Function.h"
#include "Number.h"
#include "Frame.h"
#include "Symbol.h"
#include "Expressions.h"

// class Process { ... }
#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <signal.h>
#include <string.h>

#include <sys/ioctl.h>

namespace Kai {
	
	// infocmp -L -1
	Process::Process(StringT path, std::vector<StringT> & arguments)
	{
		int in_pipe[2], out_pipe[2], error_pipe[2];
		
		// Need to check errors
		pipe(in_pipe);
		pipe(out_pipe);
		pipe(error_pipe);
		
		// Write to filedes[1], read from filedes[0]
		_input_file = in_pipe[1];
		_output_file = out_pipe[0];
		_error_file = error_pipe[0];
		
		_pid = fork();
		
		if (_pid == 0) {
			close(in_pipe[1]);
			close(out_pipe[0]);
			close(error_pipe[0]);
			
			const char ** argv = (const char **)malloc(arguments.size() * sizeof(const char *));
			
			for (std::size_t i = 0; i < arguments.size(); ++i) {
				argv[i] = (const char *)arguments[i].c_str();
			}
			
			dup2(in_pipe[0], STDIN_FILENO);
			dup2(out_pipe[1], STDOUT_FILENO);
			dup2(error_pipe[1], STDOUT_FILENO);
			
			// Replace current process with child process.
			execv((const char *)path.c_str(), (char * const *)argv);
		} else {
			close(in_pipe[0]);
			close(out_pipe[1]);
			close(error_pipe[1]);
		}
	}
	
	Process::~Process()
	{		
		wait();
	}
	
	ProcessStatusT Process::wait(bool dump)
	{
		close(_input_file);
		
		if (_pid) {
			ProcessStatusT status = 0;
			
			waitpid(_pid, &status, 0);
			
			_pid = 0;
			
			return status;
		}
		
		return 0;
	}
	
	void Process::kill(int signal) {
		::kill(_pid, signal);
	}
	
	const char * const Terminal::NAME = "Terminal";
	
	Terminal::Terminal(FileDescriptorT in, FileDescriptorT out, FileDescriptorT error) : _input_file(in), _output_file(out), _error_file(error) 
	{
		memset(&_settings, 0, sizeof(struct termios));
		
		// system("infocmp -L -1");
	}
	
	Terminal::~Terminal()
	{
		if (_raw_mode) {
			set_raw_mode(false);
		}
	}
	
	Ref<Symbol> Terminal::identity(Frame * frame) const {
		return frame->sym(NAME);
	}
	
	bool Terminal::is_tty () const
	{
		return isatty(_input_file);
	}
	
	void Terminal::fetch_current_settings()
	{
		KAI_ENSURE(tcgetattr(_input_file, &_settings) == 0);
	}
	
	void Terminal::update_settings (int optional_actions) const
	{
		KAI_ENSURE(tcsetattr(_input_file, optional_actions, &_settings) == 0);
	}
	
	void Terminal::update_flags (unsigned flags, bool state)
	{
		if (state)
			_settings.c_lflag |= flags;
		else
			_settings.c_lflag &= ~flags;
	}
	
	void Terminal::set_raw_mode(bool enabled) {
		if (enabled && !_raw_mode) {
			fetch_current_settings();
			
			_nominal_settings = _settings;
			
			// Input modes: no break, no CR to NL, no parity check, no strip char, no start/stop output control.
			_settings.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

			// Output modes: disable post processing
			_settings.c_oflag &= ~(OPOST);
			
			// Control modes: set 8 bit chars
			_settings.c_cflag |= (CS8);
			
			// Local modes: choing off, canonical off, no extended functions, no signal chars (^Z,^C)
			_settings.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
			
			// Control characters: set return condition: min number of bytes and timer.
			// We want read to return every single byte, without timeout.
			_settings.c_cc[VMIN] = 1; // 1 byte
			_settings.c_cc[VTIME] = 0; // no delay
			
			update_settings();
		} else if (!enabled && _raw_mode) {
			_settings = _nominal_settings;
			
			update_settings();
		}
		
		_raw_mode = enabled;
	}
	
	std::string Terminal::terminal_name() {
		return getenv("TERM");
	}
	
	Terminal::Size Terminal::size() const {
		struct winsize window_size;
		
		KAI_ENSURE(ioctl(1, TIOCGWINSZ, &window_size) != -1);
		
		return Size(window_size.ws_col, window_size.ws_row);
	}
	
	Ref<Object> Terminal::size(Frame * frame) {
		Terminal * terminal;
		
		frame->extract()(terminal, "self");
		
		Terminal::Size size = terminal->size();
		
		return Cell::create(frame)(new(frame) Integer(size.width))(new(frame) Integer(size.height));
	}
	
	Ref<Object> Terminal::is_tty(Frame * frame) {
		Terminal * terminal;
		
		frame->extract()(terminal, "self");
		
		if (terminal->is_tty()) {
			return Symbol::true_symbol(frame);
		} else {
			return Symbol::false_symbol(frame);
		}
	}
	
	void Terminal::import (Frame * frame) {
		Table * prototype = new(frame) Table;
		
		prototype->update(frame->sym("size"), KAI_BUILTIN_FUNCTION(Terminal::size));
		prototype->update(frame->sym("tty?"), KAI_BUILTIN_FUNCTION(Terminal::is_tty));
		
		frame->update(frame->sym("Terminal"), prototype);
	}
	
	ICommandLineEditor::~ICommandLineEditor()
	{
		
	}
	
	XTerminalSession::XTerminalSession(Terminal * terminal, const StringT & prompt) : _terminal(terminal), _prompt(prompt)
	{
	}
	
	XTerminalSession::~XTerminalSession ()
	{
	}
	
	void XTerminalSession::mark(Memory::Traversal * traversal) const {
		Object::mark(traversal);
		traversal->traverse(_terminal);
	}
	
	bool XTerminalSession::read_input (StringT & buffer)
	{
		return read_input(buffer, _prompt);
	}
	
	std::ostream & operator<<(std::ostream & output_stream, const XTerminalSession::OutputMode & mode) {
		using OutputMode = XTerminalSession::OutputMode;
		
		output_stream << "\033[";
		bool first = true;
		
		if (mode.attributes != OutputMode::UNSPECIFIED) {
			output_stream << mode.attributes;
			
			first = false;
		}
		
		if (mode.foreground_color != OutputMode::UNSPECIFIED) {
			if (first)
				first = false;
			else
				output_stream << ';';
			
			output_stream << 30 + mode.foreground_color;
		}
		
		if (mode.background_color != OutputMode::UNSPECIFIED) {
			if (first)
				first = false;
			else
				output_stream << ';';
			
			output_stream << 40 + mode.background_color;
		}
		
		output_stream << 'm';
		
		return output_stream;
	}
	
	void XTerminalSession::refresh_prompt (const StringT & prompt, const StringT & buffer) {
		StringStreamT output_buffer;
		OutputMode default_mode(OutputMode::NORMAL), prompt_mode(OutputMode::UNSPECIFIED, OutputMode::GREEN);
		
		// Move cursor to left edge and write prompt:
		output_buffer << "\x1b[0G";

		output_buffer << prompt_mode << prompt << default_mode;
		
		// Erase all characters to the right, and then append the current buffer:
		output_buffer << "\x1b[0K" << buffer;
		
		// Move the cursor to the current editing position:
		output_buffer << "\x1b[0G\x1b[" << (prompt.size() + _cursor_position) << "C";
		
		// Write the data to the terminal output:
		FileDescriptorT output_file = _terminal->output_file();
		auto output_string = output_buffer.str();
		write(output_file, output_string.data(), output_string.size());
	}
	
	bool XTerminalSession::read_input (StringT & buffer, StringT & prompt)
	{
		_cursor_position = 0;
		
		while (1) {
			unsigned char c;
			
			refresh_prompt(prompt, buffer);
			
			// We may read a partial multi-byte input sequence, could this cause problems?
			if (read(_terminal->input_file(), &c, 1) != 1) {
				return false;
			}
			
			switch (c) {
				case 13:
					// End of line ("\r"):
					write(_terminal->output_file(), "\r\n", 2);
					return true;
				
				case 3:
					// Interrupt (Ctrl-C):
					return false;
					
				case 127:
				case 8:
					// Backspace of Ctrl-H; remove character to left of cursor:
					if (_cursor_position > 0) {
						buffer.erase(_cursor_position - 1, 1);
						_cursor_position -= 1;
					}
					break;
				
				case 4:
					// Ctrl-D; remove character to right of cursor:
					if (_cursor_position < buffer.size()) {
						buffer.erase(_cursor_position, 1);
					}
					break;
					
				case 033:
					unsigned char s[2];
					// Escape sequence:
					if (read(_terminal->input_file(), s, 2) != 2) {
						return false;
					}
					
					if (s[0] == 91) {
						// Arrow keys:
						if (s[1] == 68 && _cursor_position > 0) {
							_cursor_position -= 1;
						}
						
						if (s[1] == 67 && _cursor_position < buffer.size()) {
							_cursor_position += 1;
						}
					}					
					break;
					
				default:
					buffer.insert(buffer.begin() + _cursor_position, c);
					_cursor_position += 1;
					break;
			}
		}
	}
	
	bool XTerminalSession::read_input (StringStreamT & buffer, ICommandLineEditor & editor)
	{
		StringT prompt = editor.first_prompt();
		
		do {
			StringT input = "";
			
			if (!read_input(input, prompt))
				return false;
			
			buffer << input << std::endl;
		} while (!editor.is_complete(buffer, prompt));
		
		return true;
	}
	
// MARK: -
	
	CommandLineEditor::CommandLineEditor(Frame * context) : _context(context)
	{
		_expressions = Parser::Expressions::fetch(context);
		
		KAI_ENSURE(_expressions);
	}
	
	CommandLineEditor::~CommandLineEditor()
	{
	}
	
	StringT CommandLineEditor::first_prompt()
	{
		return "kai> ";
	}
	
	bool CommandLineEditor::is_complete(const StringStreamT & buffer, StringT & prompt)
	{
		Parser::ParseResult result;
		
		SourceCode * code = new(_expressions) SourceCode("<editor>", buffer.str());
		prompt = "";
		
		try {
			result = _expressions->parse(_context, code, true);
		} catch (Parser::FatalParseFailure & ex) {
			return false;
		}
		
		if (result.is_incomplete()) {
			return false;
		} else {
			// FAILED or OKAY
			return true;
		}
	}
	
}
