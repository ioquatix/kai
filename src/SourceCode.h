//
//  SourceCode.h
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 10/07/09.
//  Copyright 2009 Samuel Williams. All rights reserved.
//
//

#ifndef _KAI_PARSER_H
#define _KAI_PARSER_H

#include "Kai.h"
#include "Object.h"
#include <vector>
#include <map>

namespace Kai {
	typedef StringT PathT;
	
	class InvalidLine {};
	
	class InvalidOffset {};
	
	class SourceFileUnreadableError {
		PathT _path;
	public:
		SourceFileUnreadableError(const PathT &);
		
		const PathT & path () const;
	};
	
	class SourceCode : public Object {
	public:
		typedef std::size_t LineT;
		
		struct Marker {
			LineT line;
			std::size_t offset;
			StringIteratorT iterator;
		};
		
		struct Segment {
			Marker begin, end;
			std::vector<StringT> lines;
		};
		
	protected:		
		StringT _input_name;
		StringT _buffer;
		
		struct LineIndex {
			LineT offset;
			LineT length;
		};
		
		std::vector<LineIndex> _line_offsets;
		
		void calculate_line_offsets();
		
	public:
		SourceCode(const PathT & source_file_path);
		SourceCode(const StringT & input_name, const StringT & source_code);
		virtual ~SourceCode();
		
		StringT input_name() const;
		
		std::size_t size() const { return _buffer.size(); }			
		std::size_t number_of_lines() const;
		
		LineT line_for_offset(std::size_t offset) const; // O(log(N))
		std::size_t offset_for_line(LineT line) const; // O(1)
		StringT string_for_line(LineT line) const;
		
		std::vector<StringT> strings_for_lines(unsigned first_line, unsigned last_line) const;
		
		std::size_t offset_for_iterator(StringIteratorT it) const;
		
		// *** Helpers ***
		
		Marker marker_for(StringIteratorT iterator) const {
			std::size_t offset = offset_for_iterator(iterator);
			LineT line = line_for_offset(offset);
			
			Marker marker = {line, offset - _line_offsets[line].offset, iterator}; 
			
			return marker;
		}
		
		Segment segment_for(StringIteratorT begin, StringIteratorT end) const {
			Segment segment = {marker_for(begin), marker_for(end)};
			
			return segment;
		}
		
		// *** Buffer Access ***
		const StringT & buffer();
		
		StringIteratorT begin() const;
		StringIteratorT end() const;
		
		// *** Kai Constructors ***
		static Ref<Object> from_path(Frame * frame);
		static Ref<Object> from_string(Frame * frame);
		
		// *** Kai Methods ***
		static Ref<Object> line_for_offset(Frame * frame);
		static Ref<Object> to_string(Frame * frame);
		static Ref<Object> input_name(Frame * frame);
		static Ref<Object> lines(Frame * frame);
		static Ref<Object> count(Frame * frame);
		
		virtual void to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const;
		
		virtual Ref<Symbol> identity(Frame * frame) const;
		virtual Ref<Object> evaluate(Frame * frame);
		
		static void import (Frame * frame);
	};
	
	class SourceCodeIndex : public Object {
	public:
		struct Association {
			const SourceCode * source_code;
			StringIteratorT begin, end;
			
			SourceCode::Segment segment() const {
				return source_code->segment_for(begin, end);
			}
		};
		
	protected:
		std::map<Object *, Association> _associations;
		
	public:
		static SourceCodeIndex * fetch(Frame * frame);
		
		virtual void mark(Memory::Traversal * traversal) const;
		
		/// Returns an association if one can be found:
		const Association * lookup(Object * object);
		static const Association * lookup(Frame * frame, Object * object);
		
		/// Associates the given object with the given source code buffer:
		void associate(Object * object, const SourceCode * source_code, StringIteratorT begin, StringIteratorT end);
		static void associate(Frame * frame, Object * object, const SourceCode * source_code, StringIteratorT begin, StringIteratorT end);
		
		virtual Ref<Symbol> identity(Frame * frame) const;
		
		static void import (Frame * frame);
	};
}

#endif
