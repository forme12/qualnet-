// Copyright (c) 2001-2009, Scalable Network Technologies, Inc.  All Rights Reserved.
//                          6100 Center Drive
//                          Suite 1250
//                          Los Angeles, CA 90045
//                          sales@scalable-networks.com
//
// This source code is licensed, not sold, and is subject to a written
// license agreement.  Among other things, no portion of this source
// code may be copied, transmitted, disclosed, displayed, distributed,
// translated, used as the basis for a derivative work, or used, in
// whole or in part, for any program or purpose other than its intended
// use in compliance with the license agreement as part of the QualNet
// software.  This source code and certain of the algorithms contained
// within it are confidential trade secrets of Scalable Network
// Technologies, Inc. and may not be used as the basis for any other
// software, hardware, product or service.

// /**
// PACKAGE     :: EXTERNAL
// DESCRIPTION :: This file defines the generic interface to external modules.
// **/

#ifndef _EXTERNAL_H_
#define _EXTERNAL_H_

#include <vector>

#include <time.h>

#ifdef _WIN32
#include "pthread.h"
#else
#include <pthread.h>
#endif // _WIN32

// Forward declarations
struct PartitionData;
struct SimulationProperties;
struct EXTERNAL_InterfaceList;

// /**
// ENUM        :: ExternalInterfaceType
// DESCRIPTION :: Enumeration of different types of external interfaces
// **/
enum ExternalInterfaceType {
    EXTERNAL_TYPE_NONE          = 0,
    EXTERNAL_HLA                = 1,
    EXTERNAL_DIS                = 2,
    EXTERNAL_IPNE,
    EXTERNAL_INTERNET_GATEWAY,
    EXTERNAL_SOCKET,
    EXTERNAL_INTERFACETUTORIAL,
    EXTERNAL_QSH,
    EXTERNAL_DXML,
    EXTERNAL_UPA,
    EXTERNAL_HITL,
    EXTERNAL_PAS,
    EXTERNAL_RECORD_REPLAY,
    EXTERNAL_RT_INDICATOR,
    EXTERNAL_MA,
    EXTERNAL_MIMULTICAST_ADAPTER,
    EXTERNAL_JSR,
    EXTERNAL_QUALNET_GUI,
    EXTERNAL_TESTTHREAD,
    EXTERNAL_TYPE_COUNT,
    EXTERNAL_AGI,
    EXTERNAL_TYPE_MAX           = 128
};

//---------------------------------------------------------------------------
// Constants
//---------------------------------------------------------------------------

// /**
// TYPEDEF     :: EXTERNAL_PerformanceParameters
// DESCRIPTION :: Parameters that govern how an external interface will
//                interact with QualNet.  Currently there are no additional
//                parameters but this may change in the future.
//
//                EXTERNAL_NONE - No additional parameters
//                EXTERNAL_CPU_HOG - If external interface should try to
//                                   use up all CPU
//                EXTERNAL_THREADED - Receive and Forward functions will be
//                    launched as separate threads.  Under this architecture
//                    the Receive and Forward threads should block.
//                EXTERNAL_THREADED_SINGLE_THREAD - Receive and Forward
//                    functions will be called from a single thread
//                    separate from the main QualNet thread.  In this
//                    architecture the Receive and Forward functions should
//                    not block.
//                EXTERNAL_THREADED_MULTIPLE_THREADS - Receive and Forward
//                    functions will be launched as separate threads.
//                    The external interface can launch several threads that
//                    could potentially send messages at the same time.  If
//                    only one thread will send messages then
//                    EXTERNAL_THREADED or EXTERNAL_THREADED_SINGLE_THREAD
//                    should be used instead.
//
// **/
typedef int EXTERNAL_PerformanceParameters;
#define EXTERNAL_NONE             0x00
#define EXTERNAL_CPU_HOG          0x01
#define EXTERNAL_THREADED         0x02
#define EXTERNAL_THREADED_SINGLE_THREAD    0x04
#define EXTERNAL_THREADED_MULTIPLE_THREADS 0x08
#define EXTERNAL_NO_WARMUP_RECEIVE         0x16

// /**
// CONSTANT    :: EXTERNAL_MAX_TIME
// DESCRIPTION :: The maximum possible time
// **/
#define EXTERNAL_MAX_TIME CLOCKTYPE_MAX

// /**
// CONSTANT    :: EXTERNAL_NUM_CPU_TIMING_INTERVAL_GUESSES : 4
// DESCRIPTION :: The number of guesses to make for the cpu timing interval
// **/
#define EXTERNAL_NUM_CPU_TIMING_INTERVAL_GUESSES 4

// /**
// CONSTANT     :: EXTERNAL_MAPPING_TABLE_SIZE : 31
// DESCRIPTION :: The size of an interface's mapping hash table
// **/
#define EXTERNAL_MAPPING_TABLE_SIZE 31

// /**
// CONSTANT    :: EXTERNAL_NUM_FUNCTIONS : 8
// DESCRIPTION :: The number of functions an interface may implement
// **/
#define EXTERNAL_NUM_FUNCTIONS 8

// /**
// CONSTANT    :: EXTERNAL_RT_INDICATOR_INTERVAL : 0.1 second
// DESCRIPTION :: The report interval of the realtime indication 
// **/
#define EXTERNAL_RT_INDICATOR_INTERVAL 100*MILLI_SECOND

// /**
// CONSTANT    :: EXTERNAL_RT_INDICATOR_THRESHOLD: 1 second now
// DESCRIPTION :: red flag if the difference between 
//              realtime and the sim time is bigger than thread 
// **/
#define EXTERNAL_RT_INDICATOR_THRESHOLD 1 

// /**
// ENUMERATION :: EXTERNAL_WarmupPhase
// DESCRIPTION :: The different warmup phases to stabilize the simulated
//                network before external traffic starts.
// **/
enum EXTERNAL_WarmupPhase
{
    EXTERNAL_NoWarmup,
    EXTERNAL_WaitingToBeginWarmup,
    EXTERNAL_PrintedWaitingToBeginWarmup,
    EXTERNAL_InWarmup,
    EXTERNAL_OutOfWarmup
};

