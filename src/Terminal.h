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

	class Terminal {
		protected:
			unsigned m_fileno;
			struct termios m_settings;

		public:
			Terminal (unsigned fileno);
			Terminal (const Terminal & other);
			
			bool isTTY () const;
			
			void getCurrentSettings ();
			void updateTerminalSettings (int optional_actions = TCSANOW) const;
			
			void updateFlags (unsigned flag, bool state);
	};
	
	class TerminalEditor {
		protected:
			StringT m_prompt;
			
		public:
			TerminalEditor (const StringT & prompt);
			~TerminalEditor ();
			
			bool readInput (StringT & buffer);
			void writeOutput (StringT);
	};

}

#endif