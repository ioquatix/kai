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
			
			FUNCTION = 26,
			CHAIN = 28,
			VALUE = 30,
			
			BINARY_OPERATOR = 32,
			BINARY_OPERATOR_FUNCTION = 33,
			LEFT_UNARY_OPERATOR = 42,
			RIGHT_UNARY_OPERATOR = 52,
			
			ARGUMENTS = 60,
			
			WHITESPACE = 100,
			INDENTATION = 200,
			
			COMMENTS = 300
		};
		
		// Token value = parseFloat(begin, end) || parseHex(begin, end) || parseInt(begin, end);
		
		struct Token {
			public:
				typedef std::vector<Token> ChildrenT;
				typedef ChildrenT::iterator IteratorT;
				
			protected:
				StringIteratorT m_begin, m_end;

				Identity m_identity;
				ChildrenT m_children;
				
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
					return isValid() ? &Token::m_begin : 0;
				}
				
				unsigned length () const;
				StringT value () const;
				
				inline StringIteratorT begin () const { return m_begin; }
				inline StringIteratorT end () const { return m_end; }
				
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
		
		/// Indicates we need to backtrack
		struct ParseFailure {
		};
		
		inline Token invalid () {
			return Token();
		}
		
		struct FatalParseFailure {
			protected:
				Token m_token;
				const char* m_failureMessage;
				
			public:
				FatalParseFailure (const Token & token, const char * failureMessage);
				
				void printError (std::ostream & outp, const SourceCode & sourceCode);
				
				const Token & token () const;
		};
		
		std::ostream & operator<< (std::ostream & outp, const Token & token);
		
#pragma mark -
#pragma mark Basic Parsing Primatives
		
		struct Counter {
			unsigned m_min, m_max, m_count;
			
			Counter (unsigned min = 0, unsigned max = -1);
			
			bool update ();
			bool failed ();
			
			unsigned count () const;
		};
		
		typedef bool(*ParserPredicate)(StringIteratorT);
		
		template <typename PredicateT>
		Token parseCharacters (StringIteratorT begin, StringIteratorT end, PredicateT predicate) {
			StringIteratorT s = begin;
			
			while (s != end && predicate(s)) s++;
			
			if (s == begin)
				return invalid();
			else
				return Token(begin, s);
		}
		
		template <typename PredicateT>
		Token parseCharacters (StringIteratorT begin, StringIteratorT end, PredicateT predicate, Counter counter) {
			StringIteratorT s = begin;
			
			while (s != end && predicate(s)) {
				s++;
				
				if (!counter.update()) break;
			}
			
			if (counter.failed())
				return invalid();
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
		
		bool isAlpha (StringIteratorT i);
		bool isNumeric (StringIteratorT i);
		bool isAlphaNumeric (StringIteratorT i);
		
		bool isTab (StringIteratorT i);
		bool isSpace (StringIteratorT i);
		bool isWhitespace (StringIteratorT i);
		bool isNewline (StringIteratorT i);
		bool isNotNewline (StringIteratorT i);
		
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

		template <typename TermParserT, typename OperatorParserT>
		struct BinaryOperatorParser {
			TermParserT termParser;
			OperatorParserT operatorParser;
			
			Token operator() (StringIteratorT begin, StringIteratorT end) {
				Token t(begin, BINARY_OPERATOR);
				
				t += parseCharacters(t.end(), end, isWhitespace);
				
				t << termParser(t.end(), end);
				
				while (t) {
					Token c(t.end(), BINARY_OPERATOR_FUNCTION);
					
					c += parseCharacters(c.end(), end, isWhitespace);
					
					c << operatorParser(c.end(), end);
					if (!c) break;
					
					c += parseCharacters(c.end(), end, isWhitespace);
					
					c << termParser(c.end(), end);
					if (!c) break;
					
					t << c;
				}
				
				return t.simplify();
			}
		};
		
		template <typename TermParserT, typename OperatorParserT>
		BinaryOperatorParser<TermParserT, OperatorParserT> binaryOperatorParser (TermParserT termParser, OperatorParserT operatorParser) {
			BinaryOperatorParser<TermParserT, OperatorParserT> parser;
			
			parser.termParser = termParser;
			parser.operatorParser = operatorParser;
			
			return parser;
		}
	}
}

#endif
