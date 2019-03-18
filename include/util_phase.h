// Copyright (c) 2001-2009, Scalable Network Technologies, Inc.  
// All Rights Reserved.
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
//
#ifndef __UTIL_PHASE_H__
# define __UTIL_PHASE_H__

#define SYNCHRONIZE_SIMULATION_PHASES (true)

namespace UTIL 
{
    char static *PhaseNames[] = 
    {
        "Null", "Kernel Init", "User Init", 
        "Simulation", "User Epoch", "Kernel Epoch",
        "Exit"
    };

    static int const PHASE_NULL = 0;
    static int const PHASE_KERNEL_INIT = 1;
    static int const PHASE_USER_INIT = 2;
    static int const PHASE_SIMULATION = 3;
    static int const PHASE_USER_EPOCH = 4;
    static int const PHASE_KERNEL_EPOCH = 5;
    static int const PHASE_EXIT = 6;

    static int const PHASE_COUNT = 7;

    class Phase 
    {
        Phase() { }
    public:
        static void initialize(int count);
        static void finalize();
        static void mark(int phase);
    } ;
        
}

#endif /* __UTIL_PHASE_H__ */
