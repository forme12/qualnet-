#include <deque>
#include <portablethreads/config.h>
#include <portablethreads/mutex.h>

#include <memory> // std::allocator

namespace PortableThreads 
{
	namespace LockFree
	{
		template<typename T, typename A = std::allocator<T> >
		class PTQueue
		{
		public:
		    PartitionData *     m_partitionData;

		typedef typename std::deque <T>                  MsgQ;
		typedef typename std::deque <T>::iterator        MsgQIter;

		    // std::deque <T>    m_msgQ;
		    MsgQ                      m_msgQ;
		    // std::deque <Message *>    m_msgQ;
		PTMutex m_msgQMutex;

		    PTQueue ()
		    {
		    }
		    /*
		    PTQueue (PartitionData * partitionData) : m_partitionData (partitionData)
		    {
			// grab another pthread lock
			pthread_mutex_init(&m_msgQMutex, NULL) ;
		    }
		    */

		    void _lock ()
		    {
			    m_msgQMutex.lock ();
		    }
		    void _unlock ()
		    {
			    m_msgQMutex.unlock ();
		    }

		    void 
		    pushBack (T msg)
		    {
			// now lock
			_lock ();
			m_msgQ.push_back (msg);
			// unlock
			_unlock ();
		    }

		    bool
		    //popFront (T & value)
		    popFront (T & value)
		    {
			value = NULL;
			_lock ();
			if (m_msgQ.empty ())
            {
                _unlock ();
			    return false;
            }

			value = m_msgQ.front ();
			m_msgQ.pop_front ();
			// unlock
			_unlock ();
			return true;
		    }

		    MsgQIter
		    begin ()
		    {
			return m_msgQ.begin ();
		    }

		    MsgQIter
		    end ()
		    {
			return m_msgQ.end ();
		    }
		};
	}
}
