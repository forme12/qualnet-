/*  Copyright (c) January 2005 Jean Gressmann (jsg@rz.uni-potsdam.de)
 *
 *  This is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version. 
 * 
 *	This file is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this file; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef PT_MESSAGE_QUEUE_H
#define PT_MESSAGE_QUEUE_H
#include <portablethreads/config.h>
#include <portablethreads/condition.h>
#include <portablethreads/mutex.h>
#include <deque>
#include <cassert>

namespace PortableThreads 
{
	/*! \class PTMessageQueue message_queue.h portablethreads/message_queue.h
		\brief PTMessageQueue is a blocking and waitable message queue implementation. 
		
		It's access is always synchronized, that is there is always at most one thread 
		accessing the queue at all times. Each change in the queue content is signaled
		to potential waiting theads via a condition variable.

		Threads that are waiting for change notification are resumed if the template 
		parameter C implements condition variable semantics. The default parameter 
		PTCondition does so.
		Adding a message is guaranteed to be an exclusive operation 
		if the template parameter LT implements mutex semantics. The
		default parameter PTMutex does so.
	*/
	template<typename T, typename LT = PTMutex, typename C = PTCondition, typename A = std::allocator<T> > 
	class PTMessageQueue
	{
		typedef std::deque<T, A> Queue; 
	public:
		typedef typename Queue::size_type size_type; //!< size type used 
		typedef LT lock_type; //!< lock type used
		typedef C condition_type; //!< condition variable type used
		typedef typename Queue::value_type value_type; //!< value type used
		typedef typename Queue::reference reference; //!< reference to value_type
		typedef typename Queue::const_reference const_reference;//!< const reference to value_type
		//! Create an empty message queue.
		PTMessageQueue() 
		{}
		
		/*! \brief Adds a message to the back of the message queue
		
			Waiting threads are resumed.

			\param message The message to be added.
		*/ 
		inline void pushBack(const_reference message)
		{
  			const LockGuard dynUnlock(lock_);  		
			const_cast<Queue&>(queue_).push_back(message);
			cond_.signal();	
		}
		/*! \brief Adds a message to the front of the queue
		
			Waiting threads are resumed.

			\param message The message to be added.
		*/ 
		inline void pushFront(const_reference message)
		{
  			const LockGuard dynUnlock(lock_);  		
			const_cast<Queue&>(queue_).push_front(message);
			cond_.signal();		
		}
		/*! \brief Remove message from the front of the queue. 
		
			Waiting threads are resumed.

			\param message A reference to an object of type value_type which is 
				assigned the removed value.

			\retval true A message was found in the queue and was removed.
			\retval false The queue is empty.
		*/	
		bool popFront(reference message)
		{
			const LockGuard dynUnlock(lock_);
			if(const_cast<Queue&>(queue_).empty()) 
				return false;
	  		
    		message = const_cast<Queue&>(queue_).front();
    		const_cast<Queue&>(queue_).pop_front();
			cond_.signal();
    		return true;		
		}

		/*! \brief Remove message from the back of the queue. 
		
			Waiting threads are resumed.

			\param message A reference to an object of type value_type which is 
				assigned the removed value.

			\retval true A message was found in the queue and was removed.
			\retval false The queue is empty.
		*/	
		bool popBack(reference message)
		{
			const LockGuard dynUnlock(lock_);
			if(const_cast<Queue&>(queue_).empty()) 
				return false;
	  		
    		message = const_cast<Queue&>(queue_).back();
    		const_cast<Queue&>(queue_).pop_back();
			cond_.signal();
    		return true;		
		}
		
		//! Check if the queue is empty.
		inline bool empty() const
		{
			const LockGuard dynUnlock(lock_);
			return const_cast<Queue&>(queue_).empty();				
		}

		//! Get the queue's current size.
		inline size_type size() const
		{
			const LockGuard dynUnlock(lock_);
			return const_cast<Queue&>(queue_).size();				
		}

		/*! \brief Wait for change in the queue's content.

			The semantics of this method depend on the
			template parameter C.
		*/
		inline void wait() const
		{
			cond_.wait();
		}
		/*! \brief Wait for a period for change in the queue's content.

			The semantics of this method depend on the
			template parameter C.
		*/
		inline bool wait(unsigned seconds, unsigned milliseconds = 0, unsigned wakeup = 50) const
		{
			return cond_.wait(seconds, milliseconds, wakeup);
		}
	private:
		// MessageQueues cannot be copied or assigned b/c
		// neither mutexes nor conditions can.
		PTMessageQueue(const PTMessageQueue&);
		PTMessageQueue& operator=(const PTMessageQueue&);
	private:
		typedef typename Queue::iterator QueueIt;
		typedef PTGuard<lock_type> LockGuard;

 		Queue queue_;
		mutable lock_type lock_;	
		mutable condition_type cond_;
	};
}
#endif
