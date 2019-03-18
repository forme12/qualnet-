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
#include <cstring>
#include <cassert>

#include "hla_HashItem.h"

//-----------------------------------------------------------------------------
// Public functions
//-----------------------------------------------------------------------------

HlaHashItem::HlaHashItem()
    : m_keyPtr(NULL), m_dataPtr(NULL), m_active(false), m_keySize(0), m_dataSize(0)
{
}

HlaHashItem::~HlaHashItem()
{
    if (m_keyPtr)
    {
        delete [] (unsigned char*) m_keyPtr;
        m_keyPtr = NULL;
    }

    if (m_dataPtr)
    {
        delete [] (unsigned char*) m_dataPtr;
        m_dataPtr = NULL;
    }
}

HlaHashItem::HlaHashItem(const HlaHashItem& src)
{
    ReportProgramErrorAndExit(__FILE__, __LINE__,
        "Copy constructor not supported");
}

HlaHashItem&
HlaHashItem::operator=(const HlaHashItem& src)
{
    assert(m_keyPtr != NULL);
    assert(m_dataPtr != NULL);
    assert(m_keySize > 0);
    assert(m_dataSize > 0);
    assert(m_keySize == src.m_keySize);
    assert(m_dataSize == src.m_dataSize);

    if (src.IsActive())
    {
        if (!IsActive()) { Activate(); }
        memcpy(m_keyPtr, src.m_keyPtr, m_keySize);
        memcpy(m_dataPtr, src.m_dataPtr, m_dataSize);
    }
    else
    {
        if (IsActive()) { Deactivate(); }
    }

    return *this;
}

void
HlaHashItem::Init(unsigned keySize, unsigned dataSize)
{
    // Allocate dataSize bytes of memory for the m_dataPtr variable.
    // Note that m_active is not set to true here.

    assert(m_dataPtr == NULL);
    assert(keySize > 0);
    assert(dataSize > 0);

    m_keySize = keySize;
    m_dataSize = dataSize;

    m_keyPtr = new unsigned char [m_keySize];
    assert(m_keyPtr);

    m_dataPtr = new unsigned char [m_dataSize];
    assert(m_dataPtr);
}

bool
HlaHashItem::IsActive() const
{
    return m_active;
}

void
HlaHashItem::Activate()
{
    // Note that this function does not allocate memory.

    assert(!m_active);
    m_active = true;
}

void
HlaHashItem::Deactivate()
{
    // Note that this function does not free memory.

    assert(m_active);
    m_active = false;
}

void*
HlaHashItem::GetKeyPtr() const
{
    assert(m_keyPtr != NULL);
    return m_keyPtr;
}

void*
HlaHashItem::GetDataPtr() const
{
    assert(m_dataPtr != NULL);
    return m_dataPtr;
}

void
HlaHashItem::SetKey(const void* key)
{
    assert(m_keyPtr != NULL);
    assert(m_keySize > 0);
    memcpy(m_keyPtr, key, m_keySize);
}

//-----------------------------------------------------------------------------
// Private functions
//-----------------------------------------------------------------------------

void
HlaHashItem::ReportProgramErrorAndExit(
    const char* filename,
    unsigned    lineNumber,
    const char* errorString)
{
    cerr << "Program error" << endl
         << endl
         << filename << ":" << lineNumber << ":" << errorString << endl;
    exit(1);
}