// /**
// ENUMERATION :: EXTERNAL_FunctionType
// DESCRIPTION :: The possible functions that will be called by QualNet.
//                The constant EXTERNAL_NUM_FUNCTIONS must reflect how many
//                functions there are.
// **/
enum EXTERNAL_FunctionType
{
    EXTERNAL_INITIALIZE = 0,
    EXTERNAL_INITIALIZE_NODES,
    EXTERNAL_TIME,
    EXTERNAL_SIMULATION_HORIZON,
    EXTERNAL_PACKET_DROPPED,
    EXTERNAL_RECEIVE,
    EXTERNAL_FORWARD,
    EXTERNAL_FINALIZE,
};

// /**
// STRUCT      :: EXTERNAL_ThreadedMessage
// DESCRIPTION :: A struct containing data needed to send a message from an
//                external thread to the main thread.
// **/
struct EXTERNAL_ThreadedMessage
{
    Node* node;
    Message* msg;
    clocktype timestamp;
};

// /**
// STRUCT      :: EXTERNAL_ThreadedForwarded
// DESCRIPTION :: A struct containing data needed to send a forwarded packet
//                from the main thread to an external forward function
// **/
struct EXTERNAL_ThreadedForward
{
    Node* node;
    void* forwardData;
    int forwardSize;
};

// /**
// CLASS       :: EXTERNAL_LockFreeQueueNode
// DESCRIPTION :: A class containing one node for the lock free queues.
//                Used for blocking and non-blocking queues.
// **/
template<typename T>
class EXTERNAL_LockFreeQueueNode
{
    public:
        T val;
        EXTERNAL_LockFreeQueueNode* next;

        EXTERNAL_LockFreeQueueNode() : next(NULL) {}
        EXTERNAL_LockFreeQueueNode(const T& newVal) : val(newVal), next(NULL) {}
};

template<typename T>
class EXTERNAL_Queue
{
    public:
        virtual bool empty() const = 0;
        virtual UInt64 size() const = 0;
        virtual void push(const T& val) = 0;
        virtual void front(T& val) = 0;
        virtual void pop(T& val) = 0;
        virtual void signal() {}
};

// /**
// CLASS       :: EXTERNAL_LockFreeQueue
// DESCRIPTION :: A class implementing a lock free queue.  This class is
//                suitable for only ONE writer thread and ONE reader thread.
// **/
template<typename T>
class EXTERNAL_LockFreeQueue : public EXTERNAL_Queue<T>
{
    private:
        EXTERNAL_LockFreeQueueNode<T>* head;
        EXTERNAL_LockFreeQueueNode<T>* tail;
        UInt64 numPush;
        UInt64 numPop;

    public:
        EXTERNAL_LockFreeQueue()
        {
            head = new EXTERNAL_LockFreeQueueNode<T>();
            tail = head;
            numPush = 0;
            numPop = 0;
        }

        bool empty() const
        {
            return head == tail;
        }

        UInt64 size() const
        {
            return numPush - numPop;
        }

        void push(const T& val)
        {
            // Change old tail value to reflect new value.  Then add a new
            // empty tail to the end.  This prevents the head thread from
            // reading this value until the new tail is set

            tail->val = val;

            EXTERNAL_LockFreeQueueNode<T>* newTail;
            newTail = new EXTERNAL_LockFreeQueueNode<T>;
            tail->next = newTail;
            tail = newTail;
            numPush++;
        }

        void front(T& val)
        {
            ERROR_Assert(
                !empty(),
                "Queue must not be empty when front is called");

            // Returns default (or null) if empty
            val = head->val;
        }

        void pop(T& val)
        {
            ERROR_Assert(
                !empty(),
                "Queue must not be empty when pop is called");

            // Get first value
            val = head->val;

            // Remove old head
            EXTERNAL_LockFreeQueueNode<T>* oldHead = head;
            head = oldHead->next;
            delete oldHead;
            numPop++;
        }
};

// /**
// CLASS       :: EXTERNAL_MultipleWritersQueue
// DESCRIPTION :: A class implementing a lock free queue.  This class is
//                suitable for multiple writers thread and ONE reader thread.
//                This class is similar to the lock free version in that the
//                reader is lock free.  The difference is that the writes
//                are protected by a mutex making it safe for multiple
//                writers.
// **/
template<typename T>
class EXTERNAL_MultipleWritersQueue : public EXTERNAL_Queue<T>
{
    private:
        EXTERNAL_LockFreeQueueNode<T>* head;
        EXTERNAL_LockFreeQueueNode<T>* tail;
        UInt64 numPush;
        UInt64 numPop;

        pthread_mutex_t condMutex;
        pthread_cond_t cond;

    public:
        EXTERNAL_MultipleWritersQueue()
        {
            head = new EXTERNAL_LockFreeQueueNode<T>();
            tail = head;
            numPush = 0;
            numPop = 0;
            pthread_mutex_init(&condMutex, NULL);
        }

        bool empty() const
        {
            return head == tail;
        }

        UInt64 size() const
        {
            return numPush - numPop;
        }

        void push(const T& val)
        {
            // Change old tail value to reflect new value.  Then add a new
            // empty tail to the end.  This prevents the head thread from
            // reading this value until the new tail is set

            EXTERNAL_LockFreeQueueNode<T>* newTail;
            newTail = new EXTERNAL_LockFreeQueueNode<T>;

            // Add val to end and signal non-empty
            pthread_mutex_lock(&condMutex);
            tail->val = val;
            tail->next = newTail;
            tail = newTail;
            numPush++;
            pthread_mutex_unlock(&condMutex);
        }

        void front(T& val)
        {
            ERROR_Assert(
                !empty(),
                "Queue must not be empty when pop is called");

            assert(!empty());
            val = head->val;
        }

        void pop(T& val)
        {
            ERROR_Assert(
                !empty(),
                "Queue must not be empty when pop is called");

            assert(!empty());
            val = head->val;

            // Remove old head
            EXTERNAL_LockFreeQueueNode<T>* oldHead = head;
            head = oldHead->next;
            delete oldHead;
            numPop++;
        }
};

