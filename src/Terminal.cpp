/*
 *  Terminal.cpp
 *  This file is part of the "Kai" project, and is licensed under the GNU GPLv3.
 *
 *  Created by Samuel Williams on 1/03/10.
 *  Copyright 2010 Samuel Williams. All rights reserved.
 *
 */

#include "Terminal.h"
#include "Ensure.h"

#include "Object.h"
#include "Frame.h"
#include "Expressions.h"

// class Process { ... }
#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <signal.h>

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
		_in = in_pipe[1];
		_out = out_pipe[0];
		_error = error_pipe[0];
		
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
		close(_in);
		
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
	
	/*
	TermInfo * TermInfo::parse(std::istream & buffer)
	{
		
	}
	
	TermInfo * TermInfo::currentTerminal()
	{
		int pid = fork();
		
		if (pid == 0) {
			// Child
		} else {
			int status = 0;
			waitpid(pid, &status, 0);
		}
	}
	
	TermInfo * TermInfo::forTerminal(StringT name)
	{
		
	}
	 */
	
	Terminal::Terminal(FileDescriptorT in, FileDescriptorT out, FileDescriptorT error) : _in(in), _out(out), _error(error) 
	{
		memset(&_settings, 0, sizeof(struct termios));
	}
	
	Terminal::~Terminal()
	{
		
	}

	bool Terminal::isTTY () const
	{
		return isatty(_in);
	}

	void Terminal::getCurrentSettings ()
	{
		KAI_ENSURE(tcgetattr(_in, &_settings) == 0);
	}

	void Terminal::updateTerminalSettings (int optional_actions) const
	{
		KAI_ENSURE(tcsetattr(_in, optional_actions, &_settings) == 0);
	}

	void Terminal::updateFlags (unsigned flags, bool state)
	{
		if (state)
			_settings.c_lflag |= flags;
		else
			_settings.c_lflag &= ~flags;
	}
	
	std::string Terminal::color(int foreground, int background, int attributes)
	{
		std::stringstream buffer;
		
		buffer << "\e[" << attributes << ";" << (30 + foreground) << ";" << (40 + background) << "m";
		
		return buffer.str();
	}
	
	IEditor::~IEditor()
	{
		
	}
	
	TerminalEditor::TerminalEditor(Terminal * terminal, const StringT & prompt)
		: _terminal(terminal), _prompt(prompt)
	{
	}
	
	TerminalEditor::~TerminalEditor ()
	{
	}
			
	bool TerminalEditor::readInput (StringT & buffer)
	{
		return readInput(buffer, _prompt);
	}
	
	bool TerminalEditor::readInput (StringT & buffer, StringT & prompt)
	{
		std::cout << prompt;
		
		// This function has some bugs in libc++, temporary fix:
		//std::getline(std::cin, buffer);
		
		StringT::value_type c;
		while (1) {
			c = getc(stdin);
			
			if (c == '\b')
				buffer.resize(buffer.size() - 1);
			else if (c == '\n')
				break;
			else
				buffer += c;
		}
		
		return !feof(stdin);
	}
	
	bool TerminalEditor::readInput (StringStreamT & buffer, IEditor & editor)
	{
		StringT prompt = editor.firstPrompt();
		
		do {
			StringT input = "";
			
			if (!readInput(input, prompt))
				return false;
			
			buffer << input << std::endl;
		} while (!editor.is_complete(buffer, prompt));
		
		return true;
	}
	
	void TerminalEditor::writeOutput (StringT buffer)
	{
		std::cout << buffer << std::endl;
	}
	
#pragma mark -
	
	BasicEditor::BasicEditor(Frame * context) : _context(context)
	{
		_expressions = Expressions::fetch(context);
		
		KAI_ENSURE(_expressions);
	}
	
	BasicEditor::~BasicEditor()
	{
	}
	
	StringT BasicEditor::firstPrompt()
	{
		return "kai> ";
	}
	
	bool BasicEditor::is_complete(const StringStreamT & buffer, StringT & prompt)
	{
		ParseResult result;
		
		SourceCode code("<editor>", buffer.str());
		prompt = "";
		
		try {
			result = _expressions->parse(_context, code, true);
		} catch (Parser::FatalParseFailure & ex) {
			return false;
		}
		
		if (result.isIncomplete()) {
			return false;
		} else {
			// FAILED or OKAY
			return true;
		}
	}

}
