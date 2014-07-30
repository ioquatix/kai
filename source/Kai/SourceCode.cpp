//
//  SourceCode.cpp
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 10/07/09.
//  Copyright 2009 Samuel Williams. All rights reserved.
//
//

#include "SourceCode.hpp"
#include "Cell.hpp"
#include "Frame.hpp"
#include "Number.hpp"
#include "Function.hpp"
#include "Table.hpp"
#include "String.hpp"
#include "Symbol.hpp"
#include "Unicode/Unicode.hpp"
#include <fstream>

namespace Kai {
	
	SourceFileUnreadableError::SourceFileUnreadableError (const PathT & path) : _path(path)
	{
		
	}
	
	SourceCode::SourceCode (const PathT & source_file_path)
	{
		std::ifstream ifs(source_file_path.c_str());
		
		if (!ifs)
			throw SourceFileUnreadableError(source_file_path);
		
		_input_name = source_file_path;
		
		_buffer = StringT((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
		
		calculate_line_offsets();
	}
	
	SourceCode::SourceCode (const StringT & input_name, const StringT & source_code)
	: _input_name(input_name), _buffer(source_code)
	{
		calculate_line_offsets();
	}
	
	SourceCode::~SourceCode()
	{
		
	}
	
	StringT SourceCode::input_name () const {
		return _input_name;
	}
	
	void SourceCode::calculate_line_offsets () {
		StringIteratorT current = _buffer.begin();
		StringIteratorT eol = current;
		
		while (current != _buffer.end()) {
			Unicode::CodePointT code_point = Unicode::next(current, _buffer.cend());
			
			if (Unicode::is_newline(code_point)) {
				LineIndex line_index = {LineT(eol - _buffer.begin()), LineT(current - eol)};
				
				// We have processed one whole line:
				_line_offsets.push_back(line_index);
				
				// Technically this points to the beginning of the next line, not the newline itself:
				eol = current;
			}
		}
		
		// The file may not end with a newline, in which case we need to create a line for the trailing characters:
		if (eol != _buffer.end()) {
			LineIndex line_index = {LineT(eol - _buffer.begin()), LineT(_buffer.end() - eol)};
			
			_line_offsets.push_back(line_index);
		}		
	}
	
	std::size_t SourceCode::number_of_lines () const {
		return _line_offsets.size();
	}
	
	SourceCode::LineT SourceCode::line_for_offset (std::size_t offset) const {
		if (offset > _buffer.size()) {
			std::cerr << "Offset: " << offset << " bigger than buffer size: " << _buffer.size() << std::endl;
			throw InvalidOffset();
		}
		
		std::size_t min = 0;
		std::size_t max = _line_offsets.size();
		
		// Is this a hack?
		if (max == 0)
			return 0;
		
		// We should converge on the correct line in log(_line_offsets.size()) iterations.
		while (true) {
			std::size_t length = (max - min);
			std::size_t line = min + (length >> 1);
			
			// Short circuit - we have converged to a single possible result
			//if (length == 1)
			//	return line;
			
			if (offset < _line_offsets[line].offset) {
				max = line;
			} else if (line+1 < max && offset >= _line_offsets[line+1].offset) {
				min = line;
			} else {
				return line;
			}
		}
	}
	
	std::size_t SourceCode::offset_for_line (LineT line) const {
		if (line >= number_of_lines())
			throw InvalidLine();
		
		return _line_offsets[line].offset;
	}
	
	StringT SourceCode::string_for_line (LineT line) const {
		if (line >= number_of_lines())
			throw InvalidLine();
		
		LineIndex li = _line_offsets[line];
		
		return StringT(&_buffer[li.offset], &_buffer[li.offset + li.length]);
	}
	
	std::vector<StringT> SourceCode::strings_for_lines (unsigned first_line, unsigned last_line) const {
		std::vector<StringT> strings;
		
		for (unsigned line = first_line; line <= last_line; line += 1) {
			strings.push_back(string_for_line(line));
		}
		
		return strings;
	}
	
	std::size_t SourceCode::offset_for_iterator (StringIteratorT it) const {
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
		buffer << "(SourceCode@" << this << " input_name=" << input_name() << " number_of_lines=" << number_of_lines() << ">";
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
		
		//source_code->line_for_offset(offset->value().to<unsigned>)
		
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
		
		return new(frame) String(source_code->input_name());
	}
	
	Ref<Object> SourceCode::count(Frame * frame)
	{
		SourceCode * source_code;
		frame->extract()(source_code);
		
		return new(frame) Integer(source_code->number_of_lines());
	}
	
	Ref<Symbol> SourceCode::identity(Frame * frame) const {
		return frame->sym("SourceCode");
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
	
// MARK: -
	
	void SourceCodeIndex::mark(Memory::Traversal * traversal) const {
		for (auto association : _associations) {
			traversal->traverse(association.first);
			traversal->traverse(association.second.source_code);
		}
	}
	
	SourceCodeIndex * SourceCodeIndex::fetch(Frame * frame) {
		return frame->lookup(frame->sym("source-code-index")).as<SourceCodeIndex>();
	}
	
	const SourceCodeIndex::Association * SourceCodeIndex::lookup(Object * object) {
		auto result = _associations.find(object);
		
		if (result != _associations.end()) {
			return &result->second;
		}
		
		return NULL;
	}
	
	const SourceCodeIndex::Association * SourceCodeIndex::lookup(Frame * frame, Object * object) {
		SourceCodeIndex * source_code_index = fetch(frame);
		
		if (source_code_index) {
			return source_code_index->lookup(object);
		}
		
		return NULL;
	}
	
	void SourceCodeIndex::associate(Object * object, const SourceCode * source_code, StringIteratorT begin, StringIteratorT end) {
		Association association = {source_code, begin, end};
		_associations[object] = association;
	}
	
	void SourceCodeIndex::associate(Frame * frame, Object * object, const SourceCode * source_code, StringIteratorT begin, StringIteratorT end) {
		SourceCodeIndex * source_code_index = fetch(frame);
		
		if (source_code_index) {
			source_code_index->associate(object, source_code, begin, end);
		}
	}
	
	Ref<Symbol> SourceCodeIndex::identity(Frame * frame) const {
		return frame->sym("SourceCodeIndex");
	}
	
	void SourceCodeIndex::import (Frame * frame) {
		Table * prototype = new(frame) Table;
		
		SourceCodeIndex * source_code_index = new(frame) SourceCodeIndex;
		frame->update(frame->sym("source-code-index"), source_code_index);
		
		frame->update(frame->sym("SourceCodeIndex"), prototype);
	}
}
