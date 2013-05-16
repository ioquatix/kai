//
//  Unicode.h
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 7/12/11.
//  Copyright (c) 2011 Orion Transfer Ltd. All rights reserved.
//

#ifndef _KAI_STRING_UNICODE_H
#define _KAI_STRING_UNICODE_H

#include <cstdint>
#include <string>
#include "utf8.h"

namespace Kai {
	namespace Unicode {
		using namespace utf8;
		
		typedef std::uint32_t CodePointT;
		typedef bool (*PredicateFn)(CodePointT);
		
		bool is_letter(CodePointT);
		
		bool is_alpha(CodePointT);
		bool is_numeric(CodePointT);
		bool is_alpha_numeric(CodePointT i);
		bool is_hexadecimal(CodePointT i);
		
		bool is_tab(CodePointT i);
		bool is_space(CodePointT i);
		bool is_whitespace(CodePointT i);
		bool is_whitespace_or_newline(CodePointT i);
		bool is_newline(CodePointT i);
		bool is_not_newline(CodePointT i);

		inline std::size_t length(const std::string & value) {
			return utf8::distance(value.begin(), value.end());
		}

		std::size_t fixed_width(const std::string & value);
		std::size_t fixed_width(CodePointT code_point);

		std::size_t sequence_length(unsigned char first_byte);
	};
}

#endif
