/*
 *  SourceCode.cpp
 *  This file is part of the "Kai" project, and is licensed under the GNU GPLv3.
 *
 *  Created by Samuel Williams on 10/07/09.
 *  Copyright 2009 Samuel Williams. All rights reserved.
 *
 */

#include "SourceCode.h"
#include <fstream>

namespace Kai {
	
	SourceFileUnreadableError::SourceFileUnreadableError (const PathT & path) : m_path(path)
	{
		
	}

	SourceCode::SourceCode (const PathT & sourceFilePath)
	{
		std::ifstream ifs(sourceFilePath.c_str());
		
		if (!ifs)
			throw SourceFileUnreadableError(sourceFilePath);
		
		m_inputName = sourceFilePath;
		
		m_buffer = StringT((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
		
		calculateLineOffsets();
	}
	
	SourceCode::SourceCode (const StringT & inputName, const StringT & sourceCode)
		: m_inputName(inputName), m_buffer(sourceCode)
	{
		calculateLineOffsets();
	}
	
	StringT SourceCode::inputName () const {
		return m_inputName;
	}
	
	bool isLineBreak (const StringT & buffer, unsigned offset) {
		if (buffer[offset] == '\r') {
			if (buffer[offset] == '\n') {
				return 2;
			}
			
			return 1;
		}
		
		 if (buffer[offset] == '\n') {
			if (buffer[offset] == '\r') {
				return 2;
			}
			
			return 1;
		}
		
		return 0;
	}
	
	void SourceCode::calculateLineOffsets () {
		unsigned offset = 0, lineEnding;
		LineIndex li;
		li.offset = 0;
		li.length = 0;
	
		while (offset < m_buffer.size()) {
			lineEnding = isLineBreak(m_buffer, offset);
			
			if (lineEnding) {
				li.length = offset - li.offset;
				m_lineOffsets.push_back(li);
				
				offset += lineEnding;
				li.offset = offset;
			} else {
				offset += 1;
			}
		}
		
		if (offset > li.offset) {
			// File does not finish with newline
			li.length = offset - li.offset;
			m_lineOffsets.push_back(li);
		}
	}
	
	unsigned SourceCode::numberOfLines () const {
		return m_lineOffsets.size();
	}
	
	unsigned SourceCode::lineForOffset (unsigned offset) const {
		if (offset > m_buffer.size()) {
			std::cerr << "Offset: " << offset << " bigger than buffer size: " << m_buffer.size() << std::endl;
			throw InvalidOffset();
		}
		
		unsigned min = 0;
		unsigned max = m_lineOffsets.size();
		
		// Is this a hack?
		if (max == 0)
			return 0;
		
		// We should converge on the correct line in log(m_lineOffsets.size()) iterations.
		while (true) {
			unsigned length = (max - min);
			unsigned line = min + (length >> 1);

			// Short circuit - we have converged to a single possible result
			//if (length == 1)
			//	return line;
			
			if (offset < m_lineOffsets[line].offset) {
				max = line;
			} else if (line+1 < max && offset >= m_lineOffsets[line+1].offset) {
				min = line;
			} else {
				return line;
			}
		}
	}
	
	unsigned SourceCode::offsetForLine (unsigned line) const {
		if (line >= numberOfLines())
			throw InvalidLine();
	
		return m_lineOffsets[line].offset;
	}
	
	StringT SourceCode::stringForLine (unsigned line) const {
		if (line >= numberOfLines())
			throw InvalidLine();
		
		LineIndex li = m_lineOffsets[line];
		
		return StringT(&m_buffer[li.offset], &m_buffer[li.offset + li.length]);
	}
	
	std::vector<StringT> SourceCode::stringsForLines (unsigned firstLine, unsigned lastLine) const {
		std::vector<StringT> strings;
		
		for (unsigned line = firstLine; line <= lastLine; line += 1) {
			strings.push_back(stringForLine(line));
		}
		
		return strings;
	}
	
	unsigned SourceCode::offsetForIterator (StringIteratorT it) const {
		return it - m_buffer.begin();
	}
	
	const StringT & SourceCode::buffer () {
		return m_buffer;
	}
		
	StringIteratorT SourceCode::begin () const {
		return m_buffer.begin();
	}
	
	StringIteratorT SourceCode::end () const {
		return m_buffer.end();
	}

}
