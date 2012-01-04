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
			ObjectT* _object;
			
		public:
			Pointer () : _object(NULL) {
			}
			
			Pointer (ObjectT * object) : _object(object) {
			}
			
			template <typename OtherObjectT>
			Pointer (OtherObjectT* object) : _object(dynamic_cast<ObjectT*>(object)) {
			}
			
			template <typename OtherObjectT>
			Pointer (Pointer<OtherObjectT> other) : _object(dynamic_cast<ObjectT*>(other.get())) {
			}
			
			ObjectT* operator-> () const {
				KAI_ENSURE(_object != NULL);
				return _object;
			}
						
			ObjectT& operator* () const {
				KAI_ENSURE(_object != NULL);
				return *_object;
			}
			
			operator ObjectT * () {
				return this->_object;
			}
			
			operator const ObjectT * () const {
				return this->_object;
			}
			
			template <typename AnyT>
			AnyT * as () {
				return dynamic_cast<AnyT *>(this->_object);
			}
			
			template <typename AnyT>
			const AnyT * as () const {
				return dynamic_cast<const AnyT *>(this->_object);
			}
			
			bool operator== (const Pointer & other) const
			{
				return _object == other._object;
			}
			
			bool operator!= (const Pointer & other) const
			{
				return _object != other._object;
			}
			
			bool operator< (const Pointer & other) const
			{
				return _object < other._object;
			}
			
			bool operator> (const Pointer & other) const
			{
				return _object > other._object;
			}
			
			bool operator<= (const Pointer & other) const
			{
				return _object <= other._object;
			}
			
			bool operator>= (const Pointer & other) const
			{
				return _object >= other._object;
			}
			
			ObjectT* get () const
			{
				return _object;
			}
			
			typedef ObjectT* Pointer::* safe_bool;
			
			operator safe_bool() const
			{
				return _object ? &Pointer::_object : 0;
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
				if (this->_object) {
					this->_object->retain();
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
				if (this->_object) {
					this->_object->release();
					this->_object = NULL;
				}
			}
		
			Reference& set (ObjectT* object) {
				clear();
				
				this->_object = object;
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
