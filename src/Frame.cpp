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
#include "Table.h"
#include "Cell.h"
#include "Number.h"
#include "String.h"
#include "Symbol.h"

//#define KAI_DEBUG

namespace Kai {

	Tracer::Statistics::Statistics() : total_time(0), count(0) {
	
	}
	
	Tracer::~Tracer() {
		
	}
	
	Ref<Symbol> Tracer::identity(Frame * frame) const {
		return frame->sym("Tracer");
	}
	
	void Tracer::mark(Memory::Traversal * traversal) const {
		for (StatisticsMapT::const_iterator i = _statistics.begin(); i != _statistics.end(); ++i) {
			traversal->traverse(i->first);
		}
	}

	void Tracer::enter(Object * value)
	{
		Statistics & stats = _statistics[value];
		
		stats.count++;
		stats.frames.push_back(Time());
	}
	
	void Tracer::exit(Object * value)
	{
		Statistics & stats = _statistics[value];
		
		stats.total_time += (Time() - stats.frames.back());
		stats.frames.pop_back();
	}
	
	void Tracer::dump(Frame * frame, std::ostream & buffer)
	{
		buffer << "Tracer Statistics" << std::endl;
		
		for (StatisticsMapT::const_iterator i = _statistics.begin(); i != _statistics.end(); ++i) {
			const Object * function = i->first;
			const Statistics & stats = i->second;
			
			buffer << "\t";
			StringStreamT code;
			function->to_code(frame, code);
			buffer << code.str();
			
			buffer << "\t\t" << stats.total_time;
			buffer << "\t" << stats.count;
			buffer << std::endl;
		}
	}
	
	Ref<Object> Tracer::dump(Frame * frame)
	{
		Tracer * tracer = NULL;
		
		frame->extract()(tracer, "self");
		
		StringStreamT buffer;
		tracer->dump(frame, buffer);
		
		return new(frame) String(buffer.str());
	}
	
	void Tracer::import(Frame * frame)
	{
		Table * prototype = new(frame) Table;
		
		prototype->update(frame->sym("dump"), KAI_BUILTIN_FUNCTION(Tracer::dump));
		
		Tracer * tracer = new(frame) Tracer;
		
		frame->update(frame->sym("tracer"), tracer);
		frame->update(frame->sym("Tracer"), prototype);
	}
	
	struct Trace {
		Frame * _frame;
		Ref<Tracer> _tracer;
		
		Trace(Frame * frame) : _frame(frame) {
			_tracer = _frame->lookup(_frame->sym("tracer"));
			
			_tracer->enter(_frame->function());
		}
		
		~Trace() {
			_tracer->exit(_frame->function());
		}
	};

#pragma mark -

	const char * const Frame::NAME = "Frame";
	
	Frame::Frame(Object * scope) : _previous(NULL), _scope(scope), _message(NULL), _function(NULL), _arguments(NULL), _depth(0) {
		_allocator = this->ObjectAllocation::allocator();
	}
	
	Frame::Frame(Object * scope, Frame * previous) : _previous(previous), _scope(scope), _message(previous->_message), _function(previous->_function), _arguments(previous->_arguments)
	{
		_allocator = previous->allocator();
		
		_depth = _previous->_depth + 1;
#ifdef KAI_DEBUG
		std::cerr << "Frame: " << this << " Arguments: " << arguments() << std::endl;
#endif
	}

	Frame::Frame(Object * scope, Cell * message, Frame * previous) : _previous(previous), _scope(scope), _message(message), _function(NULL), _arguments(NULL) {
		_allocator = previous->_allocator;
		
		_depth = _previous->_depth + 1;
	}
	
	Memory::PageAllocation * Frame::allocator() const {
		return _allocator;
	}
	
	Frame::~Frame() {
	}
	
	Ref<Symbol> Frame::identity(Frame * frame) const {
		return frame->sym("Frame");
	}
	
