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

#ifndef HLA_HASHFCNS_H
#define HLA_HASHFCNS_H

unsigned
HlaRadioKeyHashFcn(const void* key, unsigned numBuckets);

bool
HlaRadioKeysEqualFcn(const void* key1, const void* key2);

unsigned
HlaEntityKeyHashFcn(const void* key, unsigned numBuckets);

bool
HlaEntityKeysEqualFcn(const void* key1, const void* key2);

unsigned
HlaUnsignedKeyHashFcn(const void* keyPtr, unsigned numBuckets);

bool
HlaUnsignedKeysEqualFcn(const void* keyPtr1, const void* keyPtr2);

unsigned
HlaStringKeyHashFcn(const void* keyPtr, unsigned numBuckets);

bool
HlaStringKeysEqualFcn(const void* keyPtr1, const void* keyPtr2);

unsigned
HlaNonNegativeIntKeyHashFcn(const void* key, unsigned numBuckets);

bool
HlaIntKeysEqualFcn(const void* key1, const void* key2);

#endif /* HLA_HASHFCNS_H */
