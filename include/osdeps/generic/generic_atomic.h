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

#if !defined(__GENERIC_ATOMIC_H__)
#define __GENERIC_ATOMIC_H__

#warning "These atomic operations are not atomic!"

typedef struct
{ 
    volatile int value; 
} UTIL_AtomicInteger;

#define UTIL_AtomicValue(x) { (x) }

static void UTIL_AtomicSet(UTIL_AtomicInteger *v, int x)
{
    v->value = (x); 
}
static int UTIL_AtomicRead(UTIL_AtomicInteger *v)
{
    int val =  v->value; 
    return val;
}

static void UTIL_AtomicAdd(UTIL_AtomicInteger *v, int i)
{
    v->value += i; 
}
static void UTIL_AtomicSubtract(UTIL_AtomicInteger *v, int i)
{
    v->value -= i; 
}

static void UTIL_AtomicIncrement(UTIL_AtomicInteger *v)
{ UTIL_AtomicAdd(v, 1); }

static void UTIL_AtomicDecrement(UTIL_AtomicInteger *v)
{ UTIL_AtomicAdd(v, -1); }

static int UTIL_AtomicDecrementAndTest(UTIL_AtomicInteger *v)
{
    int tmp;

    tmp = --v->value;

    return tmp == 0 ? 1 : 0;
}

#endif // __GENERIC_ATOMIC_H__