// /**
// CLASS       :: EXTERNAL_SafeQueue
// DESCRIPTION :: A class implementing a lock free queue.  This class is
//                suitable for many writer threads and many reader threads.
//                This class is similar to the lock free version in that the
//                reader is lock free.  The difference is that the writes
//                are protected by a mutex making it safe for multiple
//                writers.
// **/
template<typename T>
class EXTERNAL_SafeQueue : public EXTERNAL_Queue<T>
{
    private:
        EXTERNAL_LockFreeQueueNode<T>* head;
        EXTERNAL_LockFreeQueueNode<T>* tail;
        UInt64 numPush;
        UInt64 numPop;

        pthread_mutex_t mutex;

    public:
        EXTERNAL_SafeQueue()
        {
            head = new EXTERNAL_LockFreeQueueNode<T>();
            tail = head;
            numPush = 0;
            numPop = 0;
            pthread_mutex_init(&mutex, NULL);
        }

        bool empty() const
        {
            return head == tail;
        }

        UInt64 size() const
        {
            return numPush - numPop;
        }

        void push(const T& val)
        {
            pthread_mutex_lock(&mutex);
            // Change old tail value to reflect new value.  Then add a new
            // empty tail to the end.  This prevents the head thread from
            // reading this value until the new tail is set

            EXTERNAL_LockFreeQueueNode<T>* newTail;
            newTail = new EXTERNAL_LockFreeQueueNode<T>;

            // Add val to end
            tail->val = val;
            tail->next = newTail;
            tail = newTail;
            numPush++;
            pthread_mutex_unlock(&mutex);
        }

        void front(T& val)
        {
            pthread_mutex_lock(&mutex);
            ERROR_Assert(
                !empty(),
                "Queue must not be empty when pop is called");
            
            assert(!empty());
            val = head->val;
            pthread_mutex_unlock(&mutex);
        }

        void pop(T& val)
        {
            pthread_mutex_lock(&mutex);
            ERROR_Assert(
                !empty(),
                "Queue must not be empty when pop is called");

            assert(!empty());
            val = head->val;

            // Remove old head
            EXTERNAL_LockFreeQueueNode<T>* oldHead = head;
            head = oldHead->next;
            delete oldHead;
            numPop++;
            pthread_mutex_unlock(&mutex);
        }
};

// /**
// CLASS       :: EXTERNAL_BlockingQueue
// DESCRIPTION :: A class implementing a lock free queue.  This class is
//                suitable for multiple writers thread and ONE reader thread.
//                This class is similar to the non-blocking version however
//                the front() and pop() functions block if the queue is
//                empty.  front() and pop() may exit prematurely if the
//                queue is signalled to stop such as when an interface has
//                stopped running.  Therefore front() and pop() may exit
//                in the empty() state if this situation occurs.
// **/
template<typename T>
class EXTERNAL_BlockingQueue : public EXTERNAL_Queue<T>
{
    private:
        EXTERNAL_LockFreeQueueNode<T>* head;
        EXTERNAL_LockFreeQueueNode<T>* tail;
        UInt64 numPush;
        UInt64 numPop;
        
        bool wasSignalled;
        pthread_mutex_t condMutex;
        pthread_cond_t cond;

    public:
        EXTERNAL_BlockingQueue()
        {
            head = new EXTERNAL_LockFreeQueueNode<T>();
            tail = head;
            numPush = 0;
            numPop = 0;
            wasSignalled = false;
            pthread_mutex_init(&condMutex, NULL);
            pthread_cond_init(&cond, NULL);
        }

        bool empty() const
        {
            return head == tail;
        }

        UInt64 size() const
        {
            return numPush - numPop;
        }

        void signal()
        {
            wasSignalled = true;
            pthread_mutex_lock(&condMutex);
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&condMutex);
        }

        void push(const T& val)
        {
            // Change old tail value to reflect new value.  Then add a new
            // empty tail to the end.  This prevents the head thread from
            // reading this value until the new tail is set

            EXTERNAL_LockFreeQueueNode<T>* newTail;
            newTail = new EXTERNAL_LockFreeQueueNode<T>;

            // Add val to end and signal non-empty
            pthread_mutex_lock(&condMutex);
            tail->val = val;
            tail->next = newTail;
            tail = newTail;
            numPush++;
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&condMutex);
        }

        // The front() and pop() functions may exit with an empty queue if
        // the signal() function is called.  The signal() function will only
        // be called in the External Interface API if it is explicitly
        // called by the interface developer.  This would be handy when
        // stopping QualNet when this queue is blocked.  Therefore if the
        // signal() function could be called the interface developer should
        // consider that the resulting value could be garbage.
        void front(T& val)
        {
            // If empty wait for signal
            pthread_mutex_lock(&condMutex);
            if (empty())
            {
                pthread_cond_wait(&cond, &condMutex);
            }
            pthread_mutex_unlock(&condMutex);

            if (wasSignalled)
            {
                return;
            }

            assert(!empty());
            val = head->val;
        }

        // The front() and pop() functions may exit with an empty queue if
        // the signal() function is called.  The signal() function will only
        // be called in the External Interface API if it is explicitly
        // called by the interface developer.  This would be handy when
        // stopping QualNet when this queue is blocked.  Therefore if the
        // signal() function could be called the interface developer should
        // consider that the resulting value could be garbage.
        void pop(T& val)
        {
            // If empty wait for signal
            pthread_mutex_lock(&condMutex);
            if (empty())
            {
                pthread_cond_wait(&cond, &condMutex);
            }
            pthread_mutex_unlock(&condMutex);

            if (wasSignalled)
            {
                return;
            }

            assert(!empty());
            val = head->val;

            // Remove old head
            EXTERNAL_LockFreeQueueNode<T>* oldHead = head;
            head = oldHead->next;
            delete oldHead;
            numPop++;
        }
};

//---------------------------------------------------------------------------
// Function Pointer Prototypes
//---------------------------------------------------------------------------

// /**
// TYPEDEF     :: EXTERNAL_Function
// DESCRIPTION :: A generic function that will be called by QualNet
// **/
typedef void (*EXTERNAL_Function)();

// forward declaration
struct EXTERNAL_Interface;

// /**
// TYPEDEF     :: EXTERNAL_*Function
// DESCRIPTION :: The signatures of all functions that QualNet may call
// **/
typedef void (*EXTERNAL_InitializeFunction)(
    EXTERNAL_Interface *iface,
    NodeInput *nodeInput);
