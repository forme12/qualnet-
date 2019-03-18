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

// /**
// PACKAGE :: MOBILITY
// DESCRIPTION ::
//      This file describes data structures and functions used by mobility models.
// **/

#ifndef MOBILITY_H
#define MOBILITY_H

#include "propagation.h"

typedef Coordinates Velocity;
// /**
// CONSTANT :: DEFAULT_DISTANCE_GRANULARITY : 1
// DESCRIPTION :: Defines the default distance granurality
// **/
#define DEFAULT_DISTANCE_GRANULARITY 1

// /**
// CONSTANT :: NUM_NODE_PLACEMENT_TYPES : 7
// DESCRIPTION :: Defines the number of node placement schemes
// **/
#define NUM_NODE_PLACEMENT_TYPES 8

// /**
// CONSTANT :: NUM_MOBILITY_TYPES : 5
// DESCRIPTION :: Defines the number of mobility models
// **/
#define NUM_MOBILITY_TYPES       5

// /**
// ENUM        :: NodePlacementType
// DESCRIPTION :: Specifies different node placement schemes
// **/
enum NodePlacementType {
    RANDOM_PLACEMENT = 0,
    UNIFORM_PLACEMENT,
    GRID_PLACEMENT,
    FILE_BASED_PLACEMENT,
    GROUP_PLACEMENT,
    GAUSSIAN_PLACEMENT,
    EXTERNAL_PLACEMENT
};

// /**
// ENUM        :: MobilityType
// DESCRIPTION :: Specifies different mobility models
// **/
enum MobilityType {
    NO_MOBILITY = 0,
    RANDOM_WAYPOINT_MOBILITY,
    FILE_BASED_MOBILITY,
    GROUP_MOBILITY
};

// /**
// STRUCT :: MobilityHeap
// DESCRIPTION :: A Heap that determines the earliest time
// **/
struct MobilityHeap {
    clocktype minTime;
    Node **heapNodePtr;
    int heapSize;
    int length;
};


// /**
// API :: MOBILITY_InsertEvent
// PURPOSE :: Inserts an event.
// PARAMETERS ::
// + heapPtr : MobilityHeap* : A pointer of type MobilityHeap.
// + node    : Node*         : A pointer to node.
// RETURN :: void :
// **/
void MOBILITY_InsertEvent(MobilityHeap *heapPtr, Node *node);


// /**
// API :: MOBILITY_DeleteEvent
// PURPOSE :: Deletes an event.
// PARAMETERS ::
// + heapPtr : MobilityHeap* : A pointer of type MobilityHeap.
// + node    : Node*         : A pointer to node.
// RETURN :: void :
// **/
void MOBILITY_DeleteEvent(MobilityHeap *heapPtr, Node *node);


// /**
// API :: MOBILITY_HeapFixDownEvent
// PURPOSE :: Inserts an event and sort out the heap downwards
// PARAMETERS ::
// + heapPtr : MobilityHeap* : A pointer of type MobilityHeap.
// + i       : int : index
// RETURN :: void :
// **/
void MOBILITY_HeapFixDownEvent(MobilityHeap *heapPtr, int i);


// /**
// STRUCT :: MobilityElement
// DESCRIPTION ::
//      Defines all the element of mobility model.
// **/
struct MobilityElement {
    int               sequenceNum;
    clocktype         time;
    Coordinates       position;
    Orientation       orientation;
    double            speed;
    double            zValue;
    BOOL              movingToGround;
};

// /**
// STRUCT :: MobilityRemainder
// DESCRIPTION ::
//      A structure that defines the next states of the elements of
//      mobility model.
// **/
struct MobilityRemainder {
    clocktype   nextMoveTime;
    Coordinates nextPosition;
    Orientation nextOrientation;
    double      speed;
    int         numMovesToNextDest;
    int         destCounter;
    clocktype   moveInterval;
    Coordinates delta;
    BOOL        movingToGround;
};

