/*
 *  Token.h
 *  Kai Compiler
 *
 *  Created by Samuel Williams on 7/09/09.
 *  Copyright 2009 Orion Transfer Ltd. All rights reserved.
 *
 */
 
#ifndef _KAI_PARSER_TOKEN_H
#define _KAI_PARSER_TOKEN_H

#include "../Kai.h"
#include "../Unicode/Unicode.h"

namespace Kai {
	class SourceCode;
	
	namespace Parser {
		enum Identity {
			UNIMPORTANT = 0,
			
			LIST = 5,
			EXPRESSION = 6,
			BLOCK = 7,
			CELL = 8,
			
			SYMBOL = 10,
			STRING = 20,
			NUMBER = 22,
			DECIMAL = 24,
						
			WHITESPACE = 100,
			INDENTATION = 200,
			
			COMMENTS = 300
		};
		
		struct Token {
			public:
				typedef std::vector<Token> ChildrenT;
				typedef ChildrenT::iterator IteratorT;
				
			protected:
				bool _invalid;
				
				StringIteratorT _begin, _end;

				Identity _identity;
				ChildrenT _children;
				
				void add (const Token&, bool merge);
				
			public:
				/// Invalid token constructor
				Token ();
				
				Token (StringIteratorT begin, Identity identity = UNIMPORTANT);
				Token (StringIteratorT begin, StringIteratorT end, Identity identity = UNIMPORTANT);
				Token (const Token& current, StringIteratorT end);
				
				bool isValid () const;
				
				typedef StringIteratorT Token::* safe_bool;
			
				operator safe_bool() const {
					return isValid() ? &Token::_begin : 0;
				}
				
				unsigned length () const;
				StringT value () const;
				
				inline StringIteratorT begin () const { return _begin; }
				inline StringIteratorT end () const { return _end; }
				
				/// Adds the given child token as a new child of this token.
				const Token& operator<< (const Token& child);
				
				/// Merges the given child token as part of this token.
				const Token& operator+= (const Token& child);
				
				/// Merges the given token into the current token only if it isn't invalid.
				const Token& operator|= (const Token& other);
				
				/// Merges the given token into the current token.
				/// The current token becomes invalid if the given token is invalid.
				const Token& operator&= (const Token& other);
				
				Token& operator+= (const unsigned& count);
				Token& operator++ () { return *this += 1; }
				
				const Token& operator|| (const Token & other) {
					if (this->isValid()) {
						return *this;
					} else {
						return other;
					}
				}
				
				Identity identity () const;
				void setIdentity (Identity identity);
				
				Token& operator[] (Identity i) {
					setIdentity(i);
					return *this;
				}
				
				ChildrenT & children ();
				const ChildrenT & children () const;
				
				bool terminal () const;
				
				void printTree (std::ostream& outp, unsigned indent = 0) const;
				void debug () const;
				
				Token simplify ();
		};
		
		struct FatalParseFailure {
			protected:
				Token _token;
				const char* _failureMessage;
				
			public:
				FatalParseFailure (const Token & token, const char * failureMessage);
				
				void printError (std::ostream & outp, const SourceCode & sourceCode);
				
				const Token & token () const;
		};
		
		std::ostream & operator<< (std::ostream & outp, const Token & token);
		
#pragma mark -
#pragma mark Basic Parsing Primatives
		
		struct Counter {
			unsigned _min, _max, _count;
			
			Counter (unsigned min = 0, unsigned max = -1);
			
			bool update ();
			bool failed ();
			
			unsigned count () const;
		};
				
		template <typename PredicateT>
		Token parseCharacters (StringIteratorT begin, StringIteratorT end, PredicateT predicate) {
			StringIteratorT s = begin;
			
			while (s != end) {
				StringIteratorT t = s;
				
				Unicode::CodePointT codePoint = Unicode::next(t, end);
				
				if (predicate(codePoint)) {
					s = t;
				} else {
					break;
				}
			}
			
			if (s == begin)
				return Token();
			else
				return Token(begin, s);
		}
		
		template <typename PredicateT>
		Token parseCharacters (StringIteratorT begin, StringIteratorT end, PredicateT predicate, Counter counter) {
			StringIteratorT s = begin;
			
			while (s != end) {
				StringIteratorT t = s;
				
				Unicode::CodePointT codePoint = Unicode::next(t, end);
				
				if (predicate(codePoint)) {
					s = t;
				} else {
					break;
				}
				
				if (!counter.update()) break;
			}
			
			if (counter.failed())
				return Token();
			else
				return Token(begin, s);
		}
		
		template <typename PrimitiveT>
		PrimitiveT convert (const StringT& stringValue) {
			StringStreamT stream(stringValue);
			
			PrimitiveT primitiveValue;
			stream >> primitiveValue;
			
			return primitiveValue;
		}
		
		Token parseConstant (StringIteratorT begin, StringIteratorT end, const StringT & constant);
		
#pragma mark -
#pragma mark Operator Parsing
		
		struct OperatorParser {
			typedef std::vector<StringT> OperatorsT;
			OperatorsT operators;
			
			Token operator() (StringIteratorT begin, StringIteratorT end) {
				for (OperatorsT::const_iterator i = operators.begin(); i != operators.end(); ++i) {
					Token t = parseConstant(begin, end, *i);
					
					if (t) return t[SYMBOL];
				}
				
				return Token();
			}
			
			OperatorParser& operator<< (const StringT & op) {
				operators.push_back(op);
				return *this;
			}
		};
		
	}
}

#endif
