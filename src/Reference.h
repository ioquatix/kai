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
		
			void mark() {
				if (m_object)
					m_object->mark();
			}
	};
	
	template <typename ObjectT>
	using Ptr = Pointer<ObjectT>;
	
	template <typename ObjectT>
	Ptr<ObjectT> ptr(ObjectT * object) {
		return object;
	}
	
	template <typename ObjectT>
	class Reference : public Pointer<ObjectT> {
		private:
			void construct () {
				if (this->m_object) {
					this->m_object->retain();
				}
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
				
				this->m_object = object;
				construct();
				
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
	
	template <typename ObjectT>
	using Ref = Reference<ObjectT>;
}

#endif