// /**
// CONSTANT :: NUM_PAST_MOBILITY_EVENTS : 2
// DESCRIPTION :: Number of past mobility models stored
// **/
#define NUM_PAST_MOBILITY_EVENTS 2


// /**
// STRUCT :: MobilityData
// DESCRIPTION ::
//      This structure keeps the data related to mobility model.
//      It also holds the variables which are
//      static and variable during the simulation. Buffer caches future
//      position updates as well.
// **/
struct MobilityData {
    // static during the simulation
    MobilityType mobilityType;
    D_Float32 distanceGranularity;
    D_BOOL groundNode;

    // variable during the simulation
    RandomSeed        seed;

#ifdef ADDON_NGCNMS
    RandomSeed        groupSeed;
    int               groupIndex;
    Coordinates groupTerrainOrigin;
    Coordinates groupTerrainDimensions;

    // Mobility parameters
    double groupMaxSpeed;
    double groupMinSpeed;
    double internalMaxSpeed;
    double internalMinSpeed;
    clocktype internalMobilityPause;
#endif /* ADDON_NGCNMS */

    int               sequenceNum;
    MobilityElement*  next;
    MobilityElement*  current;
    MobilityElement*  past[NUM_PAST_MOBILITY_EVENTS];

    int               numDests;
    MobilityElement*  destArray;

    MobilityRemainder remainder;

    Coordinates       lastExternalCoordinates;
    clocktype         lastExternalTrueMobilityTime;
    clocktype         lastExternalMobilityTime;
    Velocity          lastExternalVelocity;
    double            lastExternalSpeed;

    //Urban
    bool              indoors;
    //endUrban

    void *mobilityVar;
};

// /**
// FUNCTION :: MOBILITY_AllocateNodePositions
// PURPOSE :: Allocates memory for nodePositions and mobilityData
//            Note: This function is called before NODE_CreateNode().
//                  It cannot access Node structure
// PARAMETERS ::
// + numNodes                : int : number of nodes
// + nodeIdArray             : NodeAddress* : array of nodeId
// + nodePositions           : NodePositions** : pointer to the array
//                                               to be allocated.
// + nodePlacementTypeCounts : int** : array of placement type counts
// + nodeInput               : NodeInput* : configuration input
// + seedVal                 : int : seed for random number seeds
// RETURN :: void :
// **/
void MOBILITY_AllocateNodePositions(
    int numNodes,
    NodeAddress* nodeIdArray,
    NodePositions** nodePositionsPtr,
    int** nodePlacementTypeCountsPtr,
    NodeInput* nodeInput,
    int seedVal);

// /**
// FUNCTION :: MOBILITY_PreInitialize
// PURPOSE :: Initializes most variables in mobilityData.
//            (Node positions are set in MOBILITY_SetNodePositions().)
//            Note: This function is called before NODE_CreateNode().
//                  It cannot access Node structure
// PARAMETERS ::
// + nodeId       : NodeAddress : nodeId
// + mobilityData : MobilityData* : mobilityData to be initialized
// + nodeInput    : NodeInput* : configuration input
// + seedVal      : int : seed for random number seeds
// RETURN :: void :
// **/
void MOBILITY_PreInitialize(
    NodeAddress nodeId,
    MobilityData* mobilityData,
    NodeInput* nodeInput,
    int seedVal);

// /**
// FUNCTION :: MOBILITY_PostInitialize
// PURPOSE :: Initializes variables in mobilityData not initialized by
//            MOBILITY_PreInitialize().
// PARAMETERS ::
// + node      : Node* : node being initialized
// + nodeInput : NodeInput* : structure containing contents of input file
// RETURN :: void :
// **/
void MOBILITY_PostInitialize(Node *node, NodeInput *nodeInput);

