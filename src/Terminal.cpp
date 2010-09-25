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

#include <string.h>

namespace Kai {

	IEditor::~IEditor()
	{
	
	}

	Terminal::Terminal (unsigned fileno) : m_fileno(fileno) {
		memset(&m_settings, 0, sizeof(struct termios));
	}

	Terminal::Terminal (const Terminal & other) {
		m_fileno = other.m_fileno;
		memcpy(&m_settings, &other.m_settings, sizeof(struct termios));
	}

	bool Terminal::isTTY () const {
		return isatty(m_fileno);
	}

	void Terminal::getCurrentSettings () {
		ensure(tcgetattr(m_fileno, &m_settings) == 0);
	}

	void Terminal::updateTerminalSettings (int optional_actions) const {
		ensure(tcsetattr(m_fileno, optional_actions, &m_settings) == 0);
	}

	void Terminal::updateFlags (unsigned flags, bool state) {
		if (state)
			m_settings.c_lflag |= flags;
		else
			m_settings.c_lflag &= ~flags;
	}
	
	TerminalEditor::TerminalEditor (const StringT & prompt)
		: m_prompt(prompt)
	{
	}
	
	TerminalEditor::~TerminalEditor ()
	{
	}
			
	bool TerminalEditor::readInput (StringT & buffer)
	{
		return readInput(buffer, m_prompt);
	}
	
	bool TerminalEditor::readInput (StringT & buffer, StringT & prompt)
	{
		std::cout << prompt;
		std::getline(std::cin, buffer);
		return std::cin.good();
	}
	
	bool TerminalEditor::readInput (StringStreamT & buffer, IEditor & editor)
	{
		StringT input;
		StringT prompt = editor.firstPrompt();
		
		do {
			if (!readInput(input, prompt))
				return false;
			
			buffer << input << std::endl;
		} while (!editor.isComplete(buffer, prompt));
		
		return true;
	}
	
	void TerminalEditor::writeOutput (StringT buffer)
	{
		std::cout << buffer << std::endl;
	}

}
