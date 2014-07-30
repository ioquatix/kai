//
//  Token.h
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 7/09/09.
//  Copyright 2009 Orion Transfer Ltd. All rights reserved.
//
//
 
#ifndef _KAI_PARSER_TOKEN_H
#define _KAI_PARSER_TOKEN_H

#include "../Kai.hpp"
#include "../Unicode/Unicode.hpp"

namespace Kai {
	class SourceCode;
	
	/// This needs some serious tidying up...
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
			
			HEREDOC = 30,
			
			/// Used for parsing '$' from string interpolations:
			EXPRESSION_MARKER = 80,
			
			/// Used for parsing '<%' from string interpolations: 
			BODY_MARKER = 82,
			
			/// Used for parsing text from string interpolations:
			TEXT_BLOCK = 84,
			
			WHITESPACE = 100,
			INDENTATION = 200,
			COMMENTS = 250
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
				
				void add(const Token&, bool merge);
				
			public:
				/// Invalid token constructor
				Token();
				
				Token(StringIteratorT begin, Identity identity = UNIMPORTANT);
				Token(StringIteratorT begin, StringIteratorT end, Identity identity = UNIMPORTANT);
				Token(const Token& current, StringIteratorT end);
				
				bool is_valid() const;
				
				typedef StringIteratorT Token::* safe_bool;
			
				operator safe_bool() const {
					return is_valid() ? &Token::_begin : 0;
				}
				
				unsigned length() const;
				StringT value() const;
				
				inline StringIteratorT begin() const { return _begin; }
				inline StringIteratorT end() const { return _end; }
				
				/// Adds the given child token as a new child of this token.
				const Token& operator<<(const Token& child);
				
				/// Merges the given child token as part of this token.
				const Token& operator+=(const Token& child);
				
				/// Merges the given token into the current token only if it isn't invalid.
				const Token& operator|=(const Token& other);
				
				/// Merges the given token into the current token.
				/// The current token becomes invalid if the given token is invalid.
				const Token& operator&=(const Token& other);
				
				Token& operator+=(const unsigned& count);
				Token& operator++() { return *this += 1; }
				
				const Token& operator||(const Token & other) {
					if (this->is_valid()) {
						return *this;
					} else {
						return other;
					}
				}
				
				Identity identity() const;
				void set_identity(Identity identity);
				
				Token& operator[](Identity i) {
					set_identity(i);
					return *this;
				}
				
				ChildrenT & children();
				const ChildrenT & children() const;
				
				bool terminal() const;
				
				void print_tree(std::ostream& outp, unsigned indent = 0) const;
				void debug() const;
				
				Token simplify();
		};
		
		struct FatalParseFailure {
			protected:
				Token _token;
				const char* _failure_message;
				
			public:
				FatalParseFailure (const Token & token, const char * failure_message);
				
				void print_error (std::ostream & outp, const SourceCode * source_code);
				
				const Token & token () const;
		};
		
		std::ostream & operator<< (std::ostream & outp, const Token & token);
		
// MARK: -
// MARK: Basic Parsing Primatives
		
		struct Counter {
			std::size_t _min, _max, _count;
			
			Counter(std::size_t min = 0, std::size_t max = -1);
			
			bool update();
			bool failed();
			
			unsigned count() const;
		};
				
		template <typename PredicateT>
		Token parse_characters(StringIteratorT begin, StringIteratorT end, PredicateT predicate) {
			StringIteratorT s = begin;
			
			while (s != end) {
				StringIteratorT t = s;
				
				Unicode::CodePointT code_point = Unicode::next(t, end);
				
				if (predicate(code_point)) {
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
		Token parse_characters(StringIteratorT begin, StringIteratorT end, PredicateT predicate, Counter counter) {
			StringIteratorT s = begin;
			
			while (s != end) {
				StringIteratorT t = s;
				
				Unicode::CodePointT code_point = Unicode::next(t, end);
				
				if (predicate(code_point)) {
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
		PrimitiveT convert(const StringT& string_value) {
			StringStreamT stream(string_value);
			
			PrimitiveT primitive_value;
			stream >> primitive_value;
			
			return primitive_value;
		}
		
		Token parse_constant(StringIteratorT begin, StringIteratorT end, const StringT & constant);
		
// MARK: -
// MARK: Operator Parsing
		
		struct OperatorParser {
			typedef std::vector<StringT> OperatorsT;
			OperatorsT operators;
			
			Token operator()(StringIteratorT begin, StringIteratorT end) const {
				for (OperatorsT::const_iterator i = operators.begin(); i != operators.end(); ++i) {
					Token t = parse_constant(begin, end, *i);
					
					if (t) return t[SYMBOL];
				}
				
				return Token();
			}
			
			OperatorParser& operator<<(const StringT & op) {
				operators.push_back(op);
				return *this;
			}
		};
		
	}
}

#endif
