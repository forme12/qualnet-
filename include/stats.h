// Copyright (c) 2001-2008, Scalable Network Technologies, Inc.  All Rights Reserved.
//                          6701 Center Drive West
//                          Suite 520
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
// PACKAGE     :: STATS
// DESCRIPTION :: This file describes the implementation of the STATS API.
// **/

#ifndef _STATS_H_
#define _STATS_H_

#include <algorithm>
#include <vector>
#include <list>

#include "dynamic.h"
#include "dynamic_vars.h"


class STAT_Statistic;
class STAT_StatisticsList;

#define FLOAT64_MAX std::numeric_limits<float>::max()

typedef UInt32 STAT_SessionIdType;

typedef Int8 STAT_DestAddressType;
#define STAT_Unicast 0
#define STAT_Broadcast 1
#define STAT_Multicast 2
#define STAT_NUM_ADDRESS_TYPES 3

// Return a string representation of an address type
std::string STAT_AddrToString(STAT_DestAddressType type);

// Convert an address to an address type
STAT_DestAddressType STAT_NodeAddressToDestAddressType(Node* node, NodeAddress& addr);
STAT_DestAddressType STAT_AddressToDestAddressType(Node*, const Address& addr);

enum STAT_Format
{
    STAT_GuessFormat = 0,
    STAT_Float = 1,
    STAT_Integer = 2,
    STAT_Time = 3
};

#define STAT_UNDEFINED -1

typedef std::vector<STAT_Statistic*>::iterator STAT_VectorIterator;

struct STAT_Timing
{
    // If this is an application layer then this contains the source
    // address of the sender, the flow's unique session id and the
    // size of the message (not including control)
    NodeId sourceNodeId;
    int uniqueSessionId;
    int messageSize;

    // When the packet was sent from the application layer
    clocktype sentApplicationDown;

    // When the packet was received at the transport layer from upper layer
    // May be equal to sentApplicationDown for data packets
    // Also covers control packets originated at transport layer
    clocktype receivedTransportUp;
    
    // When the packet was sent from the transport layer
    clocktype sentTransportDown;

    // When the packet was sent from the network layer
    clocktype sourceSentNetworkDown;

    // When the packet leaves the mac output queue
    clocktype leftMacOutputQueue;

    // When the packet was sent from the mac layer
    clocktype sentMacDown;

    // When the packet was sent from the phy layer
    clocktype sentPhyDown;

    // Number of hops
    int hopCount;

    // Node Id of last hop 
    NodeId lastHopNodeId;

    // Interface index of last hop.
    Int32 lastHopIntfIndex;

    void Initialize();
};

// /**
// API             :: STAT_GetTiming
// PURPOSE         :: Return timing info for this message.  If no timing
//                    info is available then new info is created.  Do not
//                    use this function if you need to test for valid timing
//                    data.
// PARAMETERS      ::
// + node           : Node *       : The current node
// + message        : Message*     : Pointer to the message
// RETURN          :: STAT_Timing* : The timing info for this message
// **/
STAT_Timing* STAT_GetTiming(Node* node, Message* message);

// /**
// CLASS        :: STAT_Statistic
// DESCRIPTION  :: Statistic base class for STAT_Statistic and
//                 STAT_AggregatedStatistic
//
//                 Serialize - Efficiently serializes statistic information
//                     for this partition.  Will be sent to all other
//                     partitions.
//                 Deserialize - Efficienty deserializes a statistic
//                 Update - Update a statistic with the value from another.
//                     Mostly userful for aggregation.
// **/
class STAT_Statistic
{
protected:
    D_Statistic m_Value;
    UInt64 m_NumDataPoints;
    const std::string* m_Name;
    const std::string* m_Description;
    const std::string* m_Units;
    STAT_Format m_Format;

public:
    STAT_Statistic() : m_NumDataPoints(0), m_Name(NULL), m_Description(NULL), m_Units(NULL) {}

    void SetInfo(std::string& name, std::string& description, std::string& units, STAT_Format format = STAT_GuessFormat);
    void AddToHierarchy(D_Hierarchy* h, std::string& path);
    const std::string& GetName() const { return *m_Name; }
    const std::string& GetDescription() const { return *m_Description; }
    const std::string& GetUnits() const { return *m_Units; }
    STAT_Format GetFormat() { return m_Format; }