typedef void (*EXTERNAL_InitializeNodesFunction)(
    EXTERNAL_Interface *iface,
    NodeInput *nodeInput);
typedef clocktype (*EXTERNAL_TimeFunction)(EXTERNAL_Interface *iface);
typedef void (*EXTERNAL_SimulationHorizonFunction)(EXTERNAL_Interface *iface);
typedef void (*EXTERNAL_PacketDroppedFunction)(EXTERNAL_Interface *iface);
typedef void (*EXTERNAL_ReceiveFunction)(EXTERNAL_Interface *iface);
typedef void (*EXTERNAL_ForwardFunction)(EXTERNAL_Interface *iface, Node* node, void *forwardData, int forwardSize);
typedef void (*EXTERNAL_FinalizeFunction)(EXTERNAL_Interface *iface);

//---------------------------------------------------------------------------
// Data Structures
//---------------------------------------------------------------------------

// /**
// STRUCT      :: EXTERNAL_Mapping
// DESCRIPTION :: A linked list node containing one mapping.  The key may be
//                of any size, specified by keySize.  The value the key maps
//                to is a pointer to some piece of data.  It is assumed that
//                whoever created the mapping will know what to do with the
//                pointer.  The user will not use this structure directly.
// **/
struct EXTERNAL_Mapping
{
    // The key, and it size
    char *key;
    int keySize;

    // The value the key maps to, and it size
    char *val;
    int valSize;

    // The next mapping in the linked list
    EXTERNAL_Mapping *next;
};

typedef Coordinates Velocity;

// /**
// STRUCT      :: EXTERNAL_MobilityEvent
// DESCRIPTION :: A linked list of mobility events
// **/
struct EXTERNAL_MobilityEvent
{
    Coordinates coordinates;
    Orientation orientation;
    double speed;
    Velocity velocity;
    clocktype truePositionTime;
    BOOL isTruePosition;
    BOOL velocityOnly;
    BOOL delaySpeedCalculation;
};

// /**
// STRUCT      :: EXTERNAL_MobilityEventBuffer
// DESCRIPTION :: A buffer containing all mobility events yet to be added to
//                the simulation.
// **/
struct EXTERNAL_MobilityBuffer
{
    // The list of events to be added, sorted by time in ascending order
    // NOTE: This is currently a linked list.  A more efficient data
    // structure should be used.
    EXTERNAL_MobilityEvent *eventList;

    // The last event in the list
    EXTERNAL_MobilityEvent *lastEvent;

    // Unused events.  Avoids un-necessary memory allocations.
    EXTERNAL_MobilityEvent *unusedEventList;

    // The last time an external mobility event was scheduled
    clocktype lastScheduledExternalMobilityEventTime;
};

// /**
// STRUCT      :: EXTERNAL_InterfaceList
// DESCRIPTION :: A list containing all of the registered external entities
// **/
struct EXTERNAL_InterfaceList
{
    // the interface list
    EXTERNAL_Interface *interfaces;

    // If external interfaces are active
    BOOL isActive;

    // The number of interfaces that are registered
    int numInterfaces;

    // the partition the list is stored on
    PartitionData *partition;

    // TRUE if external interface API should try to use up all CPU
    BOOL cpuHog;

    // What time real time was initialized
    clocktype initializeTime;

    // The warmup time to use for this run.  -1 if no warmup time.
    clocktype warmupTime;
    
    BOOL warmupDrop;
    
    EXTERNAL_WarmupPhase warmupPhase;

    // Number of warmup interfaces
    // Calling EXTERNAL_SetWarmupTime increments this value
    // Calling EXTERNAL_BeginWarmup decrements this value
    // When this value reaches 0 warmup will begin
    int numWarmupInterfaces;

    // Variables for handling pausing (only in real time)
    BOOL paused;
    clocktype pauseSimTime;
    clocktype pauseRealTime;

    // Variables for handling stop
    BOOL isStopping;
    clocktype stopTime;

    // Debug variables
    BOOL printPacketLog; // TRUE if EXTERNAL-DEBUG-LEVEL is 3
    BOOL printStatistics; // TRUE If EXTERNAL-DEBUG-LEVEL is 2 or higher
    BOOL debug; // TRUE if EXTERNAL-DEBUG-LEVEL is 1 or higher

    // The mobility buffer
    EXTERNAL_MobilityBuffer mobilityBuffer;

    // Message queues for incoming messages from threaded interfaces
    std::vector<EXTERNAL_Queue<EXTERNAL_ThreadedMessage>*> messageQueues;
};

// /**
// STRUCT      :: EXTERNAL_Interface
// DESCRIPTION :: The information pertaining to one external interface
// **/
struct EXTERNAL_Interface
{
    // The partition the interface is running on
    PartitionData *partition;

    // The interface name
    char name[MAX_STRING_LENGTH];
    ExternalInterfaceType type;

    // A unique id assigned to this interface
    int interfaceId;

    // Mappings the interface might use.  This is implemented as a hash
    // table using chaining.  That is, each element in the table is a linked
    // list containing all keys that have a similar hash value.
    EXTERNAL_Mapping *table[EXTERNAL_MAPPING_TABLE_SIZE];

    // Configurable parameters
    EXTERNAL_PerformanceParameters params;
    EXTERNAL_InitializeFunction initializeFunction;
    EXTERNAL_InitializeNodesFunction initializeNodesFunction;
    EXTERNAL_TimeFunction timeFunction;
    EXTERNAL_SimulationHorizonFunction simulationHorizonFunction;
    EXTERNAL_PacketDroppedFunction packetDroppedFunction;
    EXTERNAL_ReceiveFunction receiveFunction;
    EXTERNAL_ForwardFunction forwardFunction;
    EXTERNAL_FinalizeFunction finalizeFunction;

    // Timing variables
    clocktype initializeTime;
    clocktype lookahead;
    clocktype horizon;
    clocktype lastReceiveCall;
    clocktype receiveCallDelay;

    // Timing variables for CPU Time
    BOOL cpuTimingStarted;
    clock_t cpuTimeStart;
    clock_t lastCpuTime;
    clocktype cpuTimingInterval;

    // Whether to not call the receive function during warmup time
    BOOL warmupNoReceive;

