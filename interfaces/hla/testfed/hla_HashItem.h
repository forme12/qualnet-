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

#ifndef HLA_HASHITEM_H
#define HLA_HASHITEM_H

#include <cassert>

class HlaHashItem
{
public:
    void* m_keyPtr;
    void* m_dataPtr;

    HlaHashItem();
    ~HlaHashItem();

    HlaHashItem(const HlaHashItem& src);

    HlaHashItem&
    operator=(const HlaHashItem& src);

    void
    Init(unsigned keySize, unsigned dataSize);

    bool
    IsActive() const;

    void
    Activate();

    void
    Deactivate();

    void*
    GetKeyPtr() const;

    void*
    GetDataPtr() const;

    void
    SetKey(const void* key);

private:
    bool m_active;
    unsigned m_keySize;
    unsigned m_dataSize;

    void
    ReportProgramErrorAndExit(
        const char* filename,
        unsigned    lineNumber,
        const char* errorString);
};

#endif /* HLA_HASHITEM_H */