    int GetNumDataPoints() { return (int)m_NumDataPoints;}

    // Must be implemented by all statistics.  Does not need to be
    // implemented by aggregated statistics.
    virtual void AddDataPoint(clocktype now, double val);

    virtual Float64 GetValue(clocktype now) { return m_Value; }
    Int64 GetValueAsInt(clocktype now);
    std::string GetValueAsString(clocktype now);
    clocktype GetValueAsClocktype(clocktype now);

    // Resets the statistic to its initial state (usually 0)
    virtual void Reset();

    virtual void Serialize(clocktype now, char** data, int* size);
    virtual void DeserializeAndUpdate(clocktype now, char* data, int size);
};

// /**
// CLASS        :: STAT_ModelStatistics
// DESCRIPTION  :: Base class for model statistics.  Inherited by model
//                 classes.
// **/
class STAT_ModelStatistics
{
    private:
        BOOL m_IsFinal;

    public:
        STAT_ModelStatistics() : m_IsFinal(FALSE) {}
        ~STAT_ModelStatistics();

        virtual void Initialize(Node* node);

        // Overloaded functions for stats printing

        void Print(Node* node, const char* layer, const char* model, NodeAddress addr, int instanceId);
        void Print(Node* node, const char* layer, const char* model, NodeAddress addr, const std::string& instanceId);        
        void Print(Node* node, const char* layer, const char* model, const char* addr, int instanceId);

        virtual void GetList(std::vector<STAT_Statistic*>& stats) = 0;

        // Models should call Finalize when the model ends.  After Finalize is
        // called a few things will happen:
        //     - The kernel will take control of the model statistics
        //     - The model should NOT delete its statistics class (this
        //       implies that the model statistics are stored as a pointer
        //       in the model)
        //     - The model should NOT update the statisics
        //     - Doing these things may result in an assert or inaccurate
        //       statistics
        virtual void Finalize(Node* node);
};

// /**
// CLASS        :: STAT_Sum
// DESCRIPTION  :: Implements a statistic that sums all input data points.
//                 Value is 0 if no data points have been added.
// **/
class STAT_Sum : public STAT_Statistic
{
public:
    virtual void AddDataPoint(clocktype now, double val);
};

// /**
// CLASS        :: STAT_Maximum
// DESCRIPTION  :: Implements a statistic that computes the maximum of all
//                 input data points.  Value is 0 if no data points have
//                 been added.
// **/
class STAT_Maximum : public STAT_Statistic
{
public:
    virtual void AddDataPoint(clocktype now, double val);
};

// /**
// CLASS        :: STAT_Minimum
// DESCRIPTION  :: Implements a statistic that computes the minimum of all
//                 input data points.  Value is FLOAT64_MAX if no data
//                 points have been added.
// **/
class STAT_Minimum : public STAT_Statistic
{
public:
    virtual void AddDataPoint(clocktype now, double val);
};

// /**
// CLASS        :: STAT_Average
// DESCRIPTION  :: Implements a statistic that averages all input data
//                 points.  Value is 0 if no data points have been added.
// **/
class STAT_Average : public STAT_Statistic
{
public:
    virtual void AddDataPoint(clocktype now, double val);
    virtual Float64 GetValue(clocktype now);
};

// /**
// CLASS        :: STAT_Average_dBm
// DESCRIPTION  :: Implements a statistic that averages all input data
//                 points.  Value is 0 if no data points have been added.
//                 Assumes input value is mW.  Output is dBm.  Values of
//                 0 mW are ignored.
// **/
class STAT_Average_dBm : public STAT_Average
{
public:
    virtual void AddDataPoint(clocktype now, double val);
    virtual Float64 GetValue(clocktype now);
};

