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

#include "hla_HashTable.h"

//-----------------------------------------------------------------------------
// Public functions
//-----------------------------------------------------------------------------

HlaHashTable::HlaHashTable()
    : m_buckets(NULL),
      m_numBuckets(0),
      m_hashFcn(NULL),
      m_areKeysEqualFcn(NULL)
{
}

HlaHashTable::~HlaHashTable()
{
    if (m_buckets)
    {
        delete [] m_buckets;
        m_buckets = NULL;
    }
}

HlaHashTable::HlaHashTable(const HlaHashTable& src)
{
    ReportProgramErrorAndExit(__FILE__, __LINE__,
        "Copy constructor not supported");
}

HlaHashTable&
HlaHashTable::operator=(const HlaHashTable& src)
{
    ReportProgramErrorAndExit(__FILE__, __LINE__,
        "Assignment operator not supported");

    // (Prevent warning with gcc 3.0.4.)
    // Line below is never reached.

    return *this;
}

void
HlaHashTable::Init(
    unsigned numBuckets,
    unsigned numItemsPerBucket,
    unsigned keySize,
    unsigned dataSize,
    unsigned (*hashFcn)(const void* key, unsigned numBuckets),
    bool (*areKeysEqualFcn)(const void* key1, const void* key2))
{
    assert(m_buckets == NULL);
    assert(m_numBuckets == 0);

    assert(numBuckets > 0);

    m_numBuckets = numBuckets;
    m_keySize    = keySize;
    m_dataSize   = dataSize;

    m_buckets = new HlaHashBucket [m_numBuckets];
    assert(m_buckets);

    unsigned i;
    for (i = 0; i < m_numBuckets; i++)
    {
        m_buckets[i].Init(numItemsPerBucket, m_keySize, m_dataSize);
    }

    m_hashFcn = hashFcn;
    m_areKeysEqualFcn = areKeysEqualFcn;
}

HlaHashItem&
HlaHashTable::AddItem(const void* key)
{
    assert(m_hashFcn != NULL);
    assert(m_numBuckets > 0);

    unsigned hashValue = m_hashFcn(key, m_numBuckets);
    assert(hashValue < m_numBuckets);

    HlaHashBucket& bucket = GetBucket(hashValue);

    return bucket.ActivateItem(key);
}

HlaHashItem*
HlaHashTable::GetItem(const void* key) const
{
    assert(m_hashFcn != NULL);
    assert(m_numBuckets > 0);

    unsigned hashValue = m_hashFcn(key, m_numBuckets);
    assert(hashValue < m_numBuckets);

    HlaHashBucket& bucket = GetBucket(hashValue);
    return bucket.GetItem(key, m_areKeysEqualFcn);
}

void
HlaHashTable::RemoveItem(const void* key)
{
    assert(m_hashFcn != NULL);
    assert(m_numBuckets > 0);

    unsigned hashValue = m_hashFcn(key, m_numBuckets);
    assert(hashValue < m_numBuckets);

    HlaHashBucket& bucket = GetBucket(hashValue);
    bucket.DeactivateItem(key, m_areKeysEqualFcn);
}

HlaHashBucket&
HlaHashTable::GetBucket(unsigned index) const
{
    assert(m_buckets != NULL);
    assert(index < m_numBuckets);

    return m_buckets[index];
}

unsigned
HlaHashTable::GetNumBuckets() const
{
    return m_numBuckets;
}

unsigned
HlaHashTable::GetKeySize() const
{
    return m_keySize;
}

unsigned
HlaHashTable::GetDataSize() const
{
    return m_dataSize;
}

//-----------------------------------------------------------------------------
// Private functions
//-----------------------------------------------------------------------------

void
HlaHashTable::ReportProgramErrorAndExit(
    const char* filename,
    unsigned    lineNumber,
    const char* errorString)
{
    cerr << "Program error" << endl
         << endl
         << filename << ":" << lineNumber << ":" << errorString << endl;
    exit(1);
}
