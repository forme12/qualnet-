
/*! \mainpage PortableThreads Documentation

	\section use_thread Hello multi-threaded world!
	
	Creating a thread is as simple as deriving from PortableThreads::PThread 
	and implementing the pure virtual method PThread::threadMain():

	\code
	#include <iostream>
	#include <portablethreads/thread.h>

	using namespace std;

	class HelloWorld : public PortableThreads::PThread 
	{
		void threadMain()
		{
			cout << "Hello multi-threaded world!" << endl;
		}
	};

	int main()
	{
		HelloWorld h;
		h.run();
		h.join();

		return 0;
	}
	\endcode

	When run the program outputs: <b>Hello multi-threaded world!</b>


	\section use_producer_consumer A Producer-Consumer example
	
	The Producer-Consumer is a popular concurrent pattern in which one or more
	producing threads forward their data to consuming threads.

	\code
	#include <iostream>
	#include <portablethreads/thread.h>
	#include <portablethreads/message_queue.h>
	#include <portablethreads/time.h>

	using namespace std;

	typedef int MessageType;
	const MessageType TerminiateMessage = -1;
	typedef PortableThreads::PTMessageQueue<MessageType> Queue;

	class Producer : public PortableThreads::PThread 
	{
	public:
		Producer(Queue& queue, int itemsToProduce)
			:	queue_(&queue)
			,	itemsToProduce_(itemsToProduce)
		{}
	private:
		void threadMain()
		{
			for(int i = 0; i < itemsToProduce_; ++i)
			{
				// assume item production takes some time
				PortableThreads::pt_milli_sleep(50);
				queue_->pushBack(i);
			}

			// done producing, send terminiate message
			queue_->pushBack(TerminiateMessage);
		}
	private:
		Queue* queue_;
		int itemsToProduce_;
	};

	class Consumer : public PortableThreads::PThread 
	{
	public:
		Consumer(Queue& queue)
			:	queue_(&queue)
		{}
	private:
		void threadMain()
		{
			cout << "consuming ";
			MessageType m;
			while(true)
			{
				// wait for a period, then check in case we missed a
				// message
				queue_->wait(0, 10); 
				if(queue_->popFront(m)) // got message
				{
					if(m == TerminiateMessage)
						break;
					else
						cout << m << " ";
				}
			}
			cout << "done" << endl;
		}
	private:
		Queue* queue_;
	};

	int main()
	{
		Queue q;
		Producer p(q, 10);
		Consumer c(q);

		p.run();
		c.run();

		p.join();
		c.join();

		return 0;
	}
	\endcode

	When run the program outputs: <b>consuming 0 1 2 3 4 5 6 7 8 9 done</b>


	\section copying Copying

	Copyright (c) 2005 Jean Gressmann
	
	PortableThreads are distributed under the GNU Public License. 
	Get a copy of the license at http://www.gnu.org/copyleft/gpl.html

*/


/*!	\file portablethreads/config.h

	\namespace PortableThreads	
	\brief This name space contains the implemenation of PortableThreads.

	\namespace PortableThreads::LockFree
	\brief This name space contains the lock-free part of PortableThreads.
*/


namespace PortableThreads
{
	typedef <arch-dependent> int8; //!< Signed integer which is 8 bit wide.
	typedef <arch-dependent> uint8; //!< Unsigned integer which is 8 bit wide.
	typedef <arch-dependent> int16; //!< Signed integer which is 16 bit wide.
	typedef <arch-dependent> uint16; //!< Unsigned integer which is 16 bit wide.
	typedef <arch-dependent> int32; //!< Signed integer which is 32 bit wide.
	typedef <arch-dependent> uint32; //!< Unsigned integer which is 32 bit wide.
	typedef <arch-dependent> int64; //!< Signed integer which is 64 bit wide.
	typedef <arch-dependent> uint64; //!< Unsigned integer which is 64 bit wide.
}





