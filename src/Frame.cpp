/*
 *  Frame.cpp
 *  This file is part of the "Kai" project, and is licensed under the GNU GPLv3.
 *
 *  Created by Samuel Williams on 12/04/10.
 *  Copyright 2010 Samuel Williams. All rights reserved.
 *
 */

#include "Frame.h"
#include "Function.h"
#include <iostream>

namespace Kai {

	Tracer::Statistics::Statistics() : totalTime(0), count(0)
	{
	
	}

	void Tracer::enter(Value* value)
	{
		Statistics & stats = m_statistics[value];
		
		stats.count++;
		stats.frames.push_back(Time());
	}
	
	void Tracer::exit(Value* value)
	{
		Statistics & stats = m_statistics[value];
		
		stats.totalTime += (Time() - stats.frames.back());
		stats.frames.pop_back();
	}
	
	void Tracer::dump(std::ostream & buffer)
	{
		buffer << "Tracer Statistics" << std::endl;
		
		for (StatisticsMapT::iterator iter = m_statistics.begin(); iter != m_statistics.end(); iter++) {
			const Ref<Value> function = iter->first;
			Statistics & stats = iter->second;
			
			buffer << "\t";
			StringStreamT code;
			function->toCode(code);
			buffer << code.str();
			
			buffer << "\t\t" << stats.totalTime;
			buffer << "\t" << stats.count;
			buffer << std::endl;
		}
	}
	
	Ref<Value> Tracer::dump(Frame * frame)
	{
		Tracer * tracer = NULL;
		
		frame->extract()(tracer);
		
		StringStreamT buffer;
		tracer->dump(buffer);
		
		return new String(buffer.str());
	}
	
	Ref<Value> Tracer::prototype ()
	{
		return globalPrototype();
	}
	
	Ref<Value> Tracer::globalPrototype ()
	{
		static Ref<Table> g_prototype;
		
		if (!g_prototype) {
			g_prototype = new Table;
			g_prototype->setPrototype(Value::globalPrototype());
			
			g_prototype->update(sym("dump"), KFunctionWrapper(Tracer::dump));
		}
		
		return g_prototype;
	}
	
	void Tracer::import (Table * context)
	{
		context->update(sym("tracer"), globalTracer());
		context->update(sym("Tracer"), globalPrototype());
	}
	
	Tracer * Tracer::globalTracer ()
	{
		static Tracer * tracer = NULL;
		
		if (tracer == NULL) {
			tracer = new Tracer;
		}
		
		return tracer;
	}
	
	struct Trace {
		Frame * m_frame;
		
		Trace(Frame * frame) : m_frame(frame) {
			Tracer::globalTracer()->enter(m_frame->function());
		}
		
		~Trace() {
			Tracer::globalTracer()->exit(m_frame->function());
		}
	};

#pragma mark -

	Frame::Frame (Value * scope)
		: m_previous(this), m_scope(scope), m_message(NULL), m_function(NULL), m_arguments(NULL), m_depth(0)
	{

	}
	
	Frame::Frame (Value * scope, Frame * previous)
		: m_previous(previous), m_scope(scope), m_message(previous->m_message), m_function(m_previous->m_function),
		m_arguments(previous->m_arguments)
	{
		m_depth = m_previous->m_depth + 1;
#ifdef KAI_DEBUG
		std::cerr << "Frame: " << this << " Arguments: " << arguments() << std::endl;
#endif
	}

	Frame::Frame (Value * scope, Cell * message, Frame * previous)
		: m_previous(previous), m_scope(scope), m_message(message), m_function(NULL), m_arguments(NULL)
	{
		m_depth = m_previous->m_depth + 1;
	}
	
	Frame::~Frame () {
	}
	
	void Frame::mark() {
		if (marked()) return;
		
		ManagedObject::mark();
		
		if (m_previous) m_previous->mark();
		if (m_scope) m_scope->mark();
		if (m_message) m_message->mark();
		if (m_function) m_function->mark();
		if (m_arguments) m_arguments->mark();
	}
	
	Ref<Value> Frame::lookup (Symbol * identifier, Frame *& frame)
	{
		Ref<Value> result = NULL;
		
		if (m_scope) {
			result = m_scope->lookup(identifier);
			frame = this;
		}
		
		if (!result && !top()) {
			result = m_previous->lookup(identifier, frame);
		}
		
		return result;
	}
	
	Ref<Value> Frame::lookup (Symbol * identifier)
	{
		Frame * frame = NULL;
		
		return lookup(identifier, frame);
	}
	
