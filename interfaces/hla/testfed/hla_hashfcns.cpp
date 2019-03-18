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
#include <cstring>
#include <cassert>

#include "hla_HashTable.h"
#include "hla_rpr_fom.h"
#include "hla_shared.h"
#include "hla_hashfcns.h"

unsigned
HlaRadioKeyHashFcn(const void* key, unsigned numBuckets)
{
    assert(key != NULL);
    assert(numBuckets > 0);

    const HlaRadioKey& radioKey = *((const HlaRadioKey*) key);

    return HlaStringKeyHashFcn(&radioKey.markingData, numBuckets);
}

bool
HlaRadioKeysEqualFcn(const void* key1, const void* key2)
{
    assert(key1 != NULL);
    assert(key2 != NULL);

    const HlaRadioKey& radioKey1 = *((const HlaRadioKey*) key1);
    const HlaRadioKey& radioKey2 = *((const HlaRadioKey*) key2);

    return strcmp(radioKey1.markingData, radioKey2.markingData) == 0
           && radioKey1.radioIndex == radioKey2.radioIndex;
}

unsigned
HlaEntityKeyHashFcn(const void* key, unsigned numBuckets)
{
    assert(key != NULL);
    assert(numBuckets > 0);

    const HlaEntityId& entityKey = *((const HlaEntityId*) key);

    return entityKey.entityNumber % numBuckets;
}

bool
HlaEntityKeysEqualFcn(const void* key1, const void* key2)
{
    assert(key1 != NULL);
    assert(key2 != NULL);

    const HlaEntityId& entityKey1 = *((const HlaEntityId*) key1);
    const HlaEntityId& entityKey2 = *((const HlaEntityId*) key2);

    return entityKey1 == entityKey2;
}

unsigned
HlaUnsignedKeyHashFcn(const void* keyPtr, unsigned numBuckets)
{
    assert(keyPtr != NULL);
    assert(numBuckets > 0);

    unsigned key = *((unsigned*) keyPtr);

    return key % numBuckets;
}

bool
HlaUnsignedKeysEqualFcn(const void* keyPtr1, const void* keyPtr2)
{
    assert(keyPtr1 != NULL);
    assert(keyPtr2 != NULL);

    unsigned key1 = *((unsigned*) keyPtr1);
    unsigned key2 = *((unsigned*) keyPtr2);

    return key1 == key2;
}

unsigned
HlaStringKeyHashFcn(const void* keyPtr, unsigned numBuckets)
{
    assert(keyPtr != NULL);
    assert(numBuckets > 0);

    unsigned value = 5381;
    unsigned charValue;
    const char* charPtr = (const char*) keyPtr;

    while ((charValue = (unsigned) *charPtr) != 0)
    {
        value = ((value << 5) + value) + charValue;  // value * 33 + charValue
        charPtr++;
    }

    return value % numBuckets;
}

bool
HlaStringKeysEqualFcn(const void* keyPtr1, const void* keyPtr2)
{
    assert(keyPtr1 != NULL);
    assert(keyPtr2 != NULL);

    const char* key1 = (const char*) keyPtr1;
    const char* key2 = (const char*) keyPtr2;

    return strcmp(key1, key2) == 0;
}

unsigned
HlaNonNegativeIntKeyHashFcn(const void* key, unsigned numBuckets)
{
    assert(key != NULL);
    assert(numBuckets > 0);

    int int_key = *((int*) key);
    assert(int_key >= 0);

    unsigned u_key = (unsigned) int_key;

    return u_key % numBuckets;
}

bool
HlaIntKeysEqualFcn(const void* key1, const void* key2)
{
    assert(key1 != NULL);
    assert(key2 != NULL);

    int int_key1 = *((int*) key1);
    int int_key2 = *((int*) key2);

    return (int_key1 == int_key2);
}