// /**
// CLASS        :: STAT_SessinFinish
// DESCRIPTION  :: Implements a statistic that calculates the session
//                 finish.  Returns current time if not finished or
//                 session finish time if finished.
//                 
// **/
class STAT_SessionFinish : public STAT_Statistic
{
public:
    virtual void AddDataPoint(clocktype now, double val);
};
// /**
// CLASS        :: STAT_Throughput
// DESCRIPTION  :: Implements a statistic that calculates throughput or
//                 offered load.
//                 Value is 0 if no data points have been added.
//                 AddDataPoint will call SessionStart if it has not been
//                 called explicitly.  Throughput value is computed between
//                 SessionStart time and SessionFinish time or the current
//                 sim time if SessionFinish has not been called.  Input
//                 data points should be packet sizes.
//                 
// **/
class STAT_Throughput : public STAT_Statistic
{
protected:
    Float64 m_BytesProcessed;
    Float64 m_SessionStart;
    Float64 m_SessionFinish;

public:
    STAT_Throughput();
    clocktype GetSessionStart() { return (clocktype) (m_SessionStart * SECOND); }
    clocktype GetSessionFinish() { return (clocktype) (m_SessionFinish * SECOND); }
    Float64 GetBytesProcessed() { return m_BytesProcessed; }
    void SessionStart(clocktype now);
    void SessionFinish(clocktype now);
    virtual void AddDataPoint(clocktype now, double val);
    virtual Float64 GetValue(clocktype now);
    virtual void Reset();
    Float64 GetValue(clocktype now, Float64 bytesProcessed);
};

// /**
// CLASS        :: STAT_Utilization
// DESCRIPTION  :: Implements a statistic that calculates utilization
//                 Value is 0 if no data points have been added.
//                 AddDataPoint will call SessionStart if it has not been
//                 called explicitly.  Throughput value is computed between
//                 SessionStart time and SessionFinish time or the current
//                 sim time if SessionFinish has not been called.  Input
//                 data points should be packet sizes.
//                 
// **/
class STAT_Utilization : public STAT_Statistic
{
protected:
    Float64 m_ActivityTime;
    clocktype m_SessionStart;
    clocktype m_SessionFinish;

public:
    STAT_Utilization();
    clocktype GetSessionStart() { return m_SessionStart; }
    clocktype GetSessionFinish() { return m_SessionFinish; }
    Float64 GetActivityTime() { return m_ActivityTime; }
    void SessionStart(clocktype now);
    void SessionFinish(clocktype now);
    virtual void AddDataPoint(clocktype now, double val);
    virtual Float64 GetValue(clocktype now);
    virtual void Reset();
};

// /**
// CLASS        :: STAT_SmoothedAverage
// DESCRIPTION  :: Implements a statistic that calculates smoothed average.
//                 The smoothing factor should be set before any data points
//                 have been added.
//                 Value is 0 if no data points have been added.
//                 
// **/
class STAT_SmoothedAverage : public STAT_Statistic
{
private:
    Float64 m_SmoothingFactor;

public:
    STAT_SmoothedAverage() : m_SmoothingFactor(16.0) {}
    void SetSmoothingFactor(double smoothingFactor) { m_SmoothingFactor = smoothingFactor; }
    virtual void AddDataPoint(clocktype now, double val);
};

// /**
// CLASS        :: STAT_TimeWeightedAverage
// DESCRIPTION  :: Implements a statistic that calculates weighted average by time.
//                 Value is 0 if no data points have been added.
//                 
// **/
class STAT_TimeWeightedAverage : public STAT_Statistic
{
private:
    clocktype m_timeOfLastChange;
public:
    virtual void AddDataPoint(clocktype now, double val);
    virtual Float64 GetValue(clocktype now);
    void SetLastChange(clocktype creationTime);
};

// /**
// CLASS        :: STAT_AggregatedStatistic
// DESCRIPTION  :: Base class for all aggregated statistics.  All stats must
//                 implement the following functions:
//
//                 Aggregate - Performs aggregation for all statistic values
//                     on this partition.
//
// **/
class STAT_AggregatedStatistic : public STAT_Statistic
{
protected:
    // Global id
    int m_Id;
    int m_Order;
    std::string m_IteratorPath;
    std::vector<STAT_Statistic*> m_Statistics;
    
    STAT_VectorIterator FindStatistic(STAT_Statistic* stat);

public:
    STAT_AggregatedStatistic() : m_Id(STAT_UNDEFINED), m_Order(0) {}

    void SetId(int id) { m_Id = id; }
    int GetId() { return m_Id; }

    int GetOrder() { return m_Order; }

    void SetIteratorPath(std::string& path);
    const std::string& GetIteratorPath() { return m_IteratorPath; }
    void RebuildIterator(D_Hierarchy* h);

