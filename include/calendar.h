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
// PACKAGE     :: CALENDAR QUEUE
// DESCRIPTION :: This file describes data structures and functions to implement calendar queues.
// **/

#ifndef CALENDAR_QUEUE_H
#define CALENDAR_QUEUE_H

#include "api.h"

// /**
// CONSTANT     :: CALENDAR_MAX_FREE_LIST : 50000
// DESCRIPTION  :: Maximum number of free cells in calendar queue.
// **/
#define CALENDAR_MAX_FREE_CELLS 50000

struct CalendarCell;

// /**
// STRUCT      :: CalendarItem
// DESCRIPTION :: contains a node's event
// **/
struct CalendarItem {
    clocktype           time;
    void                *itemData;
    int                 id;
    int                 event;
    Node                *node;
};

// /**
// STRUCT      :: CalendarCell
// DESCRIPTION :: a bucket in the calendar queue
// **/
struct CalendarCell {
    CalendarItem entry; 
    CalendarCell*  next;
};

// /**
// STRUCT      :: CalendarQInfo
// DESCRIPTION :: the calendar queue
// **/
struct CalendarQInfo {
    int                 capacity;               // size of bucketspace
    BOOL                resizeEnabled;
    CalendarCell        **buckets;              // array of ptrs, dyn allocd at setup time
    CalendarCell        **activeBuckets;        // array of active bucket ptrs, subset of above
    int                 firstsub;
    int                 numBuckets;
    int                 splitThreshold;         // size > splitThreshold --> resize * 2
    int                 foldThreshold;          // size < foldThreshold  --> resize * 1/2
    double              calWidth;
    int                 previousBucket;
    clocktype           previousKey;
    double              bucketTop;
    int                 maxNumBuckets;

    int                 qSize;
    int                 hiWatermark;            // largest value of size in this run
    int                 foundInBucket;
    int                 foundDirect;
    int                 foundBySearch;

    CalendarCell        *currentNode;
    int                 numEvents;

    int                 calNodeFreeListNum;
    CalendarCell        *calNodeFreeList;
    
};

// protos
//  --- For Debugging 
void CalendarDisplay(CalendarQInfo *calInfo);
void ItemDisplay(CalendarItem* item);

// /**
// FUNCTION  :: CalendarInsert
// PURPOSE   :: This routine adds one entry to the future event queue if the
//              calendar queueing method was chosen in init_simpack().  The priority
//              (or time) of the event is given in the ITEM structure.
// PARAMETERS ::
// + calInfo : CalendarQInfo* : calendar queue being used.
// + cell : CalendarCell* : the calender cell.
// RETURN    :: void : NONE
// **/
void CalendarInsert(CalendarQInfo *calInfo, CalendarCell *cell);

// /**
// FUNCTION  :: CalendarPeek
// PURPOSE   :: This function retuns the next item from the calendar queue
//              without extracting it.
// PARAMETERS ::
// + calInfo : CalendarQInfo* : calendar queue being used.
// RETURN    :: Item* : The item cell
// **/
CalendarCell* CalendarPeek(CalendarQInfo *calInfo);

// /**
// FUNCTION  :: CalendarExtract
// PURPOSE   :: This function removes the next item from the calendar queue
// PARAMETERS ::
// + calInfo : CalendarQInfo* : calendar queue being used.
// RETURN    :: Item* : The item cell
// **/
CalendarCell* CalendarRemove(CalendarQInfo *calInfo);

// /**
// FUNCTION  :: CalendarFind
// PURPOSE   :: This function finds the item with the id in the calendar queue. 
// PARAMETERS ::
// + calInfo : CalendarQInfo* : calendar queue being used.
// + id : int : the item ID.
// RETURN    :: Item* : The item cell
// **/
CalendarCell* CalendarFind(CalendarQInfo *calInfo, int id);

// /**
// FUNCTION  :: CalendarFindAndRemove
// PURPOSE   :: This function finds the item with the id from the calendar queue.
//              and remove it. 
// PARAMETERS ::
// + calInfo : CalendarQInfo* : calendar queue being used.
// + id : int : the item ID.
// RETURN    :: Item* : The item cell removed.
// **/
CalendarCell* CalendarFindAndRemove(CalendarQInfo *calInfo, int id);

// /**
// FUNCTION  :: CalendarHasItems
// PURPOSE   :: This function returns TRUE it there are items in the calendar queue.
// PARAMETERS ::
// + calInfo : CalendarQInfo* : calendar queue being used.
// RETURN    :: BOOL : TRUE has items in queue.
// **/

static
BOOL CalendarHasItems(CalendarQInfo *calInfo)
{
    if(calInfo->qSize > 0)
        return TRUE; 
    else
        return FALSE;
}

// /**
// FUNCTION  :: CalendarInit
// PURPOSE   :: This routine starts the calendar with nothing in it for initializing. 
// PARAMETERS ::
// + calInfo : CalendarQInfo* : calendar queue being used.
// RETURN    :: void :
// **/
void CalendarInit(CalendarQInfo *calInfo);

// /**
// FUNCTION  :: CalendarFinalize
// PURPOSE   :: This routine frees the calendar.  
// PARAMETERS ::
// + calInfo : CalendarQInfo* : calendar queue being used.
// RETURN    :: void :
// **/
void CalendarFinalize(CalendarQInfo *calInfo);


// /**
// FUNCTION  :: CalendarFree
// PURPOSE   :: This routine frees a cell.  
// PARAMETERS ::
// + calInfo : CalendarQInfo* : calendar queue being used.
// + cell : CalendarCell* : the calender cell.
// RETURN    :: void : NONE
// **/
static
void CalendarFreeCell(CalendarQInfo *calInfo, CalendarCell *cell)
{

    if (calInfo->calNodeFreeListNum < CALENDAR_MAX_FREE_CELLS){
        CalendarCell *cellPtr = (CalendarCell *) cell;
        cellPtr->next = calInfo->calNodeFreeList;
        calInfo->calNodeFreeList = cellPtr;
        calInfo->calNodeFreeListNum++;
    } else
        MEM_free(cell);
}

// /**
// FUNCTION  :: CalendarAllocate
// PURPOSE   :: This routine allocates a cell.  
// PARAMETERS ::
// + calInfo : CalendarQInfo* : calendar queue being used.
// + cell : CalendarCell* : the calender cell.
// RETURN    :: CalendarCell* : the new cell.
// **/
static
CalendarCell* CalendarAllocateCell(CalendarQInfo *calInfo)
{
    CalendarCell* cell = NULL;

    if (calInfo->calNodeFreeList == NULL) {
        cell = (CalendarCell*) MEM_malloc(sizeof(CalendarCell));

    } else {
        cell = calInfo->calNodeFreeList;
        calInfo->calNodeFreeList = calInfo->calNodeFreeList->next;
        calInfo->calNodeFreeListNum--;
    }

    assert(cell);
    memset(cell, 0, sizeof(CalendarCell));

    assert(cell->next == NULL);

    return cell;
}

#endif //CALENDAR_QUEUE_H

