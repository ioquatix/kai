/*
 *  Token.cpp
 *  Kai Compiler
 *
 *  Created by Samuel Williams on 7/09/09.
 *  Copyright 2009 Orion Transfer Ltd. All rights reserved.
 *
 */

#include "Token.h"
#include "../SourceCode.h"

namespace Kai {
	namespace Parser {
		const char* nameForIdentity (Identity identity) {
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
			default:
				break;
			}
			
			return     "?????";
		}
	
		void Token::add (const Token& other, bool merge) {
			if (other.begin() < m_begin || m_begin == StringIteratorT())
				m_begin = other.begin();
			
			if (other.end() > m_end || m_end == StringIteratorT())
				m_end = other.end();
			
			// Insert the children of the token in at some point:
			if (merge) {
				m_children.insert(m_children.end(), other.m_children.begin(), other.m_children.end());
			} else {
				m_children.push_back(other);
			}
		}
	
		Token::Token () : m_invalid(true), m_identity(UNIMPORTANT) {
		}

		Token::Token (StringIteratorT begin, Identity identity) : m_invalid(false), m_begin(begin), m_end(begin), m_identity(identity) {
		}
		
		Token::Token (StringIteratorT begin, StringIteratorT end, Identity identity) : m_invalid(false), m_begin(begin), m_end(end), m_identity(identity) {
		}
		
		Token::Token (const Token& current, StringIteratorT end) : m_invalid(false), m_begin(current.begin()), m_end(end), m_identity(UNIMPORTANT) {
		}
		
		bool Token::isValid () const {
			return !m_invalid;
		}
		
		unsigned Token::length () const {
			if (isValid())
				return m_end - m_begin;
			else
				return 0;
		}
		
		StringT Token::value () const {
			if (m_begin != StringIteratorT())
				return StringT(m_begin, m_end);
			else
				return "";
		}
		
		const Token& Token::operator<< (const Token& other) {
			if (isValid() && other.isValid()) {
				add(other, false);
			} else {
				// Become invalid
				m_begin = m_end = StringIteratorT();
			}
			
			return *this;
		}
		
		const Token& Token::operator+= (const Token& other) {
			if (isValid() && other.isValid()) {
				add(other, true);
				
				return *this;
			} else {
				return other;
			}
		}
		
		const Token& Token::operator|= (const Token& other) {
			if (!this->isValid()) {
				*this += other;
			}
			
			return *this;
		}
		
		const Token& Token::operator&= (const Token& other) {
			if (other.isValid()) {
				add(other, true);
			} else {
				m_invalid = true;
			}
			
			return *this;
		}
		
		Token& Token::operator+= (const unsigned& count) {
			m_end += count;
			
			return *this;
		}
		
		Identity Token::identity () const {
			return m_identity;
		}
		
		void Token::setIdentity (Identity identity) {
			m_identity = identity;
		}
		
		Token::ChildrenT & Token::children () {
			return m_children;
		}
		
		const Token::ChildrenT & Token::children () const {
			return m_children;
		}
		
		bool Token::terminal () const {
			return m_children.size() == 0;
		}
		
		Token Token::simplify () {
			if (m_children.size() == 1 && m_identity == m_children[0].identity()) {
				return m_children[0];
			} else {
				return *this;
			}
		}
		
		void Token::printTree (std::ostream& outp, unsigned indent) const {
			//if (m_identity != 0) {
				//if (terminal()) {
					outp << StringT(indent, '\t') << nameForIdentity(m_identity) << " : '" << value() << "'" << std::endl;
				//} else {
				//	outp << StringT(indent, '\t') << nameForIdentity(m_identity) << " : " << std::endl;
				//}
			//}
			
			for (ChildrenT::const_iterator i = m_children.begin(); i != m_children.end(); i += 1) {
				i->printTree(outp, indent+1);
			}
		}
		
		void Token::debug () const {
			if (isValid()) {
				std::cout << "Token: '" << value() << "'" << std::endl;
			} else {
				std::cout << "Token: Invalid" << std::endl;
			}
			
			printTree(std::cerr, 0);
		}
		
		FatalParseFailure::FatalParseFailure (const Token& token, const char * failureMessage) 
			: m_token(token), m_failureMessage(failureMessage)
		{

		}
		