	Ref<Value> Frame::apply () {
#ifdef KAI_DEBUG
		std::cerr << "-- " << Value::toString(m_message) << " <= " << Value::toString(m_scope) << std::endl;
		std::cerr << StringT(m_depth, '\t') << "Fetching Function " << Value::toString(m_message->head()) << std::endl;
#endif

		m_function = m_message->head()->evaluate(this);

#ifdef KAI_DEBUG
		std::cerr << StringT(m_depth, '\t') << "Executing Function " << Value::toString(m_function) << std::endl;		
		this->debug(true);
#endif

		if (!m_function) {
			throw Exception("Invalid Function", m_message->head(), this);
		}

#ifdef KAI_TRACE
		// trace will be deconstructed even in the event of an exception.
		Trace trace(this);
#endif
		
		return m_function->evaluate(this);
	}

	Ref<Value> Frame::call (Value * scope, Cell * message)
	{
		if (message == NULL) {
			throw Exception("Invalid Message", this);
		}
	
		Ref<Frame> frame = new Frame(scope, message, this);
		
#ifdef KAI_DEBUG
		std::cerr << "Stack pointer: " << (void*)&frame << std::endl;
		std::cerr << "Allocating new frame at address " << frame << " from frame " << this << " in scope " << scope << " with message " << message << std::endl;
		frame->debug(false);
#endif
		
		return frame->apply();
	}

	Cell * Frame::message () {
		return m_message;
	}

	Frame * Frame::previous () {
		return m_previous;
	}

	Ref<Value> Frame::scope () {
		Ref<Value> scope = m_scope;
		
		if (!scope) {
			Frame * cur = m_previous;
			
			while (!scope && cur) {
				scope = cur->m_scope;
				
				cur = cur->m_previous;
			}
		}
		
		return scope;
	}

	Ref<Value> Frame::function () {
		return m_function;
	}

	Cell * Frame::operands () {
		if (m_message)
			return m_message->tailAs<Cell>();
		else
			return NULL;
	}

	// With optimisations turned on, this function seems to cause stack frames to be reused and cause problems..!?
	Cell * Frame::unwrap () {
#ifdef KAI_DEBUG
		std::cerr << "Unwrapping with frame: " << this << std::endl;
#endif
		if (m_arguments) return m_arguments;
		
		Cell * last = NULL;
		Cell * cur = operands();
		
		while (cur) {
			Ref<Value> value = NULL;
			
			// If cur->head() == NULL, the result is also NULL.
			if (cur->head())
				value = cur->head()->evaluate(this);
			
			last = Cell::append(last, value, m_arguments);
			
			cur = cur->tail().as<Cell>();
		}
		
		return m_arguments;
	}

	Cell * Frame::arguments () {
		return m_arguments;
	}

	bool Frame::top () {
		return this == m_previous;
	}
	
	Cell::ArgumentExtractor Frame::extract (bool evaluate) {
		Cell * args = NULL;
		
		if (evaluate) {
			args = unwrap();
		} else {
			args = operands();
		}
		
		if (args == NULL) {
		//	throw Exception("No arguments provided!", this);
			// Dummy for extraction of null arguments.
			args = new Cell(NULL, NULL);
		}
		
		return args->extract(this);
	}

	void Frame::debug (bool ascend) {
		Frame * cur = this;
		
		do {
			Cell cell(cur->function(), cur->operands());
			
			std::cerr << "Frame " << cur << ":" << std::endl;
			
#ifdef KAI_DEBUG
			if (cur->scope())
				std::cerr << "\t Scope: " << Value::toString(cur->scope()) << std::endl;
#endif
			

			std::cerr << "\t Function: " << Value::toString(cell.head()) << std::endl;
			std::cerr << "\t Message: " << Value::toString(m_message) << std::endl;
			
#ifdef KAI_DEBUG
			if (cur->arguments()) {
				std::cerr << "\t Arguments: " << Value::toString(cur->arguments()) << std::endl;
			}
#endif
			
			cur = cur->previous();
		} while (!cur->top() && ascend);
	}
	
	Ref<Value> Frame::benchmark (Frame * frame)
	{		
		Integer * times;
		Value * exec, * result;
		
		frame->extract()(times)(exec);
		
		int count = times->value();
		
		Time start;
		
		while (count > 0) {
			result = exec->evaluate(frame);
			
			count--;			
		}
		
		Time end;
		
		Time duration = (end - start);
		
		std::cerr << "Total time for " << times->value() << " runs = " << duration << std::endl;
		std::cerr << "Average time taken = " << (duration / times->value()) << std::endl;

		return result;
	}
	