    // Interface specific data
    void *data;

    // Thread variables
    BOOL threaded;
    BOOL running;
    pthread_t receiveThread;
    pthread_t forwardThread;
    pthread_t receiveForwardThread;
    EXTERNAL_Queue<EXTERNAL_ThreadedMessage>* receiveMessages;
    EXTERNAL_Queue<EXTERNAL_ThreadedForward>* forwards;

    // The interface list the interface is contained in
    EXTERNAL_InterfaceList *interfaceList;

    // The next external interface
    EXTERNAL_Interface *next;

    // Debug functions for this interface
    BOOL printPacketLog() { return this->interfaceList->printPacketLog; }
    BOOL printStatistics() { return this->interfaceList->printStatistics; }
    BOOL debug() { return this->interfaceList->debug; }

    // Message queues for incoming messages from threaded interfaces
    std::vector<EXTERNAL_Queue<EXTERNAL_ThreadedMessage>*> messageQueues;
};

//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

// /**
// API       :: EXTERNAL_RegisterExternalInterface
// PURPOSE   :: This function will register a new external interface with
//              QualNet and create the necessary data structures. This
//              function must be called before any other function that
//              requires an EXTERNAL_Interface* argument.
// PARAMETERS ::
// + list : EXTERNAL_InterfaceList* : The list of external interfaces
// + name : char* : The name of the external interface.
// + params : EXTERNAL_PerformanceParameters : The performance parameters
// RETURN    :: EXTERNAL_Interface * : A pointer to the newly registered
//              external interface
// **/
EXTERNAL_Interface* EXTERNAL_RegisterExternalInterface(
    EXTERNAL_InterfaceList *list,
    const char *name,
    EXTERNAL_PerformanceParameters params);

// /**
// API       :: EXTERNAL_RegisterExternalInterface
// PURPOSE   :: This function is an overloaded variation.
//              for registering a new external interface with
//              QualNet
// PARAMETERS ::
// + list : EXTERNAL_InterfaceList* : The list of external interfaces
// + name : char* : The name of the external interface.
// + params : EXTERNAL_PerformanceParameters : The performance parameters
// + type : ExternalInterfaceType : PartitionData's interfaceTable will be
//                                  filled given this type with the pointer
//                                  to the registered external interface.
// RETURN    :: EXTERNAL_Interface * : A pointer to the newly registered
//              external interface
// **/
EXTERNAL_Interface* EXTERNAL_RegisterExternalInterface(
    EXTERNAL_InterfaceList *list,
    const char *name,
    EXTERNAL_PerformanceParameters params,
    ExternalInterfaceType type);

// /**
// API       :: EXTERNAL_RegisterFunction
// PURPOSE   :: Register a new function for an interface.
// PARAMETERS ::
// + iface : EXTERNAL_Interface* : The external interface
// + type : EXTERNAL_FunctionType : the type of function
// + function : EXTERNAL : Function pointer to be called
// RETURN    :: void :
// **/
void EXTERNAL_RegisterFunction(
    EXTERNAL_Interface *iface,
    EXTERNAL_FunctionType type,
    EXTERNAL_Function function);

// /**
// API       :: EXTERNAL_SetTimeManagementRealTime
// PURPOSE   :: Turns time management on and specifies the lookahead value.
//              The lookahead value may be changed later by calling
//              EXTERNAL_ChangeRealTimeLookahead().
// PARAMETERS ::
// + iface : EXTERNAL_Interface* : The external interface
// + lookahead : clocktype : How far into the future the simulation is
//              allowed to run.
// RETURN    :: void :
// **/
void EXTERNAL_SetTimeManagementRealTime(
    EXTERNAL_Interface *iface,
    clocktype lookahead);

// /**
// API       :: EXTERNAL_ChangeRealTimeLookahead
// PURPOSE   :: Modifies the lookahead value.  Must be called after
//              EXTERNAL_SetTimeManagementRealTime().  May be called during
//              the simulation.
// PARAMETERS ::
// + iface : EXTERNAL_Interface* : The external interface
// + lookahead : clocktype : The new lookahead value
// RETURN    :: void :
// **/
void EXTERNAL_ChangeRealTimeLookahead(
    EXTERNAL_Interface *iface,
    clocktype lookahead);

// /**
// API       :: EXTERNAL_InitializeWarmupParams
// PURPOSE   :: 
// PARAMETERS ::
// + iface  : EXTERNAL_Interface* : The external interface
// + nodeInput : NodeInput* : The configuration file.
// RETURN   :: void :
// **/
void EXTERNAL_InitializeWarmupParams(
    EXTERNAL_InterfaceList* list, 
    NodeInput* nodeInput);


// /**
// API       :: EXTERNAL_RealtimeIndicator
// PURPOSE   :: for realtime indicator initialization
// PARAMETERS ::
// + iface  : EXTERNAL_Interface* : The external interface
// + nodeInput : NodeInput* : The configuration file.
// RETURN   :: void :
// **/
void EXTERNAL_RealtimeIndicator(
    EXTERNAL_InterfaceList* list,
    NodeInput* nodeInput);


// /**
// API       :: EXTERNAL_SetWarmupTime
// PURPOSE   :: Sets this interface's warmup time.  The actual warmup time
//              used is the maximum of all interface's.  The default is no
//              warmup time (warmup == -1).  This function must be called
//              before or during the initialize nodes step.  It will have
//              no effect during the simulation.
// PARAMETERS ::
// + iface  : EXTERNAL_Interface* : The external interface
// + warmup : clocktype : The warmup time for this interface
// RETURN   :: void :
// **/
void EXTERNAL_SetWarmupTime(
    EXTERNAL_Interface *iface,
    clocktype warmup);

// /**
// API       :: EXTERNAL_BeginWarmup
// PURPOSE   :: Each interface that calls EXTERNAL_SetWarmupTime must call
//              EXTERNAL_BeginWarmup when it is ready to enter warmup time.
// PARAMETERS ::
// + iface  : EXTERNAL_Interface* : The external interface
// RETURN   :: void :
// **/
void EXTERNAL_BeginWarmup(EXTERNAL_Interface *iface);