// /**
// API :: MOBILITY_UpdatePathProfiles
// PURPOSE :: Updates the path profiles.
// PARAMETERS ::
// + pathProfileHeap : MobilityHeap* : MobilityHeap structure.
// + nextEventTime   : clocktype     : Next event time.
// + upperBoundTime  : clocktype*    : Upper bound time.
// RETURN :: void :
// **/
void MOBILITY_UpdatePathProfiles(
    MobilityHeap *pathProfileHeap,
    clocktype nextEventTime,
    clocktype *upperBoundTime);


// /**
// API :: MOBILITY_Finalize
// PURPOSE :: Called at the end of simulation to collect the results of
//            the simulation of the mobility data.
// PARAMETERS ::
// + node : Node* : Node for which results are to be collected.
// RETURN :: void :
// **/
void MOBILITY_Finalize(Node* node);


// /**
// API :: MOBILITY_ProcessEvent
// PURPOSE :: Models the behaviour of the mobility models on receiving
//            a message.
// PARAMETERS ::
// + node : Node* : Node which received the message
// RETURN :: void :
// **/
void MOBILITY_ProcessEvent(Node* node);


// /**
// API :: MOBILITY_AddANewDestination
// PURPOSE :: Adds a new destination.
// PARAMETERS ::
// + mobilityData : MobilityData* : MobilityData of the node
// + arrivalTime  : clocktype     : Arrival time
// + dest         : Coordinates   : Destination
// + orientation  : Orientation   : Orientation
// + zValue       : double        : original zValue
// RETURN :: void :
// **/
void MOBILITY_AddANewDestination(
    MobilityData *mobilityData,
    clocktype arrivalTime,
    Coordinates dest,
    Orientation orientation,
    double zValue = 0.0);

// /**
// FUNCTION :: MOBILITY_NextPosition
// PURPOSE :: Update next node position for static mobility models
// PARAMETERS ::
// + node    : Node* : Node to be updated
// + element : MobilityElement* : next mobility update
// RETURN :: BOOL :
// **/
BOOL MOBILITY_NextPosition(
    Node* node,
    MobilityElement* element);

// /**
// API :: MOBILITY_NextMoveTime
// PURPOSE :: Determines the time of next movement.
// PARAMETERS ::
// + node     : Node*     : Pointer to node.
// RETURN :: clocktype : Next time of movement.
// **/
clocktype MOBILITY_NextMoveTime(const Node* node);


// /**
// API :: MOBILITY_ReturnMobilityElement
// PURPOSE :: Used to get the mobility element.
// PARAMETERS ::
// + node        : Node*             : Pointer to node.
// + sequenceNum : int               : Sequence number.
// RETURN :: MobilityElement* :
// **/
MobilityElement* MOBILITY_ReturnMobilityElement(
    Node* node,
    int sequenceNum);


// /**
// API :: MOBILITY_InsertANewEvent
// PURPOSE :: Inserts a new event.
// PARAMETERS ::
// + node         : Node*       : Pointer to node.
// + nextMoveTime : clocktype   : Time of next movement.
// + position     : Coordinates : Position of the node.
// + orientation  : Orientation : Node orientation.
// + speed        : double      : Speed of the node.
// RETURN :: void :
// **/
void MOBILITY_InsertANewEvent(
    Node* node,
    clocktype nextMoveTime,
    Coordinates position,
    Orientation orientation,
    double speed);


// /**
// API :: MOBILITY_NodeIsIndoors
// PURPOSE :: Returns whether the node is indoors.
// PARAMETERS ::
// + node     : Node*       : Pointer to node.
// RETURN :: bool : returns true if indoors.
// **/
bool MOBILITY_NodeIsIndoors(const Node* node);


// /**
// API :: MOBILITY_SetIndoors
// PURPOSE :: Sets the node's indoor variable.
// PARAMETERS ::
// + node     : Node*       : Pointer to node.
// + indoors  : bool        : true if the node is indoors.
// RETURN :: void :
// **/
void MOBILITY_SetIndoors(Node* node, bool indoors = true);


// /**
// API :: MOBILITY_ReturnCoordinates
// PURPOSE :: Returns the coordinate.
// PARAMETERS ::
// + node     : Node*       : Pointer to node.
// + position : Coordinates : Position of the node.
// RETURN :: void :
// **/
void MOBILITY_ReturnCoordinates(
    const Node* node,
    Coordinates* position);


