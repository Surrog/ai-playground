
#ifndef EVENTPROXY_HPP
#define EVENTPROXY_HPP

#include <deque>

#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>

#include "Environnement_Event.h"

namespace Logique {

	template <typename deque_type>
	class DequeProxy {
	public:
		typedef typename deque_type::value_type value_type;

		DequeProxy(deque_type& stack, boost::mutex& mut)
		: _stack(&stack), _mut(&mut)
		{
			_mut->lock();
		}

		DequeProxy(const DequeProxy& orig)
			: _mut(orig._mut), _stack(orig._stack)
		{
			orig._mut->unlock();
			orig._mut = 0;
			orig._stack = 0;
			_mut->lock();
		}

		~DequeProxy()
		{
			if (_mut > 0)
				_mut->unlock();
		}

		typename deque_type::iterator begin()
		{
			return _stack->begin();
		}

		typename deque_type::const_iterator begin() const
		{
			return _stack->begin();
		}

		typename deque_type::iterator end() 
		{
			return _stack->end();
		}

		typename deque_type::const_iterator end() const
		{
			return _stack->end();
		}

		void clear() 
		{
			_stack->clear();
		}

		void foreach(const boost::function< void (const value_type&) >& func)
		{
			BOOST_FOREACH(value_type& ev, *_stack)
			{
				func(ev);
			}
		}

		std::size_t size()
		{
			return _stack.size();
		}

		void eraseFromFront(typename deque_type::iterator last)
		{
			if (last != _stack.begin())
				_stack.erase(_stack.begin(), last);
		}

		value_type popFromFront()
		{
			value_type poped = _stack.front();
			_stack.pop_front();
			return poped;
		}

	private:
		mutable deque_type* _stack;
		mutable boost::mutex* _mut;

		DequeProxy& operator=(const DequeProxy& ) { return *this; }
	};
}

#endif