// /**
// API       :: EXTERNAL_QueryWarmupTime
// PURPOSE   :: Get the warmup time for the entire simulation.  Interfaces
//              should use this function to test when warmup time is over.
// PARAMETERS ::
// + iface  : EXTERNAL_Interface* : The external interface
// RETURN   :: clocktype : The inclusive end of warmup time.  -1 if no
//             warmup time.
// **/
clocktype EXTERNAL_QueryWarmupTime(EXTERNAL_Interface *iface);

// /**
// API       :: EXTERNAL_IsInWarmup
// PURPOSE   :: Check if QualNet is in the warmup phase
// PARAMETERS ::
// + iface  : EXTERNAL_Interface* : The external interface
// RETURN   :: BOOL : TRUE if in warmup, FALSE if not
// This is now a wrapper function ONLY. It passes a pointer to partition data. 
// We overload this function in order to check if simulator is in warm-up phase
// even when we do not have access to External interface
// **/
BOOL EXTERNAL_IsInWarmup(EXTERNAL_Interface *iface);


// /**
// API       :: EXTERNAL_IsInWarmup
// PURPOSE   :: Check if QualNet is in the warmup phase
// PARAMETERS ::
// + partitionData  : PartitionData* : pointer to partition's data structure
// RETURN   :: BOOL : TRUE if in warmup, FALSE if not
// **/
BOOL EXTERNAL_IsInWarmup(PartitionData* partitionData);


BOOL EXTERNAL_WarmupTimeEnabled (EXTERNAL_Interface *iface);


// /**
// API       :: EXTERNAL_Pause
// PURPOSE   :: Pause every interface.  Only usable when running in
//              real-time.
// PARAMETERS ::
// + iface : EXTERNAL_Interface* : The external interface
// RETURN    :: void :
// **/
void EXTERNAL_Pause(EXTERNAL_Interface *iface);

// /**
// API       :: EXTERNAL_Resume
// PURPOSE   :: Resume every interface.  Only usable when running in
//              real-time, and after calling pause.
// PARAMETERS ::
// + iface : EXTERNAL_Interface* : The external interface
// RETURN    :: void :
// **/
void EXTERNAL_Resume(EXTERNAL_Interface *iface);

void EXTERNAL_SetCpuHog(EXTERNAL_Interface *iface);

// /**
// API       :: EXTERNAL_QueryExternalTime
// PURPOSE   :: This function will return the External Time of an external
//              interface
// PARAMETERS ::
// + iface : EXTERNAL_Interface* : The external interface
// RETURN    :: clocktype : The External Time.  Returns EXTERNAL_MAX_TIME if
//              no time function is defined.
// **/
clocktype EXTERNAL_QueryExternalTime(EXTERNAL_Interface *iface);

// /**
// API       :: EXTERNAL_QuerySimulationTime
// PURPOSE   :: This function will return the Simulation Time
// PARAMETERS ::
// + iface : EXTERNAL_Interface* : The external interface
// RETURN    :: clocktype : The Simulation Time
// **/
clocktype EXTERNAL_QuerySimulationTime(EXTERNAL_Interface *iface);

// /**
// API       :: EXTERNAL_Sleep
// PURPOSE   :: This function will sleep for a minimum amount of time as
//              indicated by the amount parameter.  Depending on which
//              platform it is called on the amount of time spent sleeping
//              could be greater.
// PARAMETERS ::
// + amount : clocktype : The amount of time to sleep
// RETURN    :: void :
// **/
void EXTERNAL_Sleep(clocktype amount);

// /**
// API       :: EXTERNAL_SetReceiveDelay
// PURPOSE   :: This function will set the minimum delay between two
//              consecutive calls to the receive function.  The time used is
//              the simulation time.
// PARAMETERS ::
// + iface : EXTERNAL_Interface* : The external interface
// + delay : clocktype : The minimum delay
// RETURN    :: void :
// **/
void EXTERNAL_SetReceiveDelay(
    EXTERNAL_Interface *iface,
    clocktype delay);


// /**
// ENUMERATION  :: EXTERNAL_ForwardData_ReceiverOpt
// DESCRIPTION  :: This enum is a control flow to allow multicast fowarding
//              data to be correctly forwarded across partitions without loss
//              of the receiving node.  EXTERNAL_ForwardData has this as a default
//              parameter set to 'ON', it is expected that users will not need or
//              use this enum.
enum EXTERNAL_ForwardData_ReceiverOpt
{
    EXTERNAL_ForwardDataAssignNodeID_On = 1,
    EXTERNAL_ForwardDataAssignNodeID_Off = 0
};


// /**
// API       :: EXTERNAL_SendMessage
// PURPOSE   :: This function will send a message from the external
//     interface.  This function is thread-safe.
// PARAMETERS ::
// + iface : EXTERNAL_Interface* : The external interface
// + node : Node* : Node sending the message
// + msg : Message* : The message to send
// + timestamp : clocktype : The timestamp for this message.  Since this message is
//     sent from a separate thread it will not be scheduled immediately.
//     This extra scheduling delay (until the main thread is able to
//     schedule the message) is accounted for.  If the scheduling delay is
//     greater than the input delay then the message is sent immediately.
// RETURN    :: void :
// **/
void EXTERNAL_SendMessage(
    EXTERNAL_Interface *iface,
    Node* node,
    Message* msg,
    clocktype timestamp);

// /**
// API       :: EXTERNAL_ForwardData
// PURPOSE   :: Send data back to the external source with no time stamp.
//              The user defined Forward function will receive this message
//              and process it.  This will handle threading issues if
//              necessary.
// PARAMETERS ::
// + iface : EXTERNAL_Interface* : The external interface
// + node : Node* : The node that is forwarding the data
// + forwardData : void* : The data to forward
// + forwardSize : int : The size of the data to forward
// + FwdReceiverOpt : EXTERNAL_ForwardData_ReceiverOpt : Whether to store the 
//                    receiving node for forwarded data, default to ON
// RETURN    :: void :
// **/
void EXTERNAL_ForwardData(
    EXTERNAL_Interface *iface,
    Node *node,
    void *forwardData,
    int forwardSize,
    EXTERNAL_ForwardData_ReceiverOpt FwdReceiverOpt = EXTERNAL_ForwardDataAssignNodeID_On);

