//
//  Object.h
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 28/12/11.
//  Copyright (c) 2011 Orion Transfer Ltd. All rights reserved.
//

#ifndef _KAI_OBJECT_H
#define _KAI_OBJECT_H

#include "Kai.h"
#include "Reference.h"
#include "Memory/ManagedObject.h"

#include <set>

namespace Kai {
	
	class Object;
	
	/// Object Comparison helpers.
	class InvalidComparison {};
	
	enum ComparisonResult {
		// LHS > RHS:
		DESCENDING = -1,
		
		// LHS == RHS:
		EQUAL = 0,
		
		// LHS < RHS:
		ASCENDING = 1
	};
	
	template <typename ThisT>
	inline static ComparisonResult derived_compare(const ThisT * lhs, const Object * rhs) {
		const ThisT * other = dynamic_cast<const ThisT *>(rhs);
		
		if (other) {
			return lhs->compare(other);
		} else {
			throw InvalidComparison();
		}
	}
	
	class Symbol;
	class Frame;
		
	/** An object specifies the default behaviour for all objects in Kai.
	
	 An object provides a specification for looking up and invoking functionality, along with memory management provided by ManagedObject.
	 */
	class Object : public Memory::ManagedObject {
	public:
		static const char * const NAME;
		
		virtual ~Object();
		
		virtual Ref<Symbol> identity(Frame * frame) const;
		
		/// A prototype specifies the behaviour of the current value, and is potentially context dependent.
		virtual Ref<Object> prototype(Frame * frame) const;
		
		/// Lookup the given identifier. Defers to prototype by default:
		virtual Ref<Object> lookup(Frame * frame, Symbol * identifier);
		
		/// Evaluate the current value in the given context:
		virtual Ref<Object> evaluate(Frame * frame);
		
		/// Comparing objects:
		virtual ComparisonResult compare(const Object * other) const;
		
		template <typename LeftT, typename RightT>
		static ComparisonResult compare(LeftT * lhs, RightT * rhs) {
			if (lhs == rhs)
				return EQUAL;
			
			if (lhs == NULL || rhs == NULL) {
				throw InvalidComparison();
			} else {
				return lhs->compare(rhs);
			}
		}
		
		static bool equal(Object * lhs, Object * rhs) {
			try {
				return compare(lhs, rhs) == EQUAL;
			} catch (InvalidComparison ex) {
				return false;
			}
		}
		
		/// Converting objects back into readable code for debugging:
		typedef std::set<const Object *> MarkedT;
		
		virtual void to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const;
		
		inline void to_code(Frame * frame, StringStreamT & buffer) const {
			MarkedT marks;
			to_code(frame, buffer, marks, 0); 
		}
		
		static StringT to_string(Frame * frame, Object * object);
		static bool to_boolean(Frame * frame, Object * object);
		
		Ref<Object> as_value(Frame * frame);
				
#pragma mark -
				
		// Converts the argument to a string value
		static Ref<Object> to_string(Frame * frame);
		
		// Converts the argument to a boolean frame->symbol
		static Ref<Object> to_boolean(Frame * frame);
		
		// Compares the given arguments
		static Ref<Object> compare(Frame * frame);
		
		// Compares the given values and returns a true/false value
		static Ref<Object> equal(Frame * frame);
		
		/// Returns a prototype for the given object.
		static Ref<Object> prototype_(Frame * frame);
		
		/// Returns the identity of an object, that is used for context dependent lookup.
		static Ref<Object> identity_(Frame * frame);
		
		// Returns the arguments unevaluated
		static Ref<Object> value(Frame * frame);
		
		// Evaluates arguments one at a time in result of the previous.
		static Ref<Object> lookup(Frame * frame);
		
		// Performs a method call with the given function.
		static Ref<Object> call(Frame * frame);
		
		static void import(Frame * frame);
	};
	
}

#endif
