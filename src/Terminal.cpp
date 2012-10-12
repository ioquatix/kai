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

// run_code includes garbage collection
#include "Memory/Collector.h"

// class Process { ... }
#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <signal.h>
#include <string.h>

#include <sys/ioctl.h>

// wcswidth - number of columns for a given string:
#include <wchar.h>
#include <xlocale.h>

namespace Kai {

	Ref<Object> run_code (Frame * frame, SourceCode * code, int & status, Terminal * terminal) {
		Ref<Object> value = NULL, result = NULL;

		// Execution status
		status = 0;

		try {
			Ref<Parser::Expressions> expressions = Parser::Expressions::fetch(frame);

			//std::cerr << "Parsing code..." << std::endl;
			value = expressions->parse(frame, code).value;

			//std::cerr << "Evaluating expression..." << std::endl;
			//std::cerr << Object::to_string(frame, value) << std::endl;
			
			if (value) {
				result = value->evaluate(frame);
			}

			// Save the result of the expression into the special variable "_":
			frame->update(frame->sym("_"), result);

			// Run the garbage collector for the memory pool that contains value:
			Memory::Collector collector(frame->allocator());
			collector.collect();

			return result;
		} catch (Exception & ex) {
			// Execution failed
			status = 1;

			if (value) {
				std::cerr << "Executing : " << Object::to_string(frame, value) << std::endl;
			}

			std::cerr << "Exception : " << ex.message() << std::endl;

			ex.top()->debug();
		} catch (Parser::FatalParseFailure & ex) {
			// Print syntax error message

			//std::cerr << "\x1b[31;1m";
			ex.print_error(std::cerr, code);
			//std::cerr << "\x1b[0m";

			status = 2;
		}

		return NULL;
	}
	
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

	Ref<Object> Terminal::run(Frame * frame) {
		Terminal * terminal;
		Frame * context = frame->previous();

		frame->extract()(terminal, "self");

		if (terminal->is_tty()) {
			// Set the terminal editor to raw mode:
			CommandLineEditor editor(context);
			XTerminalSession terminal_editor(terminal);

			StringT buffer;
			while (terminal_editor.read_input(buffer, editor)) {
				int result = 0;

				Ref<SourceCode> input = new(context) SourceCode("<stdin>", buffer);

				Ref<Object> value = run_code(context, input, result, terminal);

				std::cout << Object::to_string(context, value) << std::endl;

				buffer = "";
			}
		} else {
			throw Exception("Terminal is not a TTY!", terminal, frame);
		}

		return nullptr;
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

		prototype->update(frame->sym("run!"), KAI_BUILTIN_FUNCTION(Terminal::run));
		prototype->update(frame->sym("size"), KAI_BUILTIN_FUNCTION(Terminal::size));
		prototype->update(frame->sym("tty?"), KAI_BUILTIN_FUNCTION(Terminal::is_tty));
		
		frame->update(frame->sym("Terminal"), prototype);
	}
	
	ICommandLineEditor::~ICommandLineEditor()
	{
		
	}
	
	XTerminalSession::XTerminalSession(Terminal * terminal) : _terminal(terminal), _history_position(0)
	{
	}
	
	XTerminalSession::~XTerminalSession ()
	{
	}
	
	void XTerminalSession::mark(Memory::Traversal * traversal) const {
		Object::mark(traversal);
		traversal->traverse(_terminal);
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
		output_buffer << "\x1b[0G\x1b[" << (prompt.size() + column_offset()) << "C";
		
		// Write the data to the terminal output:
		FileDescriptorT output_file = _terminal->output_file();
		auto output_string = output_buffer.str();
		write(output_file, output_string.data(), output_string.size());
	}

	void XTerminalSession::clear_buffer() {
		_buffer.str("");
		_current_line.clear();
		_cursor_position = 0;
	}

	void XTerminalSession::set_buffer(const StringT & line) {
		clear_buffer();
		_current_line = line;
	}

	StringT & XTerminalSession::current_line() {
		if (_history_position == 0) {
			return _current_line;
		} else {
			return _history[_history.size() - _history_position];
		}
	}

