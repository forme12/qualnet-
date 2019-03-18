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

#if !defined(__I386_ATOMIC_H__)
#define __I386_ATOMIC_H__

#define UTIL_AtomicValue(x) { (x) }

typedef struct { volatile int value; } UTIL_AtomicInteger;
typedef struct { volatile float value; } UTIL_AtomicReal;

void UTIL_AtomicSet(UTIL_AtomicInteger *v, int x);

int UTIL_AtomicRead(UTIL_AtomicInteger *v);

void UTIL_AtomicIncrement(UTIL_AtomicInteger *v);

void UTIL_AtomicDecrement(UTIL_AtomicInteger *v);

void UTIL_AtomicAdd(UTIL_AtomicInteger *v, int i);

void UTIL_AtomicSubtract(UTIL_AtomicInteger *v, int i);

int UTIL_AtomicDecrementAndTest(UTIL_AtomicInteger *v);

#endif // __I386_ATOMIC_H__