    void AddStatistic(STAT_Statistic* stat);
    void RemoveStatistic(STAT_Statistic* stat);

    virtual void Aggregate(clocktype now) = 0;
};


// /**
// STRUCT       :: STAT_SimpleAggregationData
// DESCRIPTION  :: Used for sending aggregated data across partitions.  This
//                 struct can be used for simple aggregated data types where
//                 value and num data points is enough
// **/
struct STAT_SimpleAggregationData
{
    Float64 m_Value;
    UInt64 m_NumDataPoints;
};

// /**
// STRUCT       :: STAT_AggregationData
// DESCRIPTION  :: Used for sending aggregated data across partitions.  This
//                 struct can be used for more complex aggregated data types
//                 where an additional integer is required.
// **/
struct STAT_AggregationData
{
    Float64 m_Value;
    Int64 m_Value2;
    UInt64 m_NumDataPoints;
};

// /**
// CLASS        :: STAT_AggregatedSum
// DESCRIPTION  :: Implements a statistic that aggregates the sum of all
//                 statistics.
//                 Value is 0 if no data points have been added.
// **/
class STAT_AggregatedSum : public STAT_AggregatedStatistic
{
public:
    virtual void Aggregate(clocktype now);
    virtual void Serialize(clocktype now, char** data, int* size);
    virtual void DeserializeAndUpdate(clocktype now, char* data, int size);
};

// /**
// CLASS        :: STAT_AggregatedAverage
// DESCRIPTION  :: Implements a statistic that aggregates the average of all
//                 statistics.
//                 Value is 0 if no data points have been added.
// **/
class STAT_AggregatedAverage : public STAT_AggregatedStatistic
{
private:
    int m_NumStatistics;

public:
    virtual void Aggregate(clocktype now);
    virtual void Serialize(clocktype now, char** data, int* size);
    virtual void DeserializeAndUpdate(clocktype now, char* data, int size);
};

// /**
// CLASS        :: STAT_AggregatedWeightedDataPointAverage
// DESCRIPTION  :: Implements a statistic that aggregates the average of all
//                 statistics weighted by their number of data points.
//                 Value is 0 if no data points have been added.
// **/
class STAT_AggregatedWeightedDataPointAverage : public STAT_AggregatedStatistic
{
public:
    virtual void Aggregate(clocktype now);
    virtual void Serialize(clocktype now, char** data, int* size);
    virtual void DeserializeAndUpdate(clocktype now, char* data, int size);
};

// /**
// CLASS        :: STAT_AggregatedThroughput
// DESCRIPTION  :: Implements a statistic that aggregates throughput.  
//                 Value is 0 if no data points have been added.
//                 Throughput value is computed between the first sesion
//                 start time and the current sim time (does not use session
//                 finish).  Only STAT_Throughput stats may be added to
//                 STAT_AggregatedThroughput.
//                 
// **/
class STAT_AggregatedThroughput : public STAT_AggregatedStatistic
{
private:
    Float64 m_BytesProcessed;
    
public:
    STAT_AggregatedThroughput() : m_BytesProcessed(0) {}

    Float64 GetBytesProcessed() { return m_BytesProcessed; }

    virtual void Aggregate(clocktype now);
    virtual void Serialize(clocktype now, char** data, int* size);
    virtual void DeserializeAndUpdate(clocktype now, char* data, int size);
};

// /**
// CLASS        :: STAT_AggregatedDivide
// DESCRIPTION  :: Implements a statistic that divides other statisics.
//                 If multiple statistics are added the first statistic is
//                 the numerator and all subsequent statistics are the
//                 denominator.  This statistic's order must be higher than
//                 all others.  If one of the denominators is zero then the
//                 value will be zero.
//                 
// **/
class STAT_AggregatedDivide : public STAT_AggregatedStatistic
{
public:
    virtual void Aggregate(clocktype now);
    virtual void Serialize(clocktype now, char** data, int* size);
    virtual void DeserializeAndUpdate(clocktype now, char* data, int size);
};

// /**
// STRUCT       :: STAT_DataList
// DESCRIPTION  :: List of aggregated data
//                 
// **/
struct STAT_DataList
{
    int id;
    char* data;
    int size;
    STAT_DataList* next;
};

#endif // _STATS_H_