	void Frame::import (Table * context) {
		context->update(sym("this"), KFunctionWrapper(Frame::scope));
		context->update(sym("trace"), KFunctionWrapper(Frame::trace));
		context->update(sym("unwrap"), KFunctionWrapper(Frame::unwrap));
		context->update(sym("wrap"), KFunctionWrapper(Frame::wrap));
		context->update(sym("with"), KFunctionWrapper(Frame::with));
		context->update(sym("update"), KFunctionWrapper(Frame::update));
		context->update(sym("benchmark"), KFunctionWrapper(Frame::benchmark));
		context->update(sym("tracer"), Tracer::globalTracer());


		//context->update(sym("defines"), KFunctionWrapper(Frame::where));
	}
/*	
	Ref<Value> Frame::where (Frame * frame)
	{
		Symbol * identifier = NULL;
		
		frame->extract()(identifier);
		
		Frame * location = NULL;
		frame->lookup(identifier, location);
		
		return location;
	}
*/	
	// Attempt to update inplace a value in a frame
	Ref<Value> Frame::update (Frame * frame)
	{
		Symbol * identifier = NULL;
		Value * newValue = NULL;
		
		frame->extract()(identifier)[newValue];
		
		Frame * location = NULL;
		frame->lookup(identifier, location);
		
		if (location) {
			Table * scope = location->scope().as<Table>();
			
			if (scope) {
				scope->update(identifier, newValue);
			} else {
				throw Exception("Non-table Scope", location->scope(), frame);
			}
		} else {
			throw Exception("Invalid Variable Name", identifier, frame);
		}
		
		return newValue;		
	}
	
	Ref<Value> Frame::scope (Frame * frame) {
		return frame->scope();
	}
	
	Ref<Value> Frame::trace (Frame * frame) {
		Cell * arguments = frame->unwrap();
		
		std::cerr << Value::toString(frame->message()) << " -> " << Value::toString(arguments) << std::endl;
		
		return NULL;
	}
	
	class Wrapper : public Value {
		protected:
			Ref<Value> m_value;
		
		public:
			Wrapper (Value * value) : m_value(value)
			{
			
			}
			
			virtual Ref<Value> evaluate (Frame * frame) {
				Cell * arguments = frame->unwrap();
				Cell * message = new Cell(m_value, arguments);
				return frame->call(NULL, message);
			}
			
			Ref<Value> value ()
			{
				return m_value;
			}
			
			virtual void toCode(StringStreamT & buffer, MarkedT & marks, std::size_t indentation) {
				buffer << "(wrapper ";

				if (m_value)
					m_value->toCode(buffer);
					
				buffer << ')';
			}
	};
	
	Ref<Value> Frame::wrap (Frame * frame) {	
		Value * function;
		frame->extract()(function);
		return new Wrapper(Cell::create(sym("value"))(function));
	}
	
	class Unwrapper : public Value {
		protected:
			Ref<Value> m_value;
			
		public:
			Unwrapper (Value * value) : m_value(value)
			{
			
			}
			
			virtual Ref<Value> evaluate (Frame * frame) {
				Cell * operands = frame->operands();
				Cell * message = new Cell(m_value);
				Cell * next = message;
				Symbol * value = sym("value");
				
				while (operands != NULL) {
					next = next->append(
						Cell::create(value)(operands->head())
					);
					
					operands = operands->tailAs<Cell>();
				}
				
				return frame->call(NULL, message);
			}
			
			Ref<Value> value ()
			{
				return m_value;
			}
			
			virtual void toCode(StringStreamT & buffer, MarkedT & marks, std::size_t indentation) {
				buffer << "(unwrapper ";

				if (m_value)
					m_value->toCode(buffer);
					
				buffer << ')';
			}
	};
	
	Ref<Value> Frame::unwrap (Frame * frame) {
		Value * function;
		
		frame->extract()(function);
		
		Wrapper * wrapper = ptr(function).as<Wrapper>();
		
		if (wrapper) {
			return wrapper->value();
		} else {
			return new Unwrapper(Cell::create(sym("value"))(function));
		}
	}
	
	Ref<Value> Frame::with (Frame * frame) {
		Cell * cur = frame->operands();
		Ref<Value> scope = frame->scope();
		Frame * next = frame;
				
		while (cur != NULL) {
			scope = cur->head()->evaluate(next);
			next = new Frame(scope, next);
			
			cur = cur->tailAs<Cell>();
		}
		
		return scope;
	}

}
