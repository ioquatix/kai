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
		protected:
			StringT _inputName;
			StringT _buffer;
			
			struct LineIndex {
				unsigned offset;
				unsigned length;
			};
			
			std::vector<LineIndex> _lineOffsets;
			
			void calculateLineOffsets ();
		
		public:
			SourceCode (const PathT & sourceFilePath);
			SourceCode (const StringT & inputName, const StringT & sourceCode);
			virtual ~SourceCode();
			
			StringT inputName () const;
			
			std::size_t size () const { return _buffer.size(); }
			
			unsigned numberOfLines () const;
						
			unsigned lineForOffset (unsigned offset) const; // O(log(N))
			unsigned offsetForLine (unsigned line) const; // O(1)
			StringT stringForLine (unsigned line) const;
			
			std::vector<StringT> stringsForLines (unsigned firstLine, unsigned lastLine) const;
			
			unsigned offsetForIterator (StringIteratorT it) const;
			
			const StringT & buffer ();
			
			StringIteratorT begin () const;
			StringIteratorT end () const;

			// Kai Constructors
			static Ref<Object> from_path(Frame * frame);
			static Ref<Object> from_string(Frame * frame);
		
			// Kai Methods
			static Ref<Object> line_for_offset(Frame * frame);
			static Ref<Object> to_string(Frame * frame);
			static Ref<Object> input_name(Frame * frame);
			static Ref<Object> lines(Frame * frame);
			static Ref<Object> count(Frame * frame);
		
			virtual void to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const;
			
			/// A prototype specifies the behaviour of the current value.
			virtual Ref<Object> prototype(Frame * frame);
			
			/// Evaluate the current value in the given context.
			virtual Ref<Object> evaluate(Frame * frame);
			
			static void import (Frame * frame);
	};
}

#endif