	Ref<Object> Frame::update(Symbol * identifier, Object * value, bool local) {
		Frame * frame = NULL;
		
		if (local) {
			frame = this;
			
			while (!frame->_scope) {
				frame = frame->previous();
			}
		} else {
			lookup(identifier, frame);
		}
		
		Table * scope = ptr(frame->_scope).as<Table>();
		
		// Potentially use an interface for this operation, rather than hard coding Table.
		scope->update(identifier, value);
		
		return scope;
	}
	
	void Frame::mark(Memory::Traversal * traversal) const {
		traversal->traverse(_previous);
		traversal->traverse(_scope);
		traversal->traverse(_message);
		traversal->traverse(_function);
		traversal->traverse(_arguments);
	}
	
	Ref<Object> Frame::lookup(Symbol * identifier, Frame *& frame) {
		Ref<Object> result = NULL;
		
		if (_scope) {
			result = _scope->lookup(this, identifier);
			frame = this;
		}
		
		if (!result && !top()) {
			result = _previous->lookup(identifier, frame);
		}
		
		return result;
	}
	
	Ref<Object> Frame::lookup(Symbol * identifier) {
		Frame * frame = NULL;
		
		return lookup(identifier, frame);
	}
	
	Ref<Object> Frame::lookup(Frame * frame, Symbol * identifier) {
		return this->lookup(identifier);
	}
	
	Ref<Object> Frame::apply() {
#ifdef KAI_DEBUG
		std::cerr << "-- " << Object::to_string(this, _message) << " <= " << Object::to_string(this, _scope) << std::endl;
		std::cerr << StringT(_depth, '\t') << "Fetching Function " << Object::to_string(this, _message->head()) << std::endl;
#endif

		_function = _message->head()->evaluate(this);

#ifdef KAI_DEBUG
		std::cerr << StringT(_depth, '\t') << "Executing Function " << Object::to_string(this, _function) << std::endl;		
		this->debug(true);
#endif

		if (!_function) {
			throw Exception("Invalid Function", _message->head(), this);
		}

#ifdef KAI_TRACE
		// trace will be deconstructed even in the event of an exception.
		Trace trace(this);
#endif
		
		return _function->evaluate(this);
	}

	Ref<Object> Frame::call(Object * scope, Cell * message) {
		if (message == NULL) {
			throw Exception("Invalid Message", this);
		}
		
		Ref<Frame> frame = new(this) Frame(scope, message, this);
		
#ifdef KAI_DEBUG
		std::cerr << "Stack pointer: " << (void*)&frame << std::endl;
		std::cerr << "Allocating new frame at address " << frame << " from frame " << this << " in scope " << scope << " with message " << message << std::endl;
		frame->debug(false);
#endif
		
		return frame->apply();
	}

	Cell * Frame::message() {
		return _message;
	}

	Frame * Frame::previous() {
		return _previous;
	}

	Object * Frame::scope() {
		Ref<Object> scope = _scope;
		
		if (!scope) {
			Frame * cur = _previous;
			
			while (!scope && cur) {
				scope = cur->_scope;
				
				cur = cur->_previous;
			}
		}
		
		return scope;
	}

	Ref<Object> Frame::function() {
		return _function;
	}

	Cell * Frame::operands() {
		if (_message)
			return _message->tail().as<Cell>();
		else
			return NULL;
	}

	// With optimisations turned on, this function seems to cause stack frames to be reused and cause problems..!?
	Cell * Frame::unwrap() {
#ifdef KAI_DEBUG
		std::cerr << "Unwrapping with frame: " << this << std::endl;
#endif
		if (_arguments) return _arguments;
		
		Cell * last = NULL;
		Cell * cur = operands();
		
		while (cur) {
			Ref<Object> value = NULL;
			
			// If cur->head() == NULL, the result is also NULL.
			if (cur->head())
				value = cur->head()->evaluate(this);
			
			last = Cell::append(this, last, value, _arguments);
			
			cur = cur->tail().as<Cell>();
		}
		
		return _arguments;
	}

