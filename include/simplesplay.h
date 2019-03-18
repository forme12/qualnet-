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
// PACKAGE      :: SPLAYTREE
// DESCRIPTION  :: This file describes a tree and a heap structure called
//                 splaytree and heapSplaytree to handle the timed activities
//                 of the network model
// **/

#ifndef _SIMPLE_SPLAYTREE_H_
#define _SIMPLE_SPLAYTREE_H_

#include "clock.h"
#include "memory.h"

// /**
// STRUCT       :: SimpleSplayNode
// DESCRIPTION  :: Structure of each node of a Splaytree
// **/
typedef struct simple_splay_node_str {
    clocktype timeValue;
    Message*     element;
    struct simple_splay_node_str* leftPtr;
    struct simple_splay_node_str* rightPtr;
    struct simple_splay_node_str* parentPtr;
} SimpleSplayNode;

// /**
// STRUCT       :: SimpleSplayTree
// DESCRIPTION  :: Structure of a Splaytree
// **/
typedef struct simple_splay_tree_str {
    clocktype        timeValue;
    SimpleSplayNode* rootPtr;
    SimpleSplayNode* leastPtr;
} SimpleSplayTree;

// /**
// API       :: SPLAY_Insert
// PURPOSE   :: To insert an element into the Splaytree
// PARAMETERS ::
// + tree         : SimpleSplayTree* : Pointer to the splaytree node
// + splayNodePtr : SimpleSplayNode* : Pointer to the splayNode
//                               to be inserted
// RETURN    :: void :
// **/
void SPLAY_Insert(SimpleSplayTree* tree,
                  SimpleSplayNode* splayNodePtr);

// API       :: SPLAY_ExtractMin
// PURPOSE   :: To extract an element from the Splaytree
// PARAMETERS ::
// + tree       : SimpleSplayTree* : Pointer to the splaytree node
//                           to be extracted
// RETURN    :: SimpleSplayNode* : Pointer to extracted Splaynode
// **/
SimpleSplayNode* SPLAY_ExtractMin(SimpleSplayTree* tree);


// API       :: SPLAY_AllocateNode
// PURPOSE   :: To allocate a node from the SplaytreeFreeList
// PARAMETERS ::
// + node       : Node*    : Pointer to the splaytree node
//                           to be extracted
// RETURN    :: SimpleSplayNode* : Pointer to extracted Splaynode
// **/
static
SimpleSplayNode* SPLAY_AllocateNode() {

    SimpleSplayNode* splayNodePtr =
        (SimpleSplayNode *) MEM_malloc( sizeof(SimpleSplayNode) );
    assert(splayNodePtr != NULL);
    memset(splayNodePtr, 0, sizeof(SimpleSplayNode));

    return splayNodePtr;
}


// API       :: SPLAY_FreeNode
// PURPOSE   :: To free a node from the SplaytreeFreeList
// PARAMETERS ::
// + splayNodePtr       : SimpleSplayNode*        : Pointer to splay node
// RETURN    :: void :
// **/
static
void SPLAY_FreeNode(SimpleSplayNode* splayNodePtr) {

    MEM_free(splayNodePtr);
    splayNodePtr = NULL;
}

// /**
// API       :: SPLAY_SplayTreeAtNode
// PURPOSE   :: utility function used by SPLAY_Insert
// PARAMETERS ::
// + splayPtr : SimpleSplayTree* : Pointer to the splaytree node
// + nodePtr  : SimpleSplayNode* : Pointer to the splayNode
//                               to be inserted
// RETURN    :: void :
// **/
void SPLAY_SplayTreeAtNode(SimpleSplayTree *splayPtr,
                           SimpleSplayNode *nodePtr);

#endif /* _SIMPLE_SPLAYTREE_H_ */

