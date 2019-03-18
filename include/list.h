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
// PACKAGE :: LIST
// DESCRIPTION :: This file describes the data structures and functions used in the
//                implementation of lists.
// **/


#ifndef _QUALNET_LIST_H_
#define _QUALNET_LIST_H_


// /**
// STRUCT      :: ListItem template
// DESCRIPTION :: Structure for each item of a generic container list
// **/
template<typename DATA>
struct tListItem
{
    DATA data;
    clocktype timeStamp;
    tListItem<DATA> *prev;
    tListItem<DATA> *next;
};

//
//
//
//       DEFINES A LIST THAT HOLDS POINTERS
//
//
///////////////////////////////////////////////////////////////////

typedef tListItem<void*> ListItem;

// /**
// STRUCT      :: List
// DESCRIPTION :: A list that stores different types of structures.
// **/
typedef struct LinkedList_
{
    int size;
    ListItem *first;       // First item in list.
    ListItem *last;        // Last item in list.
} LinkedList;

// /**
// FUNCTION     :: ListInit
// LAYER        :: ANY LAYER
// PURPOSE      :: Initialize the list
// PARAMETERS   ::
// + node        : Node* : Node that contains the list
// + list        : LinkedList**: Pointer to list pointer
// RETURN       :: void : NULL
// **/
void ListInit(Node *node, LinkedList **list);

// /**
// FUNCTION     :: ListIsEmpty
// LAYER        :: ANY LAYER
// PURPOSE      :: Check if list is empty
// PARAMETERS   ::
// + node        : Node* : Node that contains the list
// + list        : LinkedList* : Pointer to the list
// RETURN       :: BOOL : If empty, TRUE, non-empty, FALSE
// **/
BOOL ListIsEmpty(Node *node, LinkedList *list);

// /**
// FUNCTION     :: ListGetSize
// LAYER        :: ANY LAYER
// PURPOSE      :: Get the size of the list
// PARAMETERS   ::
// + node        : Node* : Pointer to the node containing the list
// + list        : LinkedList* : Pointer to the list
// RETURN       :: int : Size of the list
// **/
int ListGetSize(Node *node, LinkedList *list);

// /**
// FUNCTION     :: ListInsert
// LAYER        :: ANY LAYER
// PURPOSE      :: Insert an item at the end of the list
// PARAMETERS   ::
// + node        : Node* : Pointer to the node containing the list
// + list        : LinkedList* : Pointer to the list
// + timeStamp   : clocktype : Time the item was last inserted.
// + data        : void* : item to be inserted
// RETURN       :: void : NULL
// **/
void ListInsert(Node *node,
                LinkedList *list,
                clocktype timeStamp,
                void *data);

// /**
// FUNCTION     :: FindItem
// LAYER        :: ANY LAYER
// PURPOSE      :: Find an item from the list
// PARAMETERS   ::
// + node        : Node* : Pointer to the node containing the list
// + list        : List* : Pointer to the list
// + byteSkip    : int   : How many bytes skip to get the key item
//                         from the listItem.
// + key         : char* : The key that an item is idendified.
// + size        : int   : Size of the key element in byte
// RETURN       :: void* : Item found, NULL if not found
// **/
void* FindItem(Node *node,
               LinkedList *list,
               int byteSkip,
               char* key,
               int size);

// /**
// FUNCTION     :: FindItem
// LAYER        :: ANY LAYER
// PURPOSE      :: Find an item from the list
// PARAMETERS   ::
// + node        : Node* : Pointer to the node containing the list
// + list        : List* : Pointer to the list
// + byteSkip    : int   : How many bytes skip to get the key item
//                         from the listItem.
// + key         : char* : The key that an item is idendified.
// + size        : int   : Size of the key element in byte
// RETURN       :: void* : Item found, NULL if not found
// **/
void* FindItem(Node *node,
               LinkedList *list,
               int byteSkip,
               char* key,
               int size,
               ListItem** item);

