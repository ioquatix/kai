//
//  SourceCode.cpp
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 10/07/09.
//  Copyright 2009 Samuel Williams. All rights reserved.
//
//

#include "SourceCode.h"
#include "Cell.h"
#include "Frame.h"
#include "Number.h"
#include "Function.h"
#include "Table.h"
#include "String.h"
#include <fstream>

namespace Kai {
	
	SourceFileUnreadableError::SourceFileUnreadableError (const PathT & path) : _path(path)
	{
		
	}

	SourceCode::SourceCode (const PathT & sourceFilePath)
	{
		std::ifstream ifs(sourceFilePath.c_str());
		
		if (!ifs)
			throw SourceFileUnreadableError(sourceFilePath);
		
		_inputName = sourceFilePath;
		
		_buffer = StringT((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
		
		calculateLineOffsets();
	}
	
	SourceCode::SourceCode (const StringT & inputName, const StringT & sourceCode)
		: _inputName(inputName), _buffer(sourceCode)
	{
		calculateLineOffsets();
	}
	
	SourceCode::~SourceCode()
	{
		
	}
	
	StringT SourceCode::inputName () const {
		return _inputName;
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
	
		while (offset < _buffer.size()) {
			lineEnding = isLineBreak(_buffer, offset);
			
			if (lineEnding) {
				li.length = offset - li.offset;
				_lineOffsets.push_back(li);
				
				offset += lineEnding;
				li.offset = offset;
			} else {
				offset += 1;
			}
		}
		
		if (offset > li.offset) {
			// File does not finish with newline
			li.length = offset - li.offset;
			_lineOffsets.push_back(li);
		}
	}
	
	unsigned SourceCode::numberOfLines () const {
		return _lineOffsets.size();
	}
	
	unsigned SourceCode::lineForOffset (unsigned offset) const {
		if (offset > _buffer.size()) {
			std::cerr << "Offset: " << offset << " bigger than buffer size: " << _buffer.size() << std::endl;
			throw InvalidOffset();
		}
		
		unsigned min = 0;
		unsigned max = _lineOffsets.size();
		
		// Is this a hack?
		if (max == 0)
			return 0;
		
		// We should converge on the correct line in log(_lineOffsets.size()) iterations.
		while (true) {
			unsigned length = (max - min);
			unsigned line = min + (length >> 1);

			// Short circuit - we have converged to a single possible result
			//if (length == 1)
			//	return line;
			
			if (offset < _lineOffsets[line].offset) {
				max = line;
			} else if (line+1 < max && offset >= _lineOffsets[line+1].offset) {
				min = line;
			} else {
				return line;
			}
		}
	}
	
	unsigned SourceCode::offsetForLine (unsigned line) const {
		if (line >= numberOfLines())
			throw InvalidLine();
	
		return _lineOffsets[line].offset;
	}
	
	StringT SourceCode::stringForLine (unsigned line) const {
		if (line >= numberOfLines())
			throw InvalidLine();
		
		LineIndex li = _lineOffsets[line];
		
		return StringT(&_buffer[li.offset], &_buffer[li.offset + li.length]);
	}
	
	std::vector<StringT> SourceCode::stringsForLines (unsigned firstLine, unsigned lastLine) const {
		std::vector<StringT> strings;
		
		for (unsigned line = firstLine; line <= lastLine; line += 1) {
			strings.push_back(stringForLine(line));
		}
		
		return strings;
	}
	
	unsigned SourceCode::offsetForIterator (StringIteratorT it) const {
		return it - _buffer.begin();
	}
	
	const StringT & SourceCode::buffer () {
		return _buffer;
	}
		
	StringIteratorT SourceCode::begin () const {
		return _buffer.begin();
	}
	
	StringIteratorT SourceCode::end () const {
		return _buffer.end();
	}

	void SourceCode::to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const
	{
		buffer << "<SourceCode@" << this << "input_name=" << inputName() << " number_of_lines=" << numberOfLines() << ">";
	}
	
	Ref<Object> SourceCode::evaluate (Frame * frame)
	{
		return NULL;
	}
	
	Ref<Object> SourceCode::line_for_offset(Frame * frame)
	{
		SourceCode * source_code;
		Integer * offset;
		frame->extract()(source_code)(offset);
		
		//source_code->lineForOffset(offset->value().to<unsigned>)
		
		return NULL;
	}
	
	Ref<Object> SourceCode::from_path(Frame * frame)
	{
		Table * prototype;
		String * path;
		
		frame->extract()(prototype)(path);
		
		return new(frame) SourceCode(path->value());
	}
	
	Ref<Object> SourceCode::from_string(Frame * frame)
	{
		Table * prototype;
		String * input_name, * buffer;
		
		frame->extract()(prototype)(input_name)(buffer);
		
		return new(frame) SourceCode(input_name->value(), buffer->value());
	}
	
	Ref<Object> SourceCode::to_string(Frame * frame)
	{
		SourceCode * source_code;
		frame->extract()(source_code);
		
		return new(frame) String(source_code->buffer());
	}
	
	Ref<Object> SourceCode::input_name(Frame * frame)
	{
		SourceCode * source_code;
		frame->extract()(source_code);
		
		return new(frame) String(source_code->inputName());
	}
	
	Ref<Object> SourceCode::count(Frame * frame)
	{
		SourceCode * source_code;
		frame->extract()(source_code);
		
		return new(frame) Integer(source_code->numberOfLines());
	}
	
	Ref<Object> SourceCode::prototype(Frame * frame) {
		return frame->lookup(frame->sym("SourceCode"));
	}
		
	/// Import the global prototype and associated functions into an execution context.
	void SourceCode::import(Frame * frame)
	{
		Table * prototype = new(frame) Table;
				
		prototype->update(frame->sym("from-path"), KAI_BUILTIN_FUNCTION(SourceCode::from_path));
		prototype->update(frame->sym("from-string"), KAI_BUILTIN_FUNCTION(SourceCode::from_string));
		
		prototype->update(frame->sym("to-string"), KAI_BUILTIN_FUNCTION(SourceCode::to_string));
		prototype->update(frame->sym("input-name"), KAI_BUILTIN_FUNCTION(SourceCode::input_name));
		
		prototype->update(frame->sym("count"), KAI_BUILTIN_FUNCTION(SourceCode::count));

		frame->update(frame->sym("SourceCode"), prototype);
	}

}
