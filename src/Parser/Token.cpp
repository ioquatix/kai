//
//  Token.cpp
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 7/09/09.
//  Copyright 2009 Orion Transfer Ltd. All rights reserved.
//
//

#include "Token.h"
#include "../SourceCode.h"

namespace Kai {
	namespace Parser {
		const char* name_for_identity(Identity identity) {
			switch(identity) {
			case UNIMPORTANT:
				return "-----";
			case LIST:
				return " LIST";
			case EXPRESSION:
				return " EXPR";
			case BLOCK:
				return "BLOCK";
			case SYMBOL:
				return "SYMBL";
			case STRING:
				return "  STR";
			case NUMBER:
				return "  NUM";
			case DECIMAL:
				return "  DEC";
			case WHITESPACE:
				return "   WS";
			case COMMENTS:
				return " CMTS";
			case CELL:
				return " CELL";
			case EXPRESSION_MARKER:
				return "EXP-M";
			case BODY_MARKER:
				return "BOD-M";
			case TEXT_BLOCK:
				return " TEXT";	
			default:
				break;
			}
			
			return     "?????";
		}
	
		void Token::add(const Token& other, bool merge) {
			if (other.begin() < _begin || !is_valid())
				_begin = other.begin();
			
			if (other.end() > _end || !is_valid())
				_end = other.end();
			
			// Insert the children of the token in at some point:
			if (merge) {
				_children.insert(_children.end(), other._children.begin(), other._children.end());
			} else {
				_children.push_back(other);
			}
		}
	
		Token::Token() : _invalid(true), _identity(UNIMPORTANT) {
		}

		Token::Token(StringIteratorT begin, Identity identity) : _invalid(false), _begin(begin), _end(begin), _identity(identity) {
		}
		
		Token::Token(StringIteratorT begin, StringIteratorT end, Identity identity) : _invalid(false), _begin(begin), _end(end), _identity(identity) {
		}
		
		Token::Token(const Token& current, StringIteratorT end) : _invalid(false), _begin(current.begin()), _end(end), _identity(UNIMPORTANT) {
		}
		
		bool Token::is_valid() const {
			return !_invalid;
		}
		
		unsigned Token::length() const {
			if (is_valid())
				return _end - _begin;
			else
				return 0;
		}
		
		StringT Token::value() const {
			if (is_valid())
				return StringT(_begin, _end);
			else
				return "";
		}
		
		const Token& Token::operator<<(const Token& other) {
			if (is_valid() && other.is_valid()) {
				add(other, false);
			} else {
				_invalid = true;
			}
			
			return *this;
		}
		
		const Token& Token::operator+=(const Token& other) {
			if (is_valid() && other.is_valid()) {
				add(other, true);
				
				return *this;
			} else {
				return other;
			}
		}
		
		const Token& Token::operator|=(const Token& other) {
			if (!this->is_valid()) {
				*this += other;
			}
			
			return *this;
		}
		
		const Token& Token::operator&=(const Token& other) {
			if (other.is_valid()) {
				add(other, true);
			} else {
				_invalid = true;
			}
			
			return *this;
		}
		
		Token& Token::operator+=(const unsigned& count) {
			_end += count;
			
			return *this;
		}
		
		Identity Token::identity() const {
			return _identity;
		}
		
		void Token::set_identity(Identity identity) {
			_identity = identity;
		}
		
		Token::ChildrenT & Token::children() {
			return _children;
		}
		
		const Token::ChildrenT & Token::children() const {
			return _children;
		}
		
		bool Token::terminal() const {
			return _children.size() == 0;
		}
		
		Token Token::simplify() {
			if (_children.size() == 1 && _identity == _children[0].identity()) {
				return _children[0];
			} else {
				return *this;
			}
		}
		
		void Token::print_tree(std::ostream& outp, unsigned indent) const {
			//if (_identity != 0) {
				//if (terminal()) {
					outp << StringT(indent, '\t') << name_for_identity(_identity) << " : '" << value() << "'" << std::endl;
				//} else {
				//	outp << StringT(indent, '\t') << name_for_identity(_identity) << " : " << std::endl;
				//}
			//}
			
			for (ChildrenT::const_iterator i = _children.begin(); i != _children.end(); i += 1) {
				i->print_tree(outp, indent+1);
			}
		}
		