	Cell * Frame::arguments() {
		return _arguments;
	}

	bool Frame::top() {
		return _previous == NULL;
	}
	
	ArgumentExtractor Frame::extract(bool evaluate) {
		Cell * args = NULL;
		
		if (evaluate) {
			args = unwrap();
		} else {
			args = operands();
		}
		
		// We don't need this as long as all arguments are optional.. otherwise, as expected, required arguments will cause an error.
		//if (args == NULL) {
		//	throw Exception("No arguments provided!", this);
			// Dummy for extraction of null arguments.
		//	args = new Cell(NULL, NULL);
		//}
		
		return args->extract(this);
	}

	void Frame::debug(bool ascend) {
		Frame * cur = this;
		
		do {
			Cell cell(cur->function(), cur->operands());
			
			std::cerr << "Frame " << cur << ":" << std::endl;
			
#ifdef KAI_DEBUG
			if (cur->scope())
				std::cerr << "\t Scope: " << Object::to_string(this, cur->scope()) << std::endl;
#endif
			

			std::cerr << "\t Function: " << Object::to_string(this, cell.head()) << std::endl;
			std::cerr << "\t Message: " << Object::to_string(this, _message) << std::endl;
			
#ifdef KAI_DEBUG
			if (cur->arguments()) {
				std::cerr << "\t Arguments: " << Object::to_string(this, cur->arguments()) << std::endl;
			}
#endif
			
			cur = cur->previous();
		} while (!cur->top() && ascend);
	}
	
	Ref<Object> Frame::benchmark(Frame * frame)
	{		
		Integer * times;
		Object * callback, * result;
		
		frame->extract()(times, "times")(callback, "callback");
		
		Math::IntermediateT count = times->value().to_intermediate();
		
		Time start;
		
		while (count > 0) {
			result = callback->evaluate(frame);
			
			count--;			
		}
		
		Time end;
		
		Time duration = (end - start);
		
		std::cerr << "Total time for " << times->value() << " runs = " << duration << std::endl;
		std::cerr << "Average time taken = " << (duration / times->value().to_intermediate()) << std::endl;

		return result;
	}
		
	Ref<Object> Frame::where(Frame * frame)
	{
		Symbol * identifier = NULL;
		
		frame->extract()(identifier, "identifier");
		
		Frame * location = NULL;
		frame->lookup(identifier, location);
		
		return location;
	}
	
	// Attempt to update inplace a value in a frame
	Ref<Object> Frame::update(Frame * frame)
	{
		Symbol * identifier = NULL;
		Object * new_value = NULL;
		
		frame->extract()(identifier, "identifier")[new_value];
		
		Frame * location = NULL;
		frame->lookup(identifier, location);
		
		if (location) {
			// TODO: Maybe improve the level of abstraction so we are not completely depenedent on Tables for scope.
			Table * scope = ptr(location->scope()).as<Table>();
			
			if (scope) {
				scope->update(identifier, new_value);
			} else {
				throw Exception("Non-table Scope", location->scope(), frame);
			}
		} else {
			throw Exception("Invalid Variable Name", identifier, frame);
		}
		
		return new_value;		
	}
	
	Ref<Object> Frame::scope(Frame * frame) {
		return frame->scope();
	}
	
	Ref<Object> Frame::trace(Frame * frame) {
		Cell * arguments = frame->unwrap();
		
		std::cerr << Object::to_string(frame, frame->message()) << " -> " << Object::to_string(frame, arguments) << std::endl;
		
		return NULL;
	}
	
	class Wrapper : public Object {
	protected:
		Object * _value;
	
	public:
		Wrapper(Object * value) : _value(value) {
		
		}
	
		virtual void mark(Memory::Traversal * traversal) const {
			traversal->traverse(_value);
		}
			
		virtual Ref<Object> evaluate(Frame * frame) {
			Cell * arguments = frame->unwrap();
			
			Cell * message = new(frame) Cell(_value, arguments);
			
			std::cerr << "Message: " << Object::to_string(frame, message) << std::endl;
			
			return frame->call(NULL, message);
		}
		