// /**
// FUNCTION     :: ListGet
// LAYER        :: ANY LAYER
// PURPOSE      :: Remove an item from the list
// PARAMETERS   ::
// + node        : Node* : Pointer to the node containing the list
// + list        : List* : Pointer to the list to remove item from
// + listItem    : ListItem* : item to be removed
// + freeItem    : BOOL : Whether to free the item
// + isMsg       : BOOL : Whether is this item a message? If it is
//                 message, we need to call MESSAGE_Free for it.
// RETURN       :: void : NULL
// **/
void ListGet(Node *node,
             LinkedList *list,
             ListItem *listItem,
             BOOL freeItem,
             BOOL isMsg);

// /**
// FUNCTION     :: ListFree
// LAYER        :: ANY LAYER
// PURPOSE      :: Free the entire list
// PARAMETERS   ::
// + node        : Node* : Pointer to the node containing the list
// + list        : List* : Pointer to the list to be freed
// + isMsg       : BOOL : Does the list contain Messages? If so, we
//                 need to call MESSAGE_Free.
// RETURN       :: void : NULL
// **/
void ListFree(Node *node, LinkedList *list, BOOL isMsg);

//
//
//
//       DEFINES A LIST THAT HOLDS INTEGERS
//
//
///////////////////////////////////////////////////////////////////

typedef tListItem<int> IntListItem;

// /**
// STRUCT      :: IntList
// DESCRIPTION :: A list that stores integers.
// **/
typedef struct IntList_
{
    int size;
    IntListItem *first;       // First item in list.
    IntListItem *last;        // Last item in list.
} IntList;

// /**
// FUNCTION     :: IntListInit
// LAYER        :: ANY LAYER
// PURPOSE      :: Initialize the list
// PARAMETERS   ::
// + node        : Node* : Node that contains the list
// + list        : IntList**: Pointer to list pointer
// RETURN       :: void : NULL
// **/
void IntListInit(Node *node, IntList **list);

// /**
// FUNCTION     :: IntListIsEmpty
// LAYER        :: ANY LAYER
// PURPOSE      :: Check if list is empty
// PARAMETERS   ::
// + node        : Node* : Node that contains the list
// + list        : IntList* : Pointer to the list
// RETURN       :: BOOL : If empty, TRUE, non-empty, FALSE
// **/
BOOL IntListIsEmpty(Node *node, IntList *list);

// /**
// FUNCTION     :: IntListGetSize
// LAYER        :: ANY LAYER
// PURPOSE      :: Get the size of the list
// PARAMETERS   ::
// + node        : Node* : Pointer to the node containing the list
// + list        : IntList* : Pointer to the list
// RETURN       :: int : Size of the list
// **/
int IntListGetSize(Node *node, IntList *list);

// /**
// FUNCTION     :: ListInsert
// LAYER        :: ANY LAYER
// PURPOSE      :: Insert an item at the end of the list
// PARAMETERS   ::
// + node        : Node* : Pointer to the node containing the list
// + list        : List* : Pointer to the list
// + timeStamp   : clocktype : Time the item was last inserted.
// + data        : void* : item to be inserted
// RETURN       :: void : NULL
// **/
void IntListInsert(Node *node,
                IntList *list,
                clocktype timeStamp,
                int data);

// /**
// FUNCTION     :: IntListGet
// LAYER        :: ANY LAYER
// PURPOSE      :: Remove an item from the list
// PARAMETERS   ::
// + node        : Node* : Pointer to the node containing the list
// + list        : IntList* : Pointer to the list to remove item from
// + listItem    : IntListItem* : item to be removed
// + freeItem    : BOOL : Whether to free the item
// + isMsg       : BOOL : Whether is this item a message? If it is
//                 message, we need to call MESSAGE_Free for it.
// RETURN       :: void : NULL
// **/
void IntListGet(Node *node,
             IntList *list,
             IntListItem *listItem,
             BOOL freeItem,
             BOOL isMsg);

// /**
// FUNCTION     :: IntListFree
// LAYER        :: ANY LAYER
// PURPOSE      :: Free the entire list
// PARAMETERS   ::
// + node        : Node* : Pointer to the node containing the list
// + list        : IntList* : Pointer to the list to be freed
// + isMsg       : BOOL : Does the list contain Messages? If so, we
//                 need to call MESSAGE_Free.
// RETURN       :: void : NULL
// **/
void IntListFree(Node *node, IntList *list, BOOL isMsg);

#endif // _QUALNET_LIST_H_
