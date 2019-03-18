// Copyright (c) 2001-2010, Scalable Network Technologies, Inc.  All Rights Reserved.
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

#ifndef _SIMULATION_CONTEXT_H_
#define _SIMULATION_CONTEXT_H_

/// This class provides a way of determining the simulation context,
/// i.e. which node on which partition is running.  These functions use
/// thread local data, so are less efficient than accessing a local node
/// pointer, if one is available.  The functions should not be used by
/// worker threads, such as those running on external interfaces.
class SimContext
{
public:
    /// Sets the current node.
    static void setCurrentNode(Node*);

    /// Unsets the current node.
    static void unsetCurrentNode();

    /// Returns a pointer to the current node, if any.
    static Node* getCurrentNode();

    /// Methods useful for querying, storing, and restoring the current Node/Interface context
    static void getCurrentContext(Node *& currentContextNode, int& currentContextInterface);
    static void saveCurrentContext(Node *& storedContextNode, int& storedContextInterface);
    static void restoreContext(Node *& storedContextNode, int& storedContextInterface);

    /// Sets the current partition.  Only called in the kernel
    static void setPartition(PartitionData*);

    /// Returns a pointer to the current partition.
    static PartitionData* getPartition();

    /// This is not fully implemented, so should not be trusted for all models.
    /// The corresponding set/unset functions are defined in Node.
    static int getCurrentInterface();
};

#endif /* _SIMULATION_CONTEXT_H_ */
