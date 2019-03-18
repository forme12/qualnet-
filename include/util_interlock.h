#ifndef __UTIL_INTERLOCK_H__
# define __UTIL_INTERLOCK_H__

#include <sys/types.h>
#include <fcntl.h>

#ifdef _WIN32
#include "pthread.h"
#else
#include <pthread.h>
#endif

#include <list>

#include "util_atomic.h"

#define INTERLOCK_DEBUG (false)

namespace UTIL 
{
    
    static std::string bool2str(bool b)
    {
        if (b) 
        {
            return std::string("true");
        }
        
        return std::string("false");
    }
    
    class IpcLock 
    {
        pthread_cond_t m_wake;
        pthread_mutex_t m_mutex;
        bool m_started;
        
    public:
            
        IpcLock() : m_started(false)
        {
        }

        void start()
        {
            if (!m_started)
            {
                pthread_cond_init(&m_wake, NULL);
                pthread_mutex_init(&m_mutex, NULL);

                m_started = true;
            }
        }
        
        ~IpcLock()
        {
            if (m_started)
            {
                pthread_cond_destroy(&m_wake);
                pthread_mutex_destroy(&m_mutex);

                m_started = false;
            }
        }
        
        void take() 
        {
            pthread_mutex_lock(&m_mutex);
        }
        
        void give()
        {
            pthread_mutex_unlock(&m_mutex);
        }
        
        void wait_xxx()
        {
            ERROR_Assert(pthread_cond_wait(&m_wake, 
                                           &m_mutex) == 0,
                         "QualNet has encountered an error in which"
                         " the synchronization behavior expected"
                         " did not occur.  This is a serious error"
                         " and the program must terminate.  Please"
                         " report this to QualNet support for"
                         " further investigation.");            
        }
        
        void sleep(bool giveAfter)
        {
            if (INTERLOCK_DEBUG)
            {
                printf("sleep(%p)\n", this);
                printf("sleep->Attempting wait\n");
            }
            
            take();
            
            wait_xxx();
            
            if (giveAfter)
            {
                give();
            }
            
            if (INTERLOCK_DEBUG)
            {
                printf("sleep->Wait succeeded/semaphore unlocked\n");
            }
        }

        void wake_one_xxx()
        {
            pthread_cond_signal(&m_wake);
        }

        void wake_all_xxx()
        {
            pthread_cond_broadcast(&m_wake);
        }
        
        void wake_one()
        {   
            if (INTERLOCK_DEBUG) 
            {
                printf("wake_one(%p)\n", this);
                printf("wake->Attempting signal\n");
            }
            
            take();
            wake_one_xxx();
            give();
            
            if (INTERLOCK_DEBUG)
            {
                printf("wake->Signal succeeded\n");
            }
        }
        
        void wake() { wake_one(); }
        
        void wake_all()
        {
            if (INTERLOCK_DEBUG) 
            {
                printf("wake_all(%p)\n", this);
                printf("wake->Attempting broadcast\n");
            }
            
            take();
            wake_all_xxx();
            give();
            
            if (INTERLOCK_DEBUG)
            {
                printf("wake->broadcast succeeded\n");
            }
        }
    } ;
    
    template <class T> class Worker 
    {
        typedef std::list<T> ListType;
    public:
        virtual void run(ListType& l) = 0;
        virtual void close() = 0;
    } ;
    