		Ref<Object> value() {
			return _value;
		}
		
		virtual void to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const {
			buffer << "(wrapper ";

			if (_value)
				_value->to_code(frame, buffer);
				
			buffer << ')';
		}
	};
	
	Ref<Object> Frame::wrap(Frame * frame) {	
		Object * function;
		
		frame->extract()(function, "function");
		
		return new(frame) Wrapper(Cell::create(frame)(frame->sym("value"))(function));
	}
	
	class Unwrapper : public Object {
		protected:
			Object * _value;
			
		public:
			Unwrapper(Object * value) : _value(value) {
			}
			
			virtual void mark(Memory::Traversal * traversal) const {
				traversal->traverse(_value);
			}
			
			virtual Ref<Object> evaluate(Frame * frame) {
				Cell * operands = frame->operands();
				Cell * message = new(frame) Cell(_value);
				Cell * next = message;
				Symbol * value = frame->sym("value");
				
				while (operands != NULL) {
					next = next->append(
						Cell::create(frame)(value)(operands->head())
					);
					
					operands = operands->tail().as<Cell>();
				}
				
				return frame->call(NULL, message);
			}
			
			Ref<Object> value() {
				return _value;
			}
			
			virtual void to_code(Frame * frame, StringStreamT & buffer, MarkedT & marks, std::size_t indentation) const {
				buffer << "(unwrapper ";

				if (_value)
					_value->to_code(frame, buffer);
					
				buffer << ')';
			}
	};
	
	Ref<Object> Frame::unwrap(Frame * frame) {
		Object * function;
		
		frame->extract()(function, "function");
		
		Wrapper * wrapper = ptr(function).as<Wrapper>();
		
		if (wrapper) {
			return wrapper->value();
		} else {
			return new(frame) Unwrapper(Cell::create(frame)(frame->sym("value"))(function));
		}
	}
	
	Ref<Object> Frame::with(Frame * frame) {
		Cell * cur = frame->operands();
		Ref<Object> scope = frame->scope();
		Frame * next = frame;
				
		while (cur != NULL) {
			scope = cur->head()->evaluate(next);
			next = new(frame) Frame(scope, next);
			
			cur = cur->tail().as<Cell>();
		}
		
		return scope;
	}
	
	Ref<Object> Frame::operands(Frame * frame) {
		return frame->operands();
	}
	
	Ref<Object> Frame::arguments(Frame * frame) {
		return frame->unwrap();
	}
		
	void Frame::import(Frame * frame) {
		Table * prototype = new(frame) Table;
		
		frame->update(frame->sym("Frame"), prototype);
		
		frame->update(frame->sym("this"), KAI_BUILTIN_FUNCTION(Frame::scope));
		frame->update(frame->sym("trace"), KAI_BUILTIN_FUNCTION(Frame::trace));
		frame->update(frame->sym("unwrap"), KAI_BUILTIN_FUNCTION(Frame::unwrap));
		frame->update(frame->sym("wrap"), KAI_BUILTIN_FUNCTION(Frame::wrap));
		frame->update(frame->sym("with"), KAI_BUILTIN_FUNCTION(Frame::with));
		frame->update(frame->sym("update"), KAI_BUILTIN_FUNCTION(Frame::update));
		frame->update(frame->sym("benchmark"), KAI_BUILTIN_FUNCTION(Frame::benchmark));
		
		frame->update(frame->sym("arguments"), KAI_BUILTIN_FUNCTION(Frame::arguments));
		frame->update(frame->sym("operands"), KAI_BUILTIN_FUNCTION(Frame::operands));
		
		frame->update(frame->sym("defines"), KAI_BUILTIN_FUNCTION(Frame::where));
	}
	
#pragma mark -
	
	Symbol * Frame::sym(const char * name) {
		return new(this) Symbol(name);
	}
}