		void printIndicatorLine (const StringT& whitespace, std::ostream& outp, unsigned start, unsigned size) {			
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
		
		void printIndicatorLineToEnd (const StringT& whitespace, std::ostream& outp, unsigned start, unsigned size) {
			printIndicatorLine(whitespace, outp, start, size - start);
		}
		
		void printIndicatorLine (const StringT& whitespace, std::ostream& outp, unsigned size) {
			printIndicatorLine(whitespace, outp, 0, size);
		}
		
		void FatalParseFailure::printError (std::ostream& outp, const SourceCode & sourceCode) {			
			unsigned firstOffset = sourceCode.offsetForIterator(m_token.begin());
			unsigned lastOffset = sourceCode.offsetForIterator(m_token.end());
			
			// Empty source code?
			if (firstOffset >= sourceCode.size() || lastOffset > sourceCode.size()) {
				return;
			}
			
			unsigned firstLine = sourceCode.lineForOffset(firstOffset);
			unsigned lastLine = sourceCode.lineForOffset(lastOffset);
			
			unsigned firstLineStart = sourceCode.offsetForLine(firstLine);
			unsigned lastLineStart = sourceCode.offsetForLine(lastLine);
			
			std::vector<StringT> strings = sourceCode.stringsForLines(firstLine, lastLine);

			outp << sourceCode.inputName() << ":" << firstLine << " " << m_failureMessage << std::endl;
			
			if (strings.size() == 0) {
				// Ignore this case - no lines to print
			} else if (strings.size() == 1) {
				outp << strings[0] << std::endl;
				
				unsigned length = m_token.length();
				if (length == 0)
					length = 1;
				
				printIndicatorLine(strings[0], outp, firstOffset - firstLineStart, length);
			} else {
				outp << strings[0] << std::endl;
				printIndicatorLineToEnd(strings[0], outp, firstOffset - firstLineStart, strings[0].size());
				
				unsigned i = 1;
				for (; i < strings.size() - 1; i += 1) {
					outp << strings[i] << std::endl;
					printIndicatorLine(strings[i], outp, strings[i].size());
				}
				
				outp << strings[i] << std::endl;
				printIndicatorLine(strings[i], outp, 0, lastOffset - lastLineStart);
			}
		}
		
		const Token& FatalParseFailure::token () const {
			return m_token;
		}
		
		std::ostream & operator<< (std::ostream & outp, const Token & token) {
			outp << std::endl;
			
			token.printTree(outp);
			
			return outp;
		}
		
#pragma mark -
#pragma mark Basic Parsing Primatives
		
		Counter::Counter (unsigned min, unsigned max) : m_min(min), m_max(max), m_count(0) {
		
		}
		
		bool Counter::update () {
			m_count += 1;
			
			return m_count < m_max;
		}
		
		bool Counter::failed () {
			return m_count < m_min || m_count > m_max;
		}
		
		unsigned Counter::count () const {
			return m_count;
		}
		
		Token parseConstant (StringIteratorT begin, StringIteratorT end, const StringT & constant) {
			StringIteratorT s = begin;
			StringIteratorT c = constant.begin();
			
			while (s != end && c != constant.end() && *c == *s) s++, c++;
			
			if (c == constant.end()) {
				return Token(begin, s);
			} else {
				return Token();
			}
		}
		
		bool isAlpha (StringIteratorT i) {
			return (*i >= 'a' && *i <= 'z') || (*i >= 'A' && *i <= 'Z') || *i == '_';
		}
		
		bool isNumeric (StringIteratorT i) {
 			return (*i >= '0' && *i <= '9');
		}
		
		bool isAlphaNumeric (StringIteratorT i) {
			return isAlpha(i) || isNumeric(i);
		}
		
		bool isTab (StringIteratorT i) {
			return *i == '\t';
		}
		
		bool isSpace (StringIteratorT i) {
			return *i == ' ';
		}
		
		bool isWhitespace (StringIteratorT i) {
			return *i == ' ' || *i == '\t' || *i == '\r' || *i == '\n';
		}
		
		bool isNewline (StringIteratorT i) {
			return *i == '\r' || *i == '\n';
		}
		
		bool isNotNewline (StringIteratorT i) {
			return !isNewline(i);
		}
	}
}
