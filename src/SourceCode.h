/*
 *  SourceCode.h
 *  This file is part of the "Kai" project, and is licensed under the GNU GPLv3.
 *
 *  Created by Samuel Williams on 10/07/09.
 *  Copyright 2009 Samuel Williams. All rights reserved.
 *
 */

#ifndef _KAI_PARSER_H
#define _KAI_PARSER_H

#include "Kai.h"
#include <vector>

namespace Kai {
	typedef StringT PathT;
	
	class InvalidLine {
	};
	
	class InvalidOffset {
	};
	
	class SourceFileUnreadableError {
		PathT m_path;
	public:
		SourceFileUnreadableError(const PathT &);
		
		const PathT & path () const;
	};
	
	class SourceCode {
		protected:
			StringT m_inputName;
			StringT m_buffer;
			
			struct LineIndex {
				unsigned offset;
				unsigned length;
			};
			
			std::vector<LineIndex> m_lineOffsets;
			
			void calculateLineOffsets ();
		
		public:
			SourceCode (const PathT & sourceFilePath);
			SourceCode (const StringT & inputName, const StringT & sourceCode);
			
			StringT inputName () const;
			
			std::size_t size () const { return m_buffer.size(); }
			
			unsigned numberOfLines () const;
						
			unsigned lineForOffset (unsigned offset) const; // O(log(N))
			unsigned offsetForLine (unsigned line) const; // O(1)
			StringT stringForLine (unsigned line) const;
			
			std::vector<StringT> stringsForLines (unsigned firstLine, unsigned lastLine) const;
			
			unsigned offsetForIterator (StringIteratorT it) const;
			
			const StringT & buffer ();
			
			StringIteratorT begin () const;
			StringIteratorT end () const;
	};
}

#endif
