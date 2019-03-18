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

#ifndef HLA_HASHBUCKET_H
#define HLA_HASHBUCKET_H

#include "hla_HashItem.h"

class HlaHashBucket
{
public:
    HlaHashItem* m_items;

    HlaHashBucket();
    ~HlaHashBucket();

    HlaHashBucket(const HlaHashBucket& src);

    HlaHashBucket&
    operator=(const HlaHashBucket& src);

    void
    Init(
        unsigned numItems,
        unsigned keySize,
        unsigned dataSize);

    HlaHashItem&
    ActivateItem(const void* key);

    HlaHashItem&
    GetItem(unsigned index) const;

    HlaHashItem*
    GetItem(
        const void* key,
        bool (*areKeysEqualFcn)(const void* key1, const void* key2)) const;

    void
    DeactivateItem(
        const void* key,
        bool (*areKeysEqualFcn)(const void* key1, const void* key2));

    unsigned
    GetNumItems() const;

private:
    unsigned m_numItems;
    unsigned m_numItemsActive;
    unsigned m_keySize;
    unsigned m_dataSize;

    void
    InitItemsArray(
        HlaHashItem*& items,
        unsigned numItems,
        unsigned keySize,
        unsigned dataSize);

    HlaHashItem&
    GetInactiveItem();

    void
    DoubleNumItems();

    void
    ReportProgramErrorAndExit(
        const char* filename,
        unsigned    lineNumber,
        const char* errorString);
};

#endif /* HLA_HASHBUCKET_H */