		void Token::debug() const {
			if (is_valid()) {
				std::cout << "Token: '" << value() << "'" << std::endl;
			} else {
				std::cout << "Token: Invalid" << std::endl;
			}
			
			print_tree(std::cerr, 0);
		}
		
		FatalParseFailure::FatalParseFailure (const Token& token, const char * failure_message) 
			: _token(token), _failure_message(failure_message)
		{

		}
		
		void print_indicator_line (const StringT& whitespace, std::ostream& outp, unsigned start, unsigned size) {			
			unsigned i = 0;
			
			while (i < start) {
				if (whitespace[i] == '\t')
					outp << '\t';
				else 
					outp << ' ';

				i += 1;
			}
			
			while (size > 0) {
				if (whitespace[i] == '\t')
					outp << "^\t";
				else
					outp << '^';

				i += 1;
				size -= 1;
			}
			
			outp << std::endl;
		}
		
		void print_indicator_line_to_end(const StringT& whitespace, std::ostream& outp, unsigned start, unsigned size) {
			print_indicator_line(whitespace, outp, start, size - start);
		}
		
		void print_indicator_line(const StringT& whitespace, std::ostream& outp, unsigned size) {
			print_indicator_line(whitespace, outp, 0, size);
		}
		
		void FatalParseFailure::print_error(std::ostream& outp, const SourceCode * source_code) {			
			unsigned first_offset = source_code->offset_for_iterator(_token.begin());
			unsigned last_offset = source_code->offset_for_iterator(_token.end());
			
			// Empty source code?
			if (first_offset >= source_code->size() || last_offset > source_code->size()) {
				return;
			}
			
			unsigned first_line = source_code->line_for_offset(first_offset);
			unsigned last_line = source_code->line_for_offset(last_offset);
			
			unsigned first_line_start = source_code->offset_for_line(first_line);
			unsigned last_line_start = source_code->offset_for_line(last_line);
			
			std::vector<StringT> strings = source_code->strings_for_lines(first_line, last_line);

			outp << source_code->input_name() << ":" << first_line << " " << _failure_message << std::endl;
			
			if (strings.size() == 0) {
				// Ignore this case - no lines to print
			} else if (strings.size() == 1) {
				outp << strings[0] << std::endl;
				
				unsigned length = _token.length();
				if (length == 0)
					length = 1;
				
				print_indicator_line(strings[0], outp, first_offset - first_line_start, length);
			} else {
				outp << strings[0] << std::endl;
				print_indicator_line_to_end(strings[0], outp, first_offset - first_line_start, strings[0].size());
				
				unsigned i = 1;
				for (; i < strings.size() - 1; i += 1) {
					outp << strings[i] << std::endl;
					print_indicator_line(strings[i], outp, strings[i].size());
				}
				
				outp << strings[i] << std::endl;
				print_indicator_line(strings[i], outp, 0, last_offset - last_line_start);
			}
		}
		
		const Token& FatalParseFailure::token() const {
			return _token;
		}
		
		std::ostream & operator<<(std::ostream & outp, const Token & token) {
			outp << std::endl;
			
			token.print_tree(outp);
			
			return outp;
		}
		
#pragma mark -
#pragma mark Basic Parsing Primatives
		
		Counter::Counter(std::size_t min, std::size_t max) : _min(min), _max(max), _count(0) {
		
		}
		
		bool Counter::update() {
			_count += 1;
			
			return _count < _max;
		}
		
		bool Counter::failed() {
			return _count < _min || _count > _max;
		}
		
		unsigned Counter::count() const {
			return _count;
		}
		
		Token parse_constant(StringIteratorT begin, StringIteratorT end, const StringT & constant) {
			StringIteratorT s = begin;
			StringIteratorT c = constant.begin();
			
			while (s != end && c != constant.end() && *c == *s) s++, c++;
			
			if (c == constant.end()) {
				return Token(begin, s);
			} else {
				return Token();
			}
		}
		
		Token parse_until_constant(StringIteratorT begin, StringIteratorT end, const StringT & constant) {
			StringIteratorT s = begin;
			StringIteratorT c = constant.begin();
			
			
			
			while (s != end && c != constant.end() && *c == *s) s++, c++;
			
			if (c == constant.end()) {
				return Token(begin, s);
			} else {
				return Token();
			}
		}
	}
}