// /**
// API       :: EXTERNAL_RemoteForwardData
// PURPOSE   :: Send data back to the external source with no time stamp.
//              This function is similar to EXTERNAL_ForwardData, except
//              that this function can forward the message to and external
//              interface on a different partition.
// PARAMETERS ::
// + iface : EXTERNAL_Interface* : The external interface
// + node : Node* : The node that is forwarding the data
// + forwardData : void* : The data to forward
// + forwardSize : int : The size of the data to forward
// + partitionId : int : The partition Id to forward the message to
// RETURN    :: void :
// **/
void EXTERNAL_RemoteForwardData(
    EXTERNAL_Interface *iface,
    Node* node,
    void *forwardData,
    int forwardSize,
    int partitionId,
    clocktype delay);

// /**
// API       :: EXTERNAL_ForwardDataTimeStamped
// PURPOSE   :: Send data in the form of a message back to the external
//              source with a time stamp.  The user defined Forward function
//              will receive this message and process it.  This will handle
//              threading issues if necessary.
// PARAMETERS ::
// + iface : EXTERNAL_Interface* : The external interface
// + node : Node* : The node that is forwarding the data
// + message : Message* : The message
// + timestamp : clocktype : The time stamp.  This value is in external
//              time, not simulation time.  The time stamp may be
//              modified if it is an invalid time.
// NOTES     :: EXTERNAL_BUFFERED_OUTPUT must be enabled.
// RETURN    :: void :
// **/
void EXTERNAL_ForwardDataTimeStamped(
    EXTERNAL_Interface *iface,
    Node *node,
    Message *message,
    clocktype timeStamp);

// /**
// API       :: EXTERNAL_UserFunctionRegistration
// PURPOSE   :: This function will give a convenient place for users to
//              add their function registration code.  This is the only
//              part of the External Interface API code that the user
//              is expected to modify.
// PARAMETERS ::
// + list : EXTERNAL_InterfaceList * : The list of external interfaces
// + nodeInput : NodeInput* : The configuration file
// RETURN    :: void :
// **/
void EXTERNAL_UserFunctionRegistration(
    EXTERNAL_InterfaceList *list,
    NodeInput *nodeInput);

// /**
// API       :: EXTERNAL_InitializeInterface
// PURPOSE   :: This function will initialize an external interface
// PARAMETERS ::
// + iface : EXTERNAL_Interface* : The external interface
// RETURN    :: void :
// **/
void EXTERNAL_InitializeInterface(EXTERNAL_Interface *iface);

// /**
// API       :: EXTERNAL_FinalizeExternalInterface
// PURPOSE   :: This function will free an external interface, as well
//              as call the finalize function registered by
//              EXTERNAL_RegisterFinalizeFunction()
// PARAMETERS ::
// + iface : EXTERNAL_Interface* : The external interface
// RETURN    :: void :
// **/
void EXTERNAL_FinalizeExternalInterface(EXTERNAL_Interface *iface);

// /**
// API       :: EXTERNAL_InitializeInterfaceList
// PURPOSE   :: This function will initialize an external interface list
// PARAMETERS ::
// + list : EXTERNAL_InterfaceList* : The external interface list
// + partition : PartitionData* : The partition it will run on
// RETURN    :: void :
// **/
void EXTERNAL_InitializeInterfaceList(
    EXTERNAL_InterfaceList *list,
    PartitionData *partition);

// /**
// API       :: EXTERNAL_Bootstrap
// PURPOSE   :: This function will be called early in the simulation
//              initialization process (after MPI_Init(), but before
//              partitions are created, and before
//              EXTERNAL_InitializeInterfaceList (). In a shared parallel
//              simulation the threads for partitions won't be created yet.
// PARAMETERS ::
//   + argc : int : The command line argument count
//   + argv [] : char* : The command line arguments
// RETURN    :: void :
// **/
void EXTERNAL_Bootstrap(int argc, 
                        char * argv [],
                        NodeInput* nodeInput,
                        PartitionData * partitionData);

// /**
// API       :: EXTERNAL_PreBootstrap
// PURPOSE   :: This function will be called early in the simulation
//              initialization process (after MPI_Init(), but before
//              partitions are created, and before
//              EXTERNAL_InitializeInterfaceList (). In a shared parallel
//              simulation the threads for partitions won't be created yet.
//              This function handles the mini-configuration file conversion, and
//              make sures that if simProps needs to change it is changed and then
//              a broadcast message is sent to other partitions.
// PARAMETERS ::
//   + argc : int : The command line argument count
//   + argv [] : char* : The command line arguments
// RETURN    :: void :
// **/
void EXTERNAL_PreBootstrap(int argc, char * argv [],
    SimulationProperties * simProps,
    PartitionData * partitionData);
// /**
// API       :: EXTERNAL_FinalizeInterfaceList
// PURPOSE   :: This function will finalize all ExternalInterfaces in the
//              list, as well as the list itself
// PARAMETERS ::
// + list : EXTERNAL_InterfaceList* : The external interface list
// RETURN    :: void :
// **/
void EXTERNAL_FinalizeInterfaceList(EXTERNAL_InterfaceList *list);

// /**
// API       :: EXTERNAL_GetInterfaceByName
// PURPOSE   :: This function will search an interface list for an interface
//              with the given name
// PARAMETERS ::
// + list : EXTERNAL_InterfaceList* : The external interface list
// + name : char* : The interface's name
// RETURN    :: EXTERNAL_Interface* : The interface, NULL if not found
// **/
EXTERNAL_Interface* EXTERNAL_GetInterfaceByName(
    EXTERNAL_InterfaceList *list,
    const char *name);

// API       :: EXTERNAL_GetInterfaceByUniqueId
// PURPOSE   :: This function will search an interface list for an interface
//              with the given uniqueId.  This is NOT the interface type
// PARAMETERS::
// + list : EXTERNAL_InterfaceList* : The external interface list
// + uniqueId : int : The interface's unqiued id
// RETURN    :: EXTERNAL_Interface* : The interface, NULL if not found
// **/
EXTERNAL_Interface* EXTERNAL_GetInterfaceByUniqueId(
    EXTERNAL_InterfaceList *list,
    int uniqueId);

