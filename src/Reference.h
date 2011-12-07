/*
 *  Reference.h
 *  This file is part of the "Dream" project, and is licensed under the GNU GPLv3.
 *
 *  Created by Samuel Williams on 3/10/09.
 *  Copyright 2009 Samuel Williams. All rights reserved.
 *
 */

#ifndef _DREAM_REFERENCE_H
#define _DREAM_REFERENCE_H

#include "Ensure.h"

#include <cstddef>
#include <algorithm>
#include <stdint.h>
#include <iostream>

namespace Kai {

	/* Why use these?
	
		Reference counting is expensive and mostly needless.
		
		The only place you need to increment a reference count is when 
		you need to save an object outside of the lexical scope. In this
		case use Reference<>.
		
		When you have a function that can accept either
			- A raw pointer
			- A Reference<>
		use Pointer<>
		
		When you use Pointer<>, you won't increment reference count. Another
		option is to use const Reference<> &, however you can't convert a
		raw pointer to this type.
	*/
	
	void debugAllocations ();
	
	class SharedObject {
		public:
			typedef int32_t NumberT;
		protected:
			/// The number of references to this instance.
			mutable volatile NumberT m_count;
		
		public:
			/// Default constructor. Sets the reference count to 0.
			SharedObject ();

			/// Default copy constructor. Sets the reference count of this object to 0.
			SharedObject (const SharedObject & other);
			
			/// Copy operator. Doesn't modify reference count.
			SharedObject & operator= (const SharedObject & other);
			
			virtual ~SharedObject ();
			
			void retain () const;
			
			/// Decrement the reference count - delete the object if zero.
			/// @returns true if the object was deleted.
			bool release () const;
			
			// delete this object
			void deallocate () const;
			
			NumberT referenceCount () const;
	};
	
	template <typename ObjectT>
	class Pointer {
		protected:
			ObjectT* m_object;
			
		public:
			Pointer () : m_object(NULL) {
			}
			
			Pointer (ObjectT * object) : m_object(object) {
			}
			
			template <typename OtherObjectT>
			Pointer (OtherObjectT* object) : m_object(dynamic_cast<ObjectT*>(object)) {
			}
			
			template <typename OtherObjectT>
			Pointer (Pointer<OtherObjectT> other) : m_object(dynamic_cast<ObjectT*>(other.get())) {
			}
			
			ObjectT* operator-> () const {
				KAI_ENSURE(m_object != NULL);
				return m_object;
			}
						
			ObjectT& operator* () const {
				KAI_ENSURE(m_object != NULL);
				return *m_object;
			}
			
			operator ObjectT * () {
				return this->m_object;
			}
			
			operator const ObjectT * () const {
				return this->m_object;
			}
			
			template <typename AnyT>
			AnyT * as () {
				return dynamic_cast<AnyT *>(this->m_object);
			}
			
			template <typename AnyT>
			const AnyT * as () const {
				return dynamic_cast<const AnyT *>(this->m_object);
			}
			
			bool operator== (const Pointer & other) const
			{
				return m_object == other.m_object;
			}
			
			bool operator!= (const Pointer & other) const
			{
				return m_object != other.m_object;
			}
			
			bool operator< (const Pointer & other) const
			{
				return m_object < other.m_object;
			}
			
			bool operator> (const Pointer & other) const
			{
				return m_object > other.m_object;
			}
			
			bool operator<= (const Pointer & other) const
			{
				return m_object <= other.m_object;
			}
			
			bool operator>= (const Pointer & other) const
			{
				return m_object >= other.m_object;
			}
			
			ObjectT* get () const
			{
				return m_object;
			}
			
			typedef ObjectT* Pointer::* safe_bool;
			
			operator safe_bool() const
			{
				return m_object ? &Pointer::m_object : 0;
			}
			
			template <typename OtherObjectT>
			Pointer<OtherObjectT> dynamicCast () const {
				return Pointer<OtherObjectT>(dynamic_cast<OtherObjectT*>(m_object));
			}
	};
	
	template <typename ValueT>
	using Ptr = Pointer<ValueT>;
	
	template <typename ValueT>
	Ptr<ValueT> ptr(ValueT * value) {
		return value;
	}
	
	void markStaticAllocation (void*);
	
	template <typename ObjectT>
	class Static : public Pointer<ObjectT> {
		public:
			Static () : Pointer<ObjectT>(new ObjectT) {
				this->m_object->retain();
				markStaticAllocation(this->m_object);
			}
			
			Static (ObjectT* object) : Pointer<ObjectT>(object) {
				this->m_object->retain();
				markStaticAllocation(this->m_object);
			}
			
			~Static () {
				this->m_object->release();
			}
	};
	
	template <typename ObjectT>
	class Reference : public Pointer<ObjectT> {
		private:
			void construct () {
				if (this->m_object)
					this->m_object->retain();
			}
			
			template <typename OtherObjectT>
			static ObjectT* extract (OtherObjectT* object) {
				return object;
			}
			
			template <typename OtherObjectT>
			static ObjectT* extract (Pointer<OtherObjectT>& object) {
				return object.get();
			}
			
