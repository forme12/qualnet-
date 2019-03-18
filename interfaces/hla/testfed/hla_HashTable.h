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

#ifndef HLA_HASHTABLE_H
#define HLA_HASHTABLE_H

// This class implements a hash table with chaining upon collision.
// (http://en2.wikipedia.org/wiki/Hash_table)
//
// The hash table contains buckets.  Each bucket contains items.  Each item
// contains two buffers, storing a key and a value.  Keys and values can be
// represented in any way.  The buffers are accessed via void pointers, and
// are also suitably aligned for storage of any type of object.
//
// The size of the buffer for each key is the same for all items.
// The size of the buffer for each value is the same for all items.
//
// On initializing the hash table, the user provides a hash function (which
// takes a key and outputs a value between [0, number of buckets - 1]), and a
// function which compares keys for equality.
//
// The user also provides the number of buckets in the table, and the initial
// number of items for each bucket.  When a bucket is filled, the bucket will
// be doubled in size.
//
// The user can add duplicate keys; however, on a call to retrieve or remove
// a key for which there is a duplicate, there is no guaranteed order on
// which key will be acted on.
//
// All memory is allocated at initialization (this trades off memory usage for
// run-time speed), except when a bucket expands.
//
// The user is expected to not overflow the char buffers.  This trades off
// development effort and memory efficiency for run-time speed.

#include "hla_HashBucket.h"

class HlaHashTable
{
public:
    HlaHashBucket* m_buckets;

    HlaHashTable();
    ~HlaHashTable();

    HlaHashTable(const HlaHashTable& src);

    HlaHashTable&
    operator=(const HlaHashTable& src);

    void
    Init(
        unsigned numBuckets,
        unsigned numItemsPerBucket,
        unsigned keySize,
        unsigned dataSize,
        unsigned (*hashFcn)(const void* key, unsigned numBuckets),
        bool (*areKeysEqualFcn)(const void* key1, const void* key2));

    HlaHashItem&
    AddItem(const void* key);

    HlaHashItem*
    GetItem(const void* key) const;

    void
    RemoveItem(const void* key);

    HlaHashBucket&
    GetBucket(unsigned index) const;

    unsigned
    GetNumBuckets() const;

    unsigned
    GetKeySize() const;

    unsigned
    GetDataSize() const;

private:
    unsigned m_numBuckets;
    unsigned m_keySize;
    unsigned m_dataSize;

    unsigned (*m_hashFcn)(const void* key, unsigned numBuckets);
    bool (*m_areKeysEqualFcn)(const void* key1, const void* key2);

    void
    ReportProgramErrorAndExit(
        const char* filename,
        unsigned    lineNumber,
        const char* errorString);
};

#endif /* HLA_HASHTABLE_H */