    template <class T> class Interlock : public IpcLock
    {
        typedef std::list<T> ListType;

        Worker<T>* m_worker;
        std::string m_name;
        
        pthread_t m_thread;
        
        bool m_runWhileLocked;

        int m_inserts; 
        int m_execs;

        ListType m_list;

        UTIL_AtomicInteger m_run_flag;

        UTIL_AtomicInteger m_finalizable;

        void set(int x)
        {
            UTIL_AtomicSet(&m_run_flag, x);
        }

        volatile int get()
        {
            return (volatile int)UTIL_AtomicRead(&m_run_flag);
        }
        
public: void start()
        {
            if (INTERLOCK_DEBUG) printf("Starting interlock process.\n");

            IpcLock::start();
            pthread_create(&m_thread, 
                           NULL, 
                           Interlock::runout,
                           (void*)this);

            UTIL_AtomicSet(&m_finalizable, 1);
        }

private: int spin_busy()
        {
            unsigned long spin(0);

            bool done(false);
            do
            {
                spin++;
                done = get() == 0;
            } while (!done);
 
            if (INTERLOCK_DEBUG)
            {
                printf("spin = %lu\n", spin); 
            }

            return spin;
        }

    public:
            
        Interlock(std::string name, 
                  Worker<T>* worker,
                  bool runWhileLocked)
        : m_name(name), m_worker(worker), 
          m_runWhileLocked(runWhileLocked), m_inserts(0),
          m_execs(0)
        {
            UTIL_AtomicSet(&m_finalizable, 0);
        }

        Interlock(std::string name,
                  bool runWhileLocked)
        : m_name(name), m_worker(NULL), 
          m_runWhileLocked(runWhileLocked), m_inserts(0),
          m_execs(0)
        {
            UTIL_AtomicSet(&m_finalizable, 0);
        }
        
        void sync()
        {
            if (INTERLOCK_DEBUG)
            {
                printf("sync(%p): \n", this);
            }

            set(1);
            wake_one();
            spin_busy();

            if (INTERLOCK_DEBUG)
            {
                printf("sync complete...\n");
            }
        }

        void setWorker(Worker<T>* worker)
        {
            bool wasNull(false);

            if (m_worker == NULL)
            {
                wasNull = true;
            }

            m_worker = worker;

            if (wasNull)
            {
                start();
            }
        }

        ~Interlock() 
        {

        }
        
        void finalize()
        {
            void *dummy(NULL);

            if (UTIL_AtomicDecrementAndTest(&m_finalizable) == 0)
            { 
                if (INTERLOCK_DEBUG) printf("Interlock already finalized.\n");
                return;
            }

            if (INTERLOCK_DEBUG)
            {   
                printf("Joining interlock worker.\n");
            }

            sync();
            
            pthread_cancel(m_thread);
            pthread_join(m_thread, &dummy);
            
            if (INTERLOCK_DEBUG)
            {
                printf("Joined.\n");
            }

            int remainingExecs = m_list.size();

            if (INTERLOCK_DEBUG)
            {
                printf("%d remaining items in work list after join.\n",
                       remainingExecs);
            }

            if (remainingExecs > 0)
            {
                m_execs += remainingExecs;
                m_worker->run(m_list); 
            }

            remainingExecs = m_list.size();
   
            if (INTERLOCK_DEBUG)
            {
                printf("Completed remaining work items (list_size=%d).\n",
                       remainingExecs);
            }

            assert(remainingExecs == 0);

            if (INTERLOCK_DEBUG)
            {
                printf("Closing module.\n");
            }

            m_worker->close();

            if (INTERLOCK_DEBUG)
            {
                printf("Module closed.\n");
            }
            
            if (INTERLOCK_DEBUG)
            {
                printf("Inserts: %d, Execs: %d: Remaining: %d\n", 
                       m_inserts, 
                       m_execs,
                       remainingExecs);
            }
        }

        static void clean(void *data)
        {
            Interlock* it = (Interlock*)data;

            if (INTERLOCK_DEBUG)
            {
                printf("Cleaning up mutex in interlock\n");
            }

            it->set(0);
            it->give();
            
            if (INTERLOCK_DEBUG)
            {
                printf("Cleaned...\n");
            }
        }
        
        static void* runout(void *data)
        {
            Interlock* it = (Interlock*)data;

            pthread_cleanup_push(Interlock::clean, data);

            int oldtype(0);
            pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, 
                                  &oldtype);
            
            int oldstate(0);
            pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 
                                   &oldstate);
            
            std::list<T> workList;
            
            bool first(true);
            for(;;)
            {
                if (first)
                {
                    if (INTERLOCK_DEBUG)
                    {
                        printf("Interlock thread[%s] first"
                               " run--setting to active\n",
                               it->m_name.c_str());
                    }
                    it->set(0);
                    
                    first = false;
                }

                it->take();

                if (it->m_list.size() == 0)
                {
                    if (INTERLOCK_DEBUG)
                    {
                        printf("IT[%s] sleeping...\n", 
                               it->m_name.c_str());
                    }

                    it->set(0);
                
                    it->wait_xxx();
                    if (INTERLOCK_DEBUG)
                    {
                        printf("IT[%s] woken up...finding work\n", 
                               it->m_name.c_str());
                    }
                }
                
                it->m_execs += it->m_list.size();

                workList.merge(it->m_list);

                if (it->m_runWhileLocked)
                {
                    if (INTERLOCK_DEBUG)
                    {
                        printf("IT[%s]: Spawning worked (locked=TRUE)\n",
                               it->m_name.c_str());
                    }
                    
                    it->m_worker->run(workList);
                    it->give();
                }
                else
                {
                    it->give();

                    if (INTERLOCK_DEBUG)
                    {
                        printf("IT[%s]: Spawning worked (locked=FALSE)\n",
                               it->m_name.c_str());
                    }

                    it->m_worker->run(workList);

                    int work_list_size = workList.size();
                    assert(work_list_size == 0);
                }
                
                pthread_testcancel();
            }

            int dummy;
                
            pthread_setcanceltype(oldtype, &dummy);
            pthread_setcancelstate(oldstate, &dummy);

            pthread_cleanup_pop(1);
            
            return NULL;
        }

        int push_back_xxx(T msg)
        {
            m_list.push_back(msg);
            m_inserts++;

            return m_list.size();
        }
        
        int push_back(T msg, 
                      bool wakeAfterInsert)
        {
            int size(0);

            if (INTERLOCK_DEBUG)
            {
                printf("UTIL_InterlockInsertMessage(it=%s, "
                       "wakeAfter=%s\n",
                       m_name.c_str(), 
                       bool2str(wakeAfterInsert).c_str());
            }
            
            take();

            size = push_back_xxx(msg);
            
            give();

            if (wakeAfterInsert)
            {
                force_wake();
            }

            return size;
        }
        
        void force_wake()
        {
            if (INTERLOCK_DEBUG)
            {
                printf("UTIL_Interlock[%s]: Waking worker...\n",
                       m_name.c_str());
                
            }
            
            wake();
        }   
    } ;

}

#undef INTERLOCK_DEBUG

#endif /* __UTIL_INTERLOCK_H__ */