		public:
			void clear () {
				if (this->m_object) {
					this->m_object->release();
					this->m_object = NULL;
				}
			}
		
			Reference& set (ObjectT* object) {
				clear();
				
				if (object) {
					object->retain();
					this->m_object = object;
				}
				
				return *this;
			}
			
			Reference () {
			}
			
			Reference (ObjectT* object) : Pointer<ObjectT>(object) {
				construct();
			}
			
			Reference (const Reference& other) : Pointer<ObjectT>(other.get()) {
				construct();
			}

			template <typename OtherObjectT>			
			Reference (Pointer<OtherObjectT> other) : Pointer<ObjectT>(other.get()) {
				construct();
			}
			
			Reference& operator= (const Reference& other) {
				return set(other.get());
			}
			
			template <typename OtherObjectT>
			Reference& operator= (Pointer<OtherObjectT>& other) {
				return set(other.get());
			}
			
			Reference& operator= (ObjectT* object) {
				return set(object);
			}
			
			template <typename OtherObjectT>
			Reference& operator= (OtherObjectT* object) {
				return set(dynamic_cast<ObjectT*>(object));
			}
		
			~Reference () {
				clear();
			}
	};
	
	template <typename ValueT>
	using Ref = Reference<ValueT>;
	
	/// This wrapper is primarily for return value semantics to avoid reference count increment and decrement as the object moves along the stack.
	/// A transient MUST be assigned to a Reference in order to be cleared up correctly.
	template <typename ObjectT>
	class Transient : public Pointer<ObjectT> {		
	protected:
		Transient& operator= (Transient& other);
		
	public:
		/// This is used to implement move semantics.
		ObjectT * revoke ()
		{
			ObjectT * object = this->m_object;
			this->m_object = NULL;
			
			return object;
		}
		
		Transient (ObjectT* object) : Pointer<ObjectT>(object) {
			// We take ownership of this object.
		}
		
		Transient (Transient& other) : Pointer<ObjectT>(other.revoke()) {
		}
		
		template <typename OtherObjectT>
		Transient (Transient<OtherObjectT> & other) : Pointer<ObjectT>(other.revoke()) {
		}
		
		template <typename OtherObjectT>
		Transient (const Pointer<OtherObjectT> & other) : Pointer<ObjectT>(other.get()) {
		}
		
		// If we still have ownership over the object and it hasn't been retained by anyone else,
		// deallocate it.
		~Transient () {
			if (this->m_object->referenceCount() == 0)
				this->m_object->deallocate();
		}
	};
	
	template <typename ValueT>
	using Ret = Reference<ValueT>;
	
	template <typename ValueT>
	class Shared
	{
		protected:
			Ref<SharedObject> m_controller;
			ValueT * m_value;
		
		public:
			Ref<SharedObject> controller () const
			{
				return m_controller;
			}
			
			Shared ()
				: m_controller(NULL), m_value(NULL)
			{
			
			}
			
			~Shared ()
			{
				// If this is the last Shared<...> we delete the value.
				if (m_value && m_controller->referenceCount() == 1) {
					delete m_value;
				}
			}
			
			Shared (ValueT * value)
				: m_controller(new SharedObject), m_value(value)
			{
				
			}
			
			Shared (const Shared & other)
				: m_controller(other.m_controller), m_value(other.m_value)
			{
				
			}
			
			template <typename OtherValueT>
			Shared (OtherValueT * object)
				: m_controller(new SharedObject), m_value(dynamic_cast<ValueT*>(object))
			{
			
			}
			
			template <typename OtherValueT>
			Shared (Shared<OtherValueT> other)
			{
				m_value = dynamic_cast<ValueT*>(other.get());
				
				if (m_value)
					m_controller = other.controller();
				else
					m_controller = NULL;
			}
			
			ValueT * get () const
			{
				return m_value;
			}
			
			/// Copy operator. Doesn't modify reference count.
			Shared & operator= (const Shared & other)
			{
				m_controller = other.m_controller;
				m_value = other.m_value;
				
				return *this;
			}
			
			/// Copy operator. Doesn't modify reference count.
			template <typename OtherValueT>
			Shared & operator= (const Shared<OtherValueT> & other)
			{
				m_value = dynamic_cast<ValueT*>(other.get());
				
				if (m_value)
					m_controller = other.controller();
				
				return *this;
			}
			
			template <typename OtherValueT>
			Shared & operator= (OtherValueT * other)
			{
				m_value = dynamic_cast<ValueT*>(other);
				
				if (m_value)
					m_controller = new SharedObject;
				
				return *this;
			}
			
			ValueT* operator-> () const {
				KAI_ENSURE(m_value != NULL);
				return m_value;
			}
						
			ValueT& operator* () const {
				KAI_ENSURE(m_value != NULL);
				return *m_value;
			}
			
			typedef ValueT* Shared::* safe_bool;
			
			operator safe_bool() const
			{
				return m_value ? &Shared::m_value : 0;
			}
	};
}

#endif
