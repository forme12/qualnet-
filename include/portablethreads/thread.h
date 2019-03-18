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

#ifndef PT_THREAD_H
#define PT_THREAD_H
#include <memory>
#include <stdexcept>
#include <portablethreads/config.h>
#ifdef PT_WINDOWS
#	include <portablethreads/win32/thread.h>
#endif
#ifdef PT_UNIX
#	include <portablethreads/unix/thread.h>
#endif
#include <portablethreads/utility.h>
#include <portablethreads/mutex.h>

namespace PortableThreads 
{
	namespace Private
	{
		class ThreadManager;
	}

	/*!	\class PThread thread.h portablethreads/thread.h 
		\brief Implements a thread of control which may be executed concurrently to all other threads.
	*/
	class PThread
	{	
		friend class Private::ThreadManager;
		typedef OSSpecific::ThreadTraits OSThreadTraits;
	public:
		typedef void* thread_id_type; //!< Thread identifier (pod type).
		/*! \brief Operation succeeded.
			\hideinitializer
		*/
		static const int OK;
		/*! \brief A thread is already running in this PThread object.
			\hideinitializer
		*/
		static const int ALREADY_RUNNING;
		/*! \brief Join failed because there is no thread running at the time.
			\hideinitializer
		*/
		static const int NOT_RUNNING;
		/*! \brief Join failed the calling thread is the thread to be joined.
			\hideinitializer
		*/
		static const int DEADLOCK;
		/*! \brief The thread object has been destroyed previous to the return of the call.
			\hideinitializer
		*/
		static const int ABANDONED;
		
		/*! \brief The thread is currently running.
			\hideinitializer
		*/		
		static const int RUNNING;
		/*! \brief The thread is currently not running.
			\hideinitializer
		*/		
		static const int STOPPED;
		/*! \brief There is no PThread object associated with this thread identifier at this time.
			\hideinitializer
		*/
		static const int DESTROYED;
	public:
		virtual ~PThread();
		PThread();
	    /*! \brief Get the identifier of the PThread object.
			
			Each PThread object is guaranteed to have an identifier
			that compares different (!=) to all other PThread objects.
		*/
		thread_id_type id() const;
		/*! \brief Start the thread.
			
			Upon returning from this method a thread will be started using the 
			threadMain() method as an entry point for the thread. Per PThread
			object only one thread is allowed at any time.
			
			\retval OK A thread has been created.
			\retval NOT_RUNNING The object has no thread object at this time.

			\exception PTResourceError
		*/
		int run();

		/*! \brief Wait for a thread to finish. 

			Blocks the calling thread until the thread of execution of the PThead object 
			has finished.

			\retval OK The thread has ended. 
			\retval ALREADY_RUNNING This object already has a thread associated with it. 
			\retval DEADLOCK The method was called from within the implementation of threadMain(). 
			
		*/
		int join();

		/*!	\brief Get status information about a thread.

			As the implementation of PThread may reuse thread identifiers returned
			by id() for different PThread objects an identifiers of an PThread object
			may be the same as the identifier of an already destroyed PThread object.

			\param id A PThread identifier returned by id()

			\retval RUNNING The thread is currently running.
			\retval STOPPED The thread is currently not running.
			\retval DESTROYED There is not PThread object associated with the identifier at this time.
		*/
		static int poll(thread_id_type id);
	protected:
		//! Causes the calling thread to give up the CPU.
		void give() const;
		/*!	\brief Entry method of the thread.
		
			The code in the implementation of this method
			gets executed in an own thread of execution that is 
			different from the thread that invoked the run() method.

			Any exception thrown within the context of a thread are
			caught. Upon the catching of an exception the unexpectedException()
			method is invoked.
		*/
		virtual void threadMain() = 0;
		/*!	\brief Handles any uncaught exceptions thrown in the context of a thread.
		
			The default exception handler merely prints a notification message
			to STDOUT that an exception has been caught. After the message has
			been printed the thread of control ends as if it would have returned
			normally from threadMain().
		*/
		virtual void unexpectedException() throw();
	private:
		// Entry point for all threads
		static OSThreadTraits::entry_function_t entry;
		// common static code for all threads
		static void entry_common(void* arg);
		void id(thread_id_type id);
		void setObjectDestructionNotifier(volatile bool* notifiy);
		 
		// Threads cannot be copied or assigned ;-)
		PThread(const PThread&);
		PThread& operator=(const PThread&);		
	private:
		volatile thread_id_type id_;
		volatile bool* notifyOnDestruction_;
		int revision_;
	private:
		static std::auto_ptr<Private::ThreadManager>  manager_;
	};	


	//! Comparison operators for threads
	inline bool operator==(const PThread& lhs, const PThread& rhs)
	{
		return lhs.id() == rhs.id();
	}

	//! Comparison operators for threads
	inline bool operator!=(const PThread& lhs, const PThread& rhs)
	{
		return !(lhs == rhs);
	}
}

#endif