// /**
// API :: MOBILITY_ReturnOrientation
// PURPOSE :: Returns the node orientation.
// PARAMETERS ::
// + node        : Node*        : Pointer to node.
// + orientation : Orientation* : Pointer to Orientation.
// RETURN :: void :
// **/
void MOBILITY_ReturnOrientation(
    const Node *node,
    Orientation *orientation);


// /**
// API :: MOBILITY_ReturnInstantaneousSpeed
// PURPOSE :: Returns instantaneous speed of a node.
// PARAMETERS ::
// + node     : Node*     : Pointer to node.
// + speed    : double*   : Speed of the node, double pointer.
// RETURN :: void :
// **/
void MOBILITY_ReturnInstantaneousSpeed(
    const Node *node,
    double *speed);


// /**
// API :: MOBILITY_ReturnSequenceNum
// PURPOSE :: Returns a sequence number for the current position.
// PARAMETERS ::
// + node        : Node* : Pointer to node.
// + sequenceNum : int*  : Sequence number.
// RETURN :: void :
// **/
void MOBILITY_ReturnSequenceNum(
    const Node* node,
    int* sequenceNum);


// /**
// API :: MOBILITY_SetNodePositions
// PURPOSE :: Set positions of nodes
// PARAMETERS ::
// + numNodes : int : Defines the number of nodes to be distributed.
// + nodePositions : NodePositions* : Pointer to NodePositionInfo. States
//                                    the node position information.
// + nodePlacementTypeCounts: int* : Array of placement type counts
// + terrainData : TerrainData* : Terrain data.
// + nodeInput : NodeInput* : Pointer to NodeInput, defines the
//                            node input structure.
// + seed : RandomSeed : Stores the seed value.
// + maxSimTime : clocktype : Maximum simulation time.
// RETURN :: void  :
// **/
void MOBILITY_SetNodePositions(
    int numNodes,
    NodePositions* nodePositions,
    int* nodePlacementTypeCounts,
    TerrainData* terrainData,
    NodeInput* nodeInput,
    RandomSeed seed,
    clocktype maxSimTime,
    clocktype startSimTime);


// /**
// API       :: MOBILITY_PostInitializePartition
// PURPOSE   :: Initialization of mobility models that most be done
//              after partition is created; MOBILITY_SetNodePositions
//              would be too early
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the partition data
// RETURN    :: void :
// **/
void MOBILITY_PostInitializePartition(PartitionData* partitionData);


// /**
// API       :: MOBILITY_NodePlacementFinalize
// PURPOSE   :: Finalize mobility models
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the partition data
// RETURN    :: void :
// **/
void MOBILITY_NodePlacementFinalize(PartitionData* partitionData);

// /**
// API :: MOBILITY_ChangeGroundNode
// PURPOSE :: Change GroundNode value..
// PARAMETERS ::
// + node        : Node* : Pointer to node being initialized.
// + before : BOOL   : Orginal value for Ground-Node variable
// + after  : BOOL   : new value for Ground-Node variable.
// RETURN :: void :
// **/
void MOBILITY_ChangeGroundNode(Node* node, BOOL before, BOOL after);

// /**
// API :: MOBILITY_ChangePositionGranularity
// PURPOSE :: Change Mobility-Position-Granularity  value..
// PARAMETERS ::
// + node        : Node* : Pointer to node being initialized.
// RETURN :: void :
// **/
void MOBILITY_ChangePositionGranularity(Node* node);

/*
 * FUNCTION    MOBILITY_BuildMinHeap
 * PURPOSE     Rebuild the current Heap to maintain Min-heap properties by
 *             Call Min-Heapify from bottoms up.
 *
*/
void MOBILITY_BuildMinHeap(Node* node, MobilityHeap* heapPtr);

#endif /*MOBILITY_H*/
