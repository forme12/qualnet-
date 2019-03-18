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

#include <iostream>
using namespace std;
#include <cstdlib>
#include <cassert>

#include "hla_HashBucket.h"

//-----------------------------------------------------------------------------
// Public functions
//-----------------------------------------------------------------------------

HlaHashBucket::HlaHashBucket()
    : m_items(NULL),
      m_numItems(0),
      m_numItemsActive(0),
      m_keySize(0),
      m_dataSize(0)
{
}

HlaHashBucket::~HlaHashBucket()
{
    if (m_items)
    {
        delete [] m_items;
        m_items = NULL;
    }
}

HlaHashBucket::HlaHashBucket(const HlaHashBucket& src)
{
    ReportProgramErrorAndExit(__FILE__, __LINE__,
        "Copy constructor not supported");
}

HlaHashBucket&
HlaHashBucket::operator=(const HlaHashBucket& src)
{
    ReportProgramErrorAndExit(__FILE__, __LINE__,
        "Assignment operator not supported");

    return *this;  // This statement should not be reached.
}

void
HlaHashBucket::Init(
    unsigned numItems,
    unsigned keySize,
    unsigned dataSize)
{
    assert(m_items == NULL);
    assert(m_numItems == 0);
    assert(m_numItemsActive == 0);

    assert(numItems > 0);

    m_numItems = numItems;
    m_keySize  = keySize;
    m_dataSize = dataSize;

    InitItemsArray(m_items, m_numItems, m_keySize, m_dataSize);
}

HlaHashItem&
HlaHashBucket::ActivateItem(const void* key)
{
    HlaHashItem& item = GetInactiveItem();

    item.Activate();
    m_numItemsActive++;

    item.SetKey(key);

    return item;
}

HlaHashItem&
HlaHashBucket::GetItem(unsigned index) const
{
    assert(m_items != NULL);
    assert(index < m_numItems);

    return m_items[index];
}

HlaHashItem*
HlaHashBucket::GetItem(
    const void* key,
    bool (*areKeysEqualFcn)(const void* key1, const void* key2)) const
{
    if (m_items == NULL || m_numItems == 0 || m_numItemsActive == 0)
    {
        return NULL;
    }

    unsigned i;
    unsigned numItemsActiveFound = 0;
    for (i = 0; i < m_numItems && numItemsActiveFound < m_numItemsActive; i++)
    {
        if (m_items[i].IsActive())
        {
            numItemsActiveFound++;

            if (areKeysEqualFcn(key, m_items[i].GetKeyPtr()))
            {
                return &m_items[i];
            }
        }
    }

    return NULL;
}

void
HlaHashBucket::DeactivateItem(
    const void* key,
    bool (*areKeysEqualFcn)(const void* key1, const void* key2))
{
    HlaHashItem* hashItemPtr = GetItem(key, areKeysEqualFcn);
    assert(hashItemPtr != NULL);

    HlaHashItem& hashItem = *hashItemPtr;

    hashItem.Deactivate();
    m_numItemsActive--;
}

unsigned
HlaHashBucket::GetNumItems() const
{
    return m_numItems;
}

//-----------------------------------------------------------------------------
// Private functions
//-----------------------------------------------------------------------------

void
HlaHashBucket::InitItemsArray(
    HlaHashItem*& items,
    unsigned numItems,
    unsigned keySize,
    unsigned dataSize)
{
    items = new HlaHashItem [numItems];
    assert(items);

    unsigned i;
    for (i = 0; i < numItems; i++)
    {
        items[i].Init(keySize, dataSize);
    }
}

HlaHashItem&
HlaHashBucket::GetInactiveItem()
{
    assert(m_items != NULL);
    assert(m_numItems > 0);
    assert(m_numItemsActive <= m_numItems);

    if (m_numItemsActive == m_numItems)
    {
        DoubleNumItems();
        return m_items[m_numItemsActive];
    }

    unsigned i;
    for (i = 0; i < m_numItems; i++)
    {
        if (!m_items[i].IsActive())
        {
            return m_items[i];
        }
    }

    ReportProgramErrorAndExit(__FILE__, __LINE__,
        "Statement should not be reached");
    exit(1);  // Needed to prevent VC compiler warning.
}

void
HlaHashBucket::DoubleNumItems()
{
    assert(m_items != NULL);
    assert(m_numItems > 0);

    unsigned newNumItems = m_numItems * 2;

    HlaHashItem* newItems;
    InitItemsArray(newItems, newNumItems, m_keySize, m_dataSize);

    unsigned i;
    for (i = 0; i < m_numItems; i++)
    {
        newItems[i] = m_items[i];
    }

    delete [] m_items;
    m_items = NULL;

    m_items    = newItems;
    m_numItems = newNumItems;
}

void
HlaHashBucket::ReportProgramErrorAndExit(
    const char* filename,
    unsigned    lineNumber,
    const char* errorString)
{
    cerr << "Program error" << endl
         << endl
         << filename << ":" << lineNumber << ":" << errorString << endl;
    exit(1);
}