	std::size_t column_width(const StringT & string) {
		auto current = string.begin(), end = string.end();

		std::size_t width = 0;

		while (current != end) {
			Unicode::CodePointT code_point = Unicode::next(current, end);

			width += Unicode::fixed_width(code_point);
		}

		return width;
	}

	std::size_t XTerminalSession::column_offset() {
		auto line = current_line();
		
		auto current = line.begin(), offset = line.begin() + _cursor_position;

		std::size_t width = 0;

		while (current < offset) {
			Unicode::CodePointT code_point = Unicode::next(current, line.end());

			width += Unicode::fixed_width(code_point);
		}

		return width;
	}

	bool XTerminalSession::read_input (StringT & prompt)
	{
		_cursor_position = 0;
		
		while (1) {
			unsigned char c;
			
			refresh_prompt(prompt, current_line());
			
			// We may read a partial multi-byte input sequence, could this cause problems?
			if (read(_terminal->input_file(), &c, 1) != 1) {
				return false;
			}

			switch (c) {
				case 13:
					// End of line ("\r"):
					write(_terminal->output_file(), "\r\n", 2);

					_buffer << current_line() << std::endl;
					_history.push_back(current_line());

					_current_line = "";

					if (_history_position > 0)
						_history_position -= 1;

					_cursor_position = column_width(current_line());

					return true;
				
				case 3:
					// Interrupt (Ctrl-C):
					return false;
					
				case 127:
				case 8:
					// Backspace of Ctrl-H; remove character to left of cursor:
					if (_cursor_position > 0) {
						auto line = current_line();
						auto current = line.begin() + _cursor_position;

						// Find the start of the previous code-point:
						Unicode::prior(current, line.begin(), line.end());

						std::size_t offset = current - line.begin();
						std::size_t length = Unicode::sequence_length(*current);
						current_line().erase(offset, length);
						_cursor_position = offset;
					}
					break;
				
				case 4:
					// Ctrl-D; remove character to right of cursor:
					if (_cursor_position < current_line().size()) {
						current_line().erase(_cursor_position, 1);
					}
					break;
					
				case 033:
					unsigned char s[2];
					// Escape sequence:
					if (read(_terminal->input_file(), s, 2) != 2) {
						return false;
					}

					if (s[0] == 91) {
						// Arrow keys (left & right):
						if (s[1] == 65) {
							// Up arrow
							_history_position = std::min(_history_position + 1, _history.size());
							_cursor_position = current_line().size();
						}

						if (s[1] == 66) {
							// Down arrow
							if (_history_position > 0)
								_history_position -= 1;
							
							_cursor_position = current_line().size();
						}

						if (s[1] == 67 && _cursor_position < current_line().size()) {
							// Right arrow
							auto offset = current_line().begin() + _cursor_position;
							Unicode::next(offset, current_line().end());

							_cursor_position = offset - current_line().begin();
						}

						if (s[1] == 68 && _cursor_position > 0) {
							// Left arrow
							auto offset = current_line().begin() + _cursor_position;
							Unicode::prior(offset, current_line().begin(), current_line().end());

							_cursor_position = offset - current_line().begin();
						}
					}
					break;

				default:
					unsigned char input[4];
					input[0] = c;

					// The length of the character in bytes:
					std::size_t total = Unicode::sequence_length(c);
					std::size_t required = total - 1;
					
					if (total > 1) {
						// Read the remainder of the character:
						if (read(_terminal->input_file(), input + 1, required) != total - 1) {
							return false;
						}
					}
					
					current_line().insert(_cursor_position, StringT(input, input + total));
					_cursor_position += total;
					break;
			}
		}
	}
	
	bool XTerminalSession::read_input (StringT & buffer, ICommandLineEditor & editor)
	{
		// Lock the terminal into raw mode for the duration of this call:
		Terminal::RawModeLock raw_mode_lock(_terminal);

		// Copy the existing buffer, if any, into the edit buffer:
		set_buffer(buffer);

		StringT prompt = editor.first_prompt();
		
		do {
			if (!read_input(prompt))
				return false;

			buffer = _buffer.str();
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
	
	bool CommandLineEditor::is_complete(const StringT & buffer, StringT & prompt)
	{
		Parser::ParseResult result;
		
		SourceCode * code = new(_expressions) SourceCode("<editor>", buffer);
		prompt = "...> ";
		
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