// /**
// API       :: EXTERNAL_CallInitializeFunctions
// PURPOSE   :: This function will call all initialize functions
// PARAMETERS ::
// + list : EXTERNAL_InterfaceList* : The list of external interfaces
// + nodeInput : NodeInput* : The input configuration file
// RETURN    :: void :
// **/
void EXTERNAL_CallInitializeFunctions(
    EXTERNAL_InterfaceList *list,
    NodeInput *nodeInput);

// /**
// API       :: EXTERNAL_CallInitializeNodesFunctions
// PURPOSE   :: This function will call all intialize nodes functions
// PARAMETERS ::
// + list : EXTERNAL_InterfaceList* : The list of external interfaces
// + nodeInput : NodeInput* : The input configuration file
// RETURN    :: void :
// **/
void EXTERNAL_CallInitializeNodesFunctions(
    EXTERNAL_InterfaceList *list,
    NodeInput *nodeInput);

// /**
// API       :: EXTERNAL_StartThreads
// PURPOSE   :: This function will start the receive/forward threads for
//              all threaded interfaces.  Called after
//              EXTERNAL_CallInitializeNodesFunctions.
// PARAMETERS ::
// + list : EXTERNAL_InterfaceList* : The list of external interfaces
// RETURN    :: void :
// **/
void EXTERNAL_StartThreads(EXTERNAL_InterfaceList *list);

// /**
// API       :: EXTERNAL_CalculateMinSimulationHorizon
// PURPOSE   :: This function will call all simulation horizon functions
//              to determine how far into the future the simulation can run.
//              An individual simulation horizon function will only be called
//              if the current time (now) is >= that interface's current
//              horizon.
// PARAMETERS ::
// + list : EXTERNAL_InterfaceList* : The list of external interfaces
// + now : clocktype : The current time
// RETURN    :: clocktype : The minimum Simulation Horizon, or
//                          EXTERNAL_MAX_TIME if no horizon.
// **/
clocktype EXTERNAL_CalculateMinSimulationHorizon(
    EXTERNAL_InterfaceList *list,
    clocktype now);

// /**
// API       :: EXTERNAL_CallReceiveFunctions
// PURPOSE   :: This function will call all receive function that were not
//              started in a thread
// PARAMETERS ::
// + list : EXTERNAL_InterfaceList* : The list of external interfaces
// RETURN    :: void :
// **/
void EXTERNAL_CallReceiveFunctions(EXTERNAL_InterfaceList *list);

// /**
// API       :: EXTERNAL_CallFinalizeFunctions
// PURPOSE   :: This function will call all finalize functions
// PARAMETERS ::
// + list : EXTERNAL_InterfaceList* : The list of external interfaces
// RETURN    :: void :
// **/
void EXTERNAL_CallFinalizeFunctions(EXTERNAL_InterfaceList *list);

// /**
// API         :: EXTERNAL_InitializeExternalInterfaces
// PURPOSE     :: Function used to initialize a generic interface to
// an external source of messages, e.g. an HLA federate. Called before
// nodes are created.
// PARAMETERS  ::
// + partitionData : partitionData* : pointer to data for this partition
// RETURN      :: void :
// **/
void EXTERNAL_InitializeExternalInterfaces(PartitionData* partitionData);

// /**
// API         :: EXTERNAL_PostInitialize
// PURPOSE     :: Function used to initialize a generic interface to
// an external source of messages, e.g. an HLA federate.Called after
// nodes are created.  The developer can use either this function,
// the preceding one or both.
// PARAMETERS  ::
// + partitionData : partitionData* : pointer to data for this partition
// RETURN      :: void :
// **/
void EXTERNAL_PostInitialize(PartitionData* partitionData);

// /**
// API         :: EXTERNAL_GetExternalMessages
// PURPOSE     :: Function used to retrieve messages from a remote source,
// such as a DIS gateway or HLA federation. Called before events at time
// X are executed. Many events at time X may be executed before the next call
// PARAMETERS  ::
// + partitionData : partitionData* : pointer to data for this partition
// + nextInternalEventTime : clocktype : the time of the next event,
// not including the yet to be loaded external events, but possibly
// including previously loaded external events.
// RETURN      :: void :
// **/
void EXTERNAL_GetExternalMessages(PartitionData* partitionData,
                                  clocktype      nextInternalEventTime);

// /**
// API         :: EXTERNAL_Finalize
// PURPOSE     :: Shuts down interfaces to external simulators
// PARAMETERS  ::
// + partitionData : partitionData* : pointer to data for this partition
// RETURN      :: void :
// **/
void EXTERNAL_Finalize(PartitionData* partitionData);

// /**
// API         :: EXTERNAL_SetActive
// PURPOSE     :: Sets isActive parameter based on interface registration
// PARAMETERS  ::
// + partitionData : partitionData* : pointer to data for this partition
// RETURN      :: void :
// **/
void EXTERNAL_SetActive(EXTERNAL_InterfaceList *list);

// /**
// API         :: EXTERNAL_DeactivateInterface
// PURPOSE     :: Remove the indicated interface for the list of currently
//                activateed interfaces.
// PARAMETERS  ::
// + ifaceToDeactivate : EXTERNAL_Interface* : Pointer to the interface
// RETURN      :: void
// **/
void EXTERNAL_DeactivateInterface (EXTERNAL_Interface * ifaceToDeactivate);

// /**
// API         :: EXTERNAL_ProcessEvent
// PURPOSE     :: Process events meant for external code.
// PARAMETERS  ::
// + node : Node* : Pointer to node data structure.
// + msg : Message* : Message to be processed.
// RETURN      :: void :
// **/
void EXTERNAL_ProcessEvent(Node* node, Message* msg);

// /**
// API       :: GetNextInternalEventTime
// PURPOSE   :: Get the next internal event on the given partition.  This
//              includes both regular events and mobility events.
// PARAMETERS ::
// + partitionData : PartitionData* : Pointer to the partition
// RETURN    :: clocktype : The next internal event
// **/
clocktype GetNextInternalEventTime(PartitionData* partitionData);

#ifdef EXATA
#ifndef _WIN32
void EXTERNAL_AddFileDescForListening(
    EXTERNAL_Interface *iface,
    int fd);
#endif
#endif

#endif /* _EXTERNAL_H_ */
