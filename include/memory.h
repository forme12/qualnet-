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
// PACKAGE      :: MEMORY
// DESCRIPTION  :: This file describes the memory management
//                 data structures and functions.
// **/

#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

// /**
// STRUCT      :: MemoryUsageData
// DESCRIPTION :: Defines the parameters collected by the memory system.
//                Restricted to kernel use.
// **/
struct MemoryUsageData
{
    UInt32 forPeakUsage;
    UInt32 totalAllocated;
    UInt32 totalFreed;
    UInt32 peakUsage;
    UInt32 partitionId;
};

extern UInt32 totalAllocatedMemory;
extern UInt32 totalFreedMemory;
extern UInt32 totalPeakUsage;

// /**
// API         :: MEM_CreateThreadData
// PURPOSE     :: Creates partition-specific space for collecting memory usage
//                statistics.  This is used in threaded versions of QualNet,
//                but not in distributed versions, currently.
// PARAMETERS  ::
// RETURN      :: void :
// **/
void MEM_CreateThreadData();

// /**
// API         :: MEM_InitializeThreadData
// PURPOSE     :: Sets the partition-specific memory data for this partition.
// PARAMETERS  ::
// + data : MemoryUsageData* : the data
// RETURN      :: void :
// **/
void MEM_InitializeThreadData(MemoryUsageData* data);

// /**
// API         :: MEM_PrintThreadData
// PURPOSE     :: Prints the partition-specific memory data.
// PARAMETERS  ::
// RETURN      :: void :
// **/
void MEM_PrintThreadData();

// /**
// API         :: MEM_ReportPartitionUsage
// PURPOSE     :: Prints out the total memory used by this partition.
// PARAMETERS  ::
// + partitionId          : int    : the partition number
// + totalAllocatedMemory : UInt32 : sum of all MEM_malloc calls
// + totalFreedMemory     : UInt32 : sum of all MEM_free calls
// + totalPeakUsage       : UInt32 : peak usage of allocated memory
// RETURN      :: void :
// **/
void MEM_ReportPartitionUsage(int    partitionId,
                              UInt32 totalAllocatedMemory,
                              UInt32 totalFreedMemory,
                              UInt32 totalPeakUsage);

// /**
// API         :: MEM_ReportTotalUsage
// PURPOSE     :: Prints out the total memory usage statistics for the
//                simulation.  In a parallel run, the peak usage is the sum
//                of the partition's peak usage and might not be precisely
//                accurate.
// PARAMETERS  ::
// + totalAllocatedMemory : UInt32 : sum of all MEM_malloc calls
// + totalFreedMemory     : UInt32 : sum of all MEM_free calls
// + totalPeakUsage       : UInt32 : peak usage of allocated memory
// RETURN      :: void :
// **/
void MEM_ReportTotalUsage(UInt32 totalAllocatedMemory,
                          UInt32 totalFreedMemory,
                          UInt32 totalPeakUsage);

#endif // MEMORY_H
