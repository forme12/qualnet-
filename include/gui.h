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
// PACKAGE :: GUI
// DESCRIPTION ::
//     This file describes data structures and functions for
//     interfacing with the QualNet GUI and the other
//     graphical tools.
// **/

#ifndef GUI_H
#define GUI_H

#include <string>

#include "main.h"
#include "coordinates.h"
#include "external.h"
#include "dynamic.h"

/*------------------------------------------------------------------
 * Types to be used for animation.  These are used by protocol
 * developers to add animation to their protocols.
 *------------------------------------------------------------------*/

#ifdef _WIN32
    #define SOCKET_HANDLE unsigned int      // The socket
#else /* unix/linux */
   #define SOCKET_HANDLE unsigned int      // The socket
#endif


// /**
// ENUM :: GuiLayers
// DESCRIPTION :: Layer in protocol stack.  Allows animation filtering.
// **/
enum GuiLayers {
    GUI_MOBILITY_LAYER,
    GUI_CHANNEL_LAYER,
    GUI_PHY_LAYER,
    GUI_MAC_LAYER,
    GUI_NETWORK_LAYER,
    GUI_TRANSPORT_LAYER,
    GUI_APP_LAYER,
    GUI_ROUTING_LAYER,
    GUI_ANY_LAYER
};

// /**
// ENUM :: GuiEvents
// DESCRIPTION :: Semantic events to be animated.
// **/
enum GuiEvents {
    GUI_INITIALIZE                = 0,
    GUI_INIT_NODE                 = 1,
    GUI_MOVE_NODE                 = 2,
    GUI_SET_NODE_TYPE             = 3,
    GUI_ADD_LINK                  = 4,
    GUI_DELETE_LINK               = 5,
    GUI_BROADCAST                 = 6,
    GUI_MULTICAST                 = 7,
    GUI_UNICAST                   = 8,
    GUI_RECEIVE                   = 9,
    GUI_DROP                      = 10,
    GUI_COLLISION                 = 11,
    GUI_CREATE_SUBNET             = 12,
    GUI_ADD_APP                   = 13,
    GUI_DELETE_APP                = 14,
    GUI_CREATE_HIERARCHY          = 15,
    GUI_SET_ORIENTATION           = 16,
    GUI_SET_NODE_ICON             = 17,
    GUI_SET_NODE_LABEL            = 18,
    GUI_SET_NODE_RANGE            = 19,
    GUI_QUEUE_CREATE              = 20,
    GUI_QUEUE_ADD                 = 21,
    GUI_QUEUE_REMOVE              = 22,
    GUI_QUEUE_DROP                = 23,
    GUI_SET_PATTERN               = 24,
    GUI_ENDBROADCAST              = 25,
    GUI_SET_PATTERNANGLE          = 26,
    GUI_SET_INTERFACE_ADDRESS     = 27,
    GUI_SET_SUBNET_MASK           = 28,
    GUI_SET_INTERFACE_NAME        = 29,
    GUI_SET_PATHLOSS_DATA         = 30,
    GUI_SET_DISTANCE_DATA         = 31,
    GUI_SET_MAPPING_NODE_IDS      = 32,
    GUI_SET_NUMNODES              = 33,
    GUI_PATHLOSS_FILE_INITIALIZED = 34,
    GUI_INIT_WIRELESS             = 35,
    GUI_CREATE_WEATHER_PATTERN    = 36,
    GUI_MOVE_WEATHER_PATTERN      = 37,
    GUI_MOVE_HIERARCHY            = 38,
    GUI_DYNAMIC_AddObject         = 39, // DYNAMIC_API
    GUI_DYNAMIC_RemoveObject      = 40, // DYNAMIC_API
    GUI_DYNAMIC_AddLink           = 41, // DYNAMIC_API
    GUI_DYNAMIC_ObjectValue       = 42, // DYNAMIC_API
    GUI_DYNAMIC_ExecuteResult     = 43, // DYNAMIC_API
    GUI_DYNAMIC_ObjectPermissions = 44, // DYNAMIC_API
    GUI_STATS_MANAGER_UpdateValue = 45, // Stats Manager
    GUI_VISUALIZATION_OBJECT      = 46,
    GUI_ACTIVATE_INTERFACE        = 47, // activate interface
    GUI_DEACTIVATE_INTERFACE      = 48, // deactivate interface
    // EXata specific events are in range 50-100
    GUI_SET_EXTERNAL_NODE         = 50,
    GUI_RESET_EXTERNAL_NODE       = 51,
    GUI_SET_REALTIME_INDICATOR_STATUS = 52,

    // The maximum number of GUI events.  This enum value must
    // always be last.
    GUI_MAX_EVENTS
};

// /**
// ENUM :: GuiStatisticsEvents
// DESCRIPTION :: Statistics events recognized by Animator.
// **/
enum GuiStatisticsEvents {
    GUI_DEFINE_METRIC   = 0,
    GUI_SEND_REAL       = 1,
    GUI_SEND_INTEGER    = 2,
    GUI_SEND_UNSIGNED   = 3
};

// /**
// ENUM :: GuiMetrics
// DESCRIPTION :: Types of statistical metrics.
// **/
enum GuiMetrics {
    GUI_CUMULATIVE_METRIC = 0,
    GUI_AVERAGE_METRIC    = 1
};

// /**
// ENUM :: GuiDataTypes
// DESCRIPTION :: The numeric data types supported for dynamic statistics.
// **/
enum GuiDataTypes {
    GUI_INTEGER_TYPE   = 0,
    GUI_DOUBLE_TYPE    = 1,
    GUI_UNSIGNED_TYPE  = 2
};

// /**
// ENUM :: GuiEffects
// DESCRIPTION :: Animation effects that can be assigned to an event.
// **/
enum GuiEffects {
    GUI_DEFAULT_EFFECT     = 0,
    GUI_NO_EFFECT          = 1,
    GUI_CHANGE_ICON        = 2,
    GUI_CHANGE_POSITION    = 3,
    GUI_CHANGE_NAME        = 4,
    GUI_CHANGE_ORIENTATION = 5,
    GUI_DRAW_ARROW         = 6,
    GUI_ERASE_ARROW        = 7,
    GUI_FLASH_ARROW        = 8,
    GUI_DRAW_LINK          = 9,
    GUI_ERASE_LINK         = 10,
    GUI_FLASH_LINK         = 11,
    GUI_OMNI_BROADCAST     = 12,
    GUI_DIRECTED_BROADCAST = 13,
    GUI_DRAW_SUBNET        = 14,
    GUI_WIRED_BROADCAST    = 15,
    GUI_FLASH_X            = 16,
    GUI_CIRCLE_NODE        = 17,
    GUI_SET_COLOR          = 18,
    GUI_ADD_LABEL          = 19
};

// /**
// ENUM :: GuiColors
// DESCRIPTION :: Colors that can be assigned to Animator effects.
// **/
enum GuiColors {
    GUI_BLACK      = 0,
    GUI_BLUE       = 1,
    GUI_CYAN       = 2,
    GUI_DARK_GRAY  = 3,
    GUI_GRAY       = 4,
    GUI_GREEN      = 5,
    GUI_LIGHT_GRAY = 6,
    GUI_MAGENTA    = 7,
    GUI_ORANGE     = 8,
    GUI_PINK       = 9,
    GUI_RED        = 10,
    GUI_WHITE      = 11,
    GUI_YELLOW     = 12,
    GUI_RECEIVE_OTHER = 13
};

// /**
// ENUM :: GuiSubnetTypes
// DESCRIPTION :: Types of subnets recognized by the Animator.
// **/
enum GuiSubnetTypes {
    GUI_WIRED_SUBNET      = 0,
    GUI_WIRELESS_SUBNET   = 1,
    GUI_SATELLITE_NETWORK = 2
};

// /**
// ENUM :: GuiVisObjCommands
// DESCRIPTION :: Commands for displaying visualization objects
// **/
enum GuiVisObjCommands {
    GUI_ADD_FILTER        = 0,
    GUI_DRAW_LINE         = 1,
    GUI_DRAW_SHAPE        = 2,
    GUI_DRAW_TEXT         = 3,
    GUI_DELETE_OBJECTS    = 4,
    GUI_DRAW_FLOW_LINE    = 5
};

// /**
// ENUM :: GuiVisShapes
// DESCRIPTION :: Shape selections for GUI_DRAW_SHAPE command
// **/
enum GuiVisShapes {
    GUI_SHAPE_CIRCLE      = 0,
    GUI_SHAPE_TRIANGLE    = 1,
    GUI_SHAPE_SQUARE      = 2
};

//--------------------------------------------------------
// data structures for use in managing dynamic statistics
//--------------------------------------------------------

// /**
// STRUCT :: MetricData
// DESCRIPTION :: Class to identify a specific dynamic statistic.
// **/
struct MetricData {
    int  metricID;
    int  metricLayerID;
    int  metricDataType;
    char metricName[MAX_STRING_LENGTH];
};

// /**
// STRUCT :: MetricLayerData
// DESCRIPTION :: Contains a list of the metrics collected at a layer of the
//                protocol stack.
// **/
struct MetricLayerData {
    MetricData* metricList;
    int         numberAllocated;
    int         numberUsed;
};

// /**
// CONSTANT:: GUI_DEFAULT_STEP : 1 econds
// DESCRIPTION ::
//     The default interval before waiting for the Animator handshake/STEP.
// **/
#define GUI_DEFAULT_STEP 1000000000

// /**
// CONSTANT:: GUI_DEFAULT_ICON :  "<no-icon>"
// DESCRIPTION :: Icon used in case none is specified for a node.
// **/
#define GUI_DEFAULT_ICON "<no-icon>"

// /**
// CONSTANT:: MAX_LAYERS : 12
// DESCRIPTION :: By default, there are 8 layers, but users may add more
// **/
#define MAX_LAYERS  12

/*------------------------------------------------------------------
 * The type fields of the various animation API functions are used
 * to distinguish between different types of signals/nodes/links.
 * For example, the user may wish to distinguish between RTS and CTS
 * signals.  The default value for these calls is 0.  The user should
 * define additional values beginning with 1.
 *------------------------------------------------------------------*/

// /**
// CONSTANT:: GUI_DEFAULT_DATA_TYPE : 0
// DESCRIPTION :: Default value to use for data types.
// **/
#define GUI_DEFAULT_DATA_TYPE 0

// /**
// CONSTANT:: GUI_EMULATION_DATA_TYPE : 1
// DESCRIPTION :: Default value to use for data types.
// **/
#define GUI_EMULATION_DATA_TYPE 1

// /**
// CONSTANT:: GUI_DEFAULT_LINK_TYPE : 0
// DESCRIPTION :: Default value to use for link types.
// **/
#define GUI_DEFAULT_LINK_TYPE 0

// /**
// CONSTANT:: GUI_DEFAULT_NODE_TYPE : 0
// DESCRIPTION :: Default value to use for node types.
// **/
#define GUI_DEFAULT_NODE_TYPE 0

// /**
// CONSTANT:: GUI_DEFAULT_INTERFACE : 0
// DESCRIPTION :: Default interface for GUI commands.
// **/
#define GUI_DEFAULT_INTERFACE 0

// /**
// CONSTANT:: GUI_WIRELESS_LINK_TYPE : 1
// DESCRIPTION :: Used to distinguish wireless and wired links.
// **/
#define GUI_WIRELESS_LINK_TYPE 1

// /**
// CONSTANT:: GUI_ATM_LINK_TYPE : 2
// DESCRIPTION :: Used to distinguish ATM links from other types.
// **/
#define GUI_ATM_LINK_TYPE 2

// /**
// CONSTANT:: GUI_COVERAGE_LINK_TYPE : 3
// DESCRIPTION :: Used by Stats Manager
// **/
#define GUI_COVERAGE_LINK_TYPE 3

/*------------------------------------------------------------------
 * GUI functions.
 *------------------------------------------------------------------*/
// /**
// API :: GUI_HandleHITLInput
// PURPOSE ::
//   Called from GUI_EXTERNAL_ReceiveCommand() if command type
//   is GUI_USER_DEFINED. Created so that GUI Human In the loop 
//   commands can also be given through a file, instead of giving it 
//   through the GUI. Will serve for good unit testing of GUI HITL commands
// PARAMETERS ::
// + args              : const char *        : the command itself
// + partition         : PartitionData *     : the partition pointer
// RETURN :: void : NULL
// **/
void GUI_HandleHITLInput(const char *args, PartitionData *partition);

// /**
// API :: GUI_Initialize
// PURPOSE ::
//   Initializes the GUI in order to start the animation.
//   The terrain map should give the path (either absolute, or
//   relative to QUALNET_HOME) of an file to represent the
//   terrain.
// PARAMETERS ::
// + nodeInput        : NodeInput*  : configuration file
// + numNodes         : int         : the number of nodes in the simulation
// + coordinateSystem : int         : LATLONALT or CARTESIAN
// + origin           : Coordinates : Southwest corner
// + dimensions       : Coordinates : Northeast corner, or size
// + maxClock         : clocktype   : length of the simulation
// RETURN :: void : NULL
// **/
extern void GUI_Initialize(NodeInput*  nodeInput,
                           int         numNodes,
                           int         coordinateSystem,
                           Coordinates origin,
                           Coordinates dimensions,
                           clocktype   maxClock);
// /**
// API :: GUI_SetEffect
// PURPOSE ::
//    This function will allow the protocol designer to specify the
//    effect to use to display certain events.
// PARAMETERS ::
// + event  : GuiEvents  : the type of event for the new effect
// + layer  : GuiLayers  : the protocol layer
// + type   : int        : special key to distinguish similar events
// + effect : GuiEffects : the effect to use
// + color  : GuiColors  : optional color for the effect
// RETURN :: void : NULL
// **/
extern void GUI_SetEffect(GuiEvents  event,
                          GuiLayers  layer,
                          int        type,
                          GuiEffects effect,
                          GuiColors  color);

// /**
// API :: GUI_InitNode
// PURPOSE ::
//    Provides the initial location and orientation of the node, the
//    transmission range (for wireless nodes), a node type, and optional
//    icon and label.
// PARAMETERS ::
// + node      : Node*      : the node
// + nodeInput : NodeInput* : configuration file
// + time      : clocktype  : the current simulation time
// RETURN :: void : NULL
// **/
extern void GUI_InitNode(Node*      node,
                         NodeInput* nodeInput,
                         clocktype  time);

// /**
// API :: GUI_InitWirelessInterface
// PURPOSE ::
//    Provides the initial location and orientation of the node, the
//    transmission range (for wireless nodes), a node type, and optional
//    icon and label.
// PARAMETERS ::
// + node           : Node*      : the node
// + interfaceIndex : int        : the interface to initialize
// RETURN :: void : NULL
// **/
extern void GUI_InitWirelessInterface(Node* node,
                                      int interfaceIndex);

// /**
// API :: GUI_InitializeInterfaces
// PURPOSE ::
//    Sets the IP address associated with one of the node's interfaces.
// PARAMETERS ::
// + nodeInput : NodeInput* : configuration file
// RETURN :: void : NULL
// **/
extern void GUI_InitializeInterfaces(const NodeInput* nodeInput);

// /**
// API :: GUI_SetInterfaceAddress
// PURPOSE ::
//    Sets the IP address associated with one of the node's interfaces.
// PARAMETERS ::
// + nodeID           : NodeId      : the node's ID
// + interfaceAddress : NodeAddress : new IP address
// + interfaceIndex   : int         : interface Address to change
// + time             : clocktype   : the current simulation time
// RETURN :: void : NULL
// **/
extern void GUI_SetInterfaceAddress(NodeId      nodeID,
                                    NodeAddress interfaceAddress,
                                    int         interfaceIndex,
                                    clocktype   time);

// /**
// API :: GUI_SetSubnetMask
// PURPOSE ::
//    Sets the Subnet mask associated with one of the node's interfaces.
// PARAMETERS ::
// + nodeID           : NodeId      : the node's ID
// + interfaceAddress : NodeAddress : new Subnet mask
// + interfaceIndex   : int         : Subnet mask to change
// + time             : clocktype   : the current simulation time
// RETURN :: void : NULL
// **/
extern void GUI_SetSubnetMask(NodeId      nodeID,
                              NodeAddress subnetMask,
                              int         interfaceIndex,
                              clocktype   time);
// /**
// API :: GUI_SetInterfaceName
// PURPOSE ::
//    Sets the Interface name associated with one of the node's interfaces.
// PARAMETERS ::
// + nodeID           : NodeId      : the node's ID
// + interfaceAddress : char*       : new Interface name
// + interfaceIndex   : int         : interface Name to change
// + time             : clocktype   : the current simulation time
// RETURN :: void : NULL
// **/
extern void GUI_SetInterfaceName(NodeId      nodeID,
                                 char*       interfaceName,
                                 int         interfaceIndex,
                                 clocktype   time);

// /**
// API :: GUI_MoveNode
// PURPOSE :: Moves the node to a new position.
// PARAMETERS ::
// + nodeID   : NodeId      : the node's ID
// + position : Coordinates : the new position
// + time     : clocktype   : the current simulation time
// RETURN :: void : NULL
// **/
extern void GUI_MoveNode(NodeId      nodeID,
                         Coordinates position,
                         clocktype   time);

// /**
// API :: GUI_SetNodeOrientation
// PURPOSE :: Changes the orientation of a node.
// PARAMETERS ::
// + nodeID      : NodeId      : the node's ID
// + orientation : Orientation : the new orientation
// + time        : clocktype   : the current simulation time
// RETURN :: void : NULL
// **/
extern void GUI_SetNodeOrientation(NodeId      nodeID,
                                   Orientation orientation,
                                   clocktype   time);

// /**
// API :: GUI_SetNodeIcon
// PURPOSE :: Changes the icon associated with a node.
// PARAMETERS ::
// + nodeID   : NodeId      : the node's ID
// + iconFile : char*       : the path to the image file, may be the
//                            absolute path or relative to QUALNET_HOME
// + time     : clocktype   : the current simulation time
// RETURN :: void : NULL
// **/
extern void GUI_SetNodeIcon(NodeId      nodeID,
                            const char*       iconFile,
                            clocktype   time);

extern void GUI_SetExternalNode(NodeId      nodeID,
                            int type,
                            clocktype   time);

extern void GUI_ResetExternalNode(NodeId      nodeID,
                            int type,
                            clocktype   time);
// /**
// API :: GUI_SetNodeLabel
// PURPOSE :: Changes the label (the node name) of a node.
// PARAMETERS ::
// + nodeID : NodeId      : the node's ID
// + label  : char*       : a string to label the node
// + time   : clocktype   : the current simulation time
// RETURN :: void : NULL
// **/
extern void GUI_SetNodeLabel(NodeId      nodeID,
                             char*       label,
                             clocktype   time);

// /**
// API :: GUI_SetNodeRange
// PURPOSE :: Changes the transmission range of a node
// PARAMETERS ::
// + nodeID : NodeId      : the node's ID
// + interfaceIndex       : int         : which of the node's interfaces to use
// + range  : double      : the new transmission range in meters
// + time   : clocktype   : the current simulation time
// RETURN :: void : NULL
// **/
extern void GUI_SetNodeRange(NodeId      nodeID,
                             int         interfaceIndex,
                             double      range,
                             clocktype   time);

// /**
// API :: GUI_SetNodeType
// PURPOSE :: Changes the (symbolic) type of a node
// PARAMETERS ::
// + nodeID : NodeId      : the node's ID
// + type   : int         : user defined type, used with GUI_SetEffect
// + time   : clocktype   : the current simulation time
// RETURN :: void : NULL
// **/
extern void GUI_SetNodeType(NodeId      nodeID,
                            int         type,
                            clocktype   time);

// /**
// API :: GUI_SetPatternIndex
// PURPOSE ::
//    Sets the antenna pattern to one of a previously specified
//    antenna pattern file.
// PARAMETERS ::
// + node        : Node*      : the node pointer
// + interfaceIndex  : int         : which of the node's interfaces to use
// + index           : int         : index into the node's antenna pattern file
// + time            : clocktype   : the current simulation time
// RETURN :: void : NULL
// **/
extern void GUI_SetPatternIndex(Node*      node,
                                int         interfaceIndex,
                                int         index,
                                clocktype   time);

// /**
// API :: GUI_SetPatternAndAngle
// PURPOSE ::
//    For steerable antennas, it sets the pattern to use, and also
//    an angle relative to the node's current orientation.
// PARAMETERS ::
// + node*        : node      : the node pointer
// + interfaceIndex : int         : which of the node's interfaces to use
// + index          : int         : index into the node's antenna pattern file
// + angleInDegrees : int         : angle to rotate the pattern
// + time           : clocktype   : the current simulation time
// RETURN :: void : NULL
// **/
extern void GUI_SetPatternAndAngle(Node*    node,
                                   int       interfaceIndex,
                                   int       index,
                                   int       angleInDegrees,
                                   int       elevationAngle,
                                   clocktype time);

// /**
// API :: GUI_AddLink
// PURPOSE ::
//    Adds a link (one hop on a route) between two nodes.  In a wired
//    topology, this could be a static route; in wireless, a dynamic one.
// PARAMETERS ::
// + sourceID      : NodeId      : the source node for the link
// + destID        : NodeId      : the destination node
// + layer         : GuiLayers   : the protocol layer associated w/ the link
// + type          : int         : a user-defined type for the link
// + subnetAddress : NodeAddress : subnet address for network links
// + numHostBits   : int         : subnet size for network links
// + time          : clocktype   : the current simulation time
// RETURN :: void : NULL
// **/
extern void GUI_AddLink(NodeId      sourceID,
                        NodeId      destID,
                        GuiLayers   layer,
                        int         type,
                        NodeAddress subnetAddress,
                        int         numHostBits,
                        clocktype   time);

// /**
// API :: GUI_AddLink
// PURPOSE ::
//    Adds an IPv6 link (one hop on a route) between two nodes.  In a wired
//    topology, this could be a static route; in wireless, a dynamic one.
// PARAMETERS ::
// + sourceID      : NodeId    : the source node for the link
// + destID        : NodeId    : the destination node
// + layer         : GuiLayers : the protocol layer associated w/ the link
// + type          : int       : a user-defined type for the link
// + tla           : int       : TLA field of IPv6 address
// + nla           : int       : NLA field of IPv6 address
// + sla           : int       : SLA field of IPv6 address
// + time          : clocktype : the current simulation time
// RETURN :: void : NULL
// **/
extern void GUI_AddLink(NodeId    sourceID,
                        NodeId    destID,
                        GuiLayers layer,
                        int       type,
                        int       tla,
                        int       nla,
                        int       sla,
                        clocktype time);

// /**
// API :: GUI_AddLink
// PURPOSE ::
//    Adds an IPv6 link (one hop on a route) between two nodes.  In a wired
//    topology, this could be a static route; in wireless, a dynamic one.
// PARAMETERS ::
// + sourceID      : NodeId              : the source node for the link
// + destID        : NodeId              : the destination node
// + layer         : GuiLayers           : the protocol layer associated w/ the link
// + type          : int                 : a user-defined type for the link
// + ip6_addr      : IPv6subnetAddress   : IPv6 address
// + unsigned int  : IPv6subnetPrefixLen : IPv6 address prefix length
// + time          : clocktype           : the current simulation time
// RETURN :: void : NULL
// **/
extern void GUI_AddLink(NodeId       sourceID,
                        NodeId       destID,
                        GuiLayers    layer,
                        int          type,
                        char*        ip6Addr,
                        unsigned int IPv6subnetPrefixLen,
                        clocktype time);

// /**
// API :: GUI_DeleteLink
// PURPOSE :: Removes link of a specific type.
// PARAMETERS ::
// + sourceID : NodeId      : the source node for the link
// + destID   : NodeId      : the destination node
// + layer    : GuiLayers   : the protocol layer associated w/ the link
// + type     : int         : type of link being deleted
// + time     : clocktype   : the current simulation time
// RETURN :: void : NULL
// **/
void GUI_DeleteLink(NodeId      sourceID,
                    NodeId      destID,
                    GuiLayers   layer,
                    int         type,
                    clocktype   time);

// /**
// API :: GUI_DeleteLink
// PURPOSE :: Removes the aforementioned link, no matter the "type."
// PARAMETERS ::
// + sourceID : NodeId      : the source node for the link
// + destID   : NodeId      : the destination node
// + layer    : GuiLayers   : the protocol layer associated w/ the link
// + time     : clocktype  : the current simulation time
// RETURN :: void : NULL
// **/
extern void GUI_DeleteLink(NodeId      sourceID,
                           NodeId      destID,
                           GuiLayers   layer,
                           clocktype   time);

// /**
// API :: GUI_Broadcast
// PURPOSE :: Indicates a broadcast.
// PARAMETERS ::
// + nodeID           : NodeId      : the node's ID
// + layer            : GuiLayers   : the protocol layer associated w/ event
// + type             : int         : a user-defined type for the link
// + interfaceIndex   : int         : which of the node's interfaces to use
// + time             : clocktype   : the current simulation time
// RETURN :: void : NULL
// **/
extern void GUI_Broadcast(NodeId      nodeID,
                          GuiLayers   layer,
                          int         type,
                          int         interfaceIndex,
                          clocktype   time);

// /**
// API :: GUI_EndBroadcast
// PURPOSE :: Indicates the end of a broadcast.
// PARAMETERS ::
// + nodeID           : NodeId      : the node's ID
// + layer            : GuiLayers   : the protocol layer associated w/ event
// + type             : int         : a user-defined type for the link
// + interfaceIndex   : int         : which of the node's interfaces to use
// + time             : clocktype   : the current simulation time
// RETURN :: void : NULL
// **/
extern void GUI_EndBroadcast(NodeId      nodeID,
                             GuiLayers   layer,
                             int         type,
                             int         interfaceIndex,
                             clocktype   time);

// /**
// API :: GUI_Multicast
// PURPOSE ::
//    Indicates a multicast. (Probably need to add a destination address.)
// PARAMETERS ::
// + nodeID           : NodeId      : the node's ID
// + layer            : GuiLayers   : the protocol layer associated w/ event
// + type             : int         : a user-defined type for the link
// + interfaceIndex   : int         : which of the node's interfaces to use
// + time             : clocktype   : the current simulation time
// RETURN :: void : NULL
// **/
extern void GUI_Multicast(NodeId      nodeID,
                          GuiLayers   layer,
                          int         type,
                          int         interfaceIndex,
                          clocktype   time);

// /**
// API :: GUI_Unicast
// PURPOSE ::
//    Sends a unicast packet/frame/signal to a destination.
//    Will probably be drawn as a temporary line between source and
//    destination, followed by a signal (at the receiver) indicating
//    success or failure.
// PARAMETERS ::
// + sourceID                : NodeId      : the source node
// + destID                  : NodeId      : the destination node
// + layer                   : GuiLayers   : protocol layer associated w/ the event
// + type                    : int         : a user-defined type
// + sendingInterfaceIndex   : int         : sender's interface to use
// + receivingInterfaceIndex : int         : receiver's interface to use
// + time                    : clocktype   : the current simulation time
// RETURN :: void : NULL
// **/
extern void GUI_Unicast(NodeId      sourceID,
                        NodeId      destID,
                        GuiLayers   layer,
                        int         type,
                        int         sendingInterfaceIndex,
                        int         receivingInterfaceIndex,
                        clocktype   time);

// /**
// API :: GUI_Receive
// PURPOSE :: Shows a successful receipt at a destination.
// PARAMETERS ::
// + sourceID                : NodeId      : the source node
// + destID                  : NodeId      : the destination node
// + layer                   : GuiLayers   : protocol layer associated w/ the event
// + type                    : int         : a user-defined type
// + sendingInterfaceIndex   : int         : sender's interface to use
// + receivingInterfaceIndex : int         : receiver's interface to use
// + time                    : clocktype   : the current simulation time
// RETURN :: void : NULL
// **/
extern void GUI_Receive(NodeId      sourceID,
                        NodeId      destID,
                        GuiLayers   layer,
                        int         type,
                        int         sendingInterfaceIndex  ,
                        int         receivingInterfaceIndex,
                        clocktype   time);

// /**
// API :: GUI_Drop
// PURPOSE :: Shows a packet/frame/signal being dropped by a node.
// PARAMETERS ::
// + sourceID                : NodeId      : the source node
// + destID                  : NodeId      : the destination node
// + layer                   : GuiLayers   : protocol layer associated w/ the event
// + type                    : int         : a user-defined type
// + sendingInterfaceIndex   : int         : sender's interface to use
// + receivingInterfaceIndex : int         : receiver's interface to use
// + time                    : clocktype   : the current simulation time
// RETURN :: void : NULL
// **/
extern void GUI_Drop(NodeId      sourceID,
                     NodeId      destID,
                     GuiLayers   layer,
                     int         type,
                     int         sendingInterfaceIndex  ,
                     int         receivingInterfaceIndex,
                     clocktype   time);

// /**
// API :: GUI_Collision
// PURPOSE :: Shows a node detecting a collision.
// PARAMETERS ::
// + nodeID : NodeId      : the node's ID
// + layer  : GuiLayers   : the protocol layer associated w/ event
// + time   : clocktype   : the current simulation time
// RETURN :: void : NULL
// **/
extern void GUI_Collision(NodeId      nodeID,
                          GuiLayers   layer,
                          clocktype   time);

// /**
// API :: GUI_CreateSubnet
// PURPOSE :: Creates a subnet.  Normally done at startup.
// PARAMETERS ::
// + type          : GuiSubnetTypes : GUI_WIRED/WIRELESS/SATELLITE_NETWORK
// + subnetAddress : NodeAddress    : base address for the subnet
// + numHostBits   : int            : number of host bits for subnet mask
// + nodeList      : const char*    : the rest of the .config file SUBNET line
// + time          : clocktype      : the current simulation time
// RETURN :: void : NULL
// **/
extern void GUI_CreateSubnet(GuiSubnetTypes type,
                             NodeAddress    subnetAddress,
                             int            numHostBits,
                             const char*    nodeList,
                             clocktype      time);

// /**
// API :: GUI_CreateSubnet
// PURPOSE :: Creates a IPv6 subnet.  Normally done at startup.
// PARAMETERS ::
// + type          : GuiSubnetTypes : GUI_WIRED/WIRELESS/SATELLITE_NETWORK
// + IPv6subnetAddress : in6_addr       : base address for the subnet
// + IPv6subnetPrefixLen : unsigned int : number of network bits present
// + nodeList      : const char* : the rest of the .config file SUBNET line
// + time          : clocktype   : the current simulation time
// RETURN :: void : NULL
// **/
void GUI_CreateSubnet(GuiSubnetTypes type,
                      char* ip6Addr,
                      unsigned int   IPv6subnetPrefixLen,
                      const char*    nodeList,
                      clocktype      time);

// /**
// API :: GUI_CreateSubnet
// PURPOSE :: Creates a IPv6 subnet.  Normally done at startup.
// PARAMETERS ::
// + type          : GuiSubnetTypes      : GUI_WIRED/WIRELESS/SATELLITE_NETWORK
// + ip6_addr      : IPv6subnetAddress   : IPv6 address
// + unsigned int  : IPv6subnetPrefixLen : IPv6 address prefix length
// + nodeList      : const char*         : the rest of the .config file SUBNET line
// + time          : clocktype           : the current simulation time
// RETURN :: void : NULL
// **/
void GUI_CreateSubnet(GuiSubnetTypes type,
                      in6_addr       IPv6subnetAddress,
                      unsigned int   IPv6subnetPrefixLen,
                      const char*    nodeList,
                      clocktype      time);


// /**
// API :: GUI_CreateHierarchy
// PURPOSE ::
//    Since the GUI supports hierarchical design, this function informs
//    the GUI of the contents of a hierarchical component.
// PARAMETERS ::
// + componentID : int   : an identifier for the hierarchy
// + nodeList    : char* : the rest of the .config file COMPONENT line
// RETURN :: void : NULL
// **/
extern void GUI_CreateHierarchy(int   componentID,
                                char* nodeList);

// /**
// API :: GUI_MoveHierarchy
// PURPOSE :: Moves the center point of a hierarchy to a new position.
// PARAMETERS ::
// + hierarchyId       : int         : the hierarchy's ID
// + centerCoordinates : Coordinates : the new position
// + orientation       : Orientation : the new orientation
// + time              : clocktype   : the current simulation time
// RETURN :: void : NULL
// **/
extern void GUI_MoveHierarchy(int         hierarchyID,
                              Coordinates centerCoordinates,
                              Orientation orientation,
                              clocktype   time);

// /**
// API :: GUI_CreateWeatherPattern
// PURPOSE ::
//    Sends the input line describing a weather pattern to the GUI.
// PARAMETERS ::
// + patternID  : int   : the weather pattern ID
// + inputLine  : char* : the .weather file line
// RETURN :: void : NULL
// **/
extern void GUI_CreateWeatherPattern(int   patternID,
                                     char* inputLine);


// /**
// API :: GUI_MoveWeatherPattern
// PURPOSE :: Moves the first point of a weather pattern to a new position.
// PARAMETERS ::
// + patternID   : int         : the weather pattern ID
// + coordinates : Coordinates : the new position
// + time        : clocktype   : the current simulation time
// RETURN :: void : NULL
// **/
extern void GUI_MoveWeatherPattern(int         patternID,
                                   Coordinates coordinates,
                                   clocktype   time);

// /**
// API :: GUI_AddApplication
// PURPOSE ::
//    Shows label beside the client and the server as app link is setup.
// PARAMETERS ::
// + sourceID  : NodeId      : the source node
// + destID    : NodeId      : the destination node
// + appName   : char*       : the application name, e.g. "CBR"
// + uniqueId  : int         : unique label for this application session
// + time      : clocktype   : the current simulation time
// RETURN :: void : NULL
// **/
extern void GUI_AddApplication(NodeId      sourceID,
                               NodeId      destID,
                               char*       appName,
                               int         uniqueId,
                               clocktype   time);

// /**
// API :: GUI_DeleteApplication
// PURPOSE :: Deletes the labels shown by AddApplication.
// PARAMETERS ::
// + sourceID  : NodeId      : the source node
// + destID    : NodeId      : the destination node
// + appName   : char*       : the application name, e.g. "CBR"
// + uniqueId  : int         : unique label for this application session
// + time      : clocktype   : the current simulation time
// RETURN :: void : NULL
// **/
extern void GUI_DeleteApplication(NodeId      sourceID,
                                  NodeId      destID,
                                  char*       appName,
                                  int         uniqueId,
                                  clocktype   time);

// /**
// API :: GUI_AddInterfaceQueue
// PURPOSE :: Creates a queue for a node, interface and priority.
// PARAMETERS ::
// + nodeID         : NodeId      : the node's ID
// + layer          : GuiLayers   : protocol layer associated w/ the event
// + interfaceIndex : int         : associated interface of node
// + priority       : unsigned    : priority of queue
// + queueSize      : int         : maximum size in bytes
// + time           : clocktype   : the current simulation time
// RETURN :: void : NULL
// **/
extern void GUI_AddInterfaceQueue(NodeId       nodeID,
                                  GuiLayers    layer,
                                  int          interfaceIndex,
                                  unsigned int priority,
                                  int          queueSize,
                                  clocktype    currentTime);

// /**
// API :: GUI_QueueInsertPacket
// PURPOSE ::
//     Inserting one packet to a queue for a node, interface and priority
// PARAMETERS ::
// + nodeID         : NodeId      : the node's ID
// + layer          : GuiLayers   : protocol layer associated w/ the event
// + interfaceIndex : int         : associated interface of node
// + priority       : unsigned    : priority of queue
// + packetSize     : int         : size of packet
// + time           : clocktype   : the current simulation time
// RETURN :: void : NULL
// **/
extern void GUI_QueueInsertPacket(NodeId       nodeID,
                                  GuiLayers    layer,
                                  int          interfaceIndex,
                                  unsigned int priority,
                                  int          packetSize,
                                  clocktype    currentTime);

// /**
// API :: GUI_QueueDropPacket
// PURPOSE ::
//    Dropping one packet from a queue for a node, interface and priority.
// PARAMETERS ::
// + nodeID         : NodeId      : the node's ID
// + layer          : GuiLayers   : protocol layer associated w/ the event
// + interfaceIndex : int         : associated interface of node
// + priority       : unsigned    : priority of queue
// + time           : clocktype   : the current simulation time
// RETURN :: void : NULL
// **/
extern void GUI_QueueDropPacket(NodeId       nodeID,
                                GuiLayers    layer,
                                int          interfaceIndex,
                                unsigned int priority,
                                clocktype    currentTime);

// /**
// API :: GUI_QueueDequeuePacket
// PURPOSE ::
//    Dequeuing one packet from a queue for a node, interface and priority
// PARAMETERS ::
// + nodeID         : NodeId      : the node's ID
// + layer          : GuiLayers   : protocol layer associated w/ the event
// + interfaceIndex : int         : associated interface of node
// + priority       : unsigned    : priority of queue
// + packetSize     : int         : size of packet
// + time           : clocktype   : the current simulation time
// RETURN :: void : NULL
// **/
extern void GUI_QueueDequeuePacket(NodeId       nodeID,
                                   GuiLayers    layer,
                                   int          interfaceIndex,
                                   unsigned int priority,
                                   int          packetSize,
                                   clocktype    currentTime);

// /**
// API :: GUI_DefineMetric
// PURPOSE ::
//    This function defines a metric by giving it a name and a
//    description.  The system will assign a number to this data
//    item.  Future references to the data should use the number
//    rather than the name.  The link ID will be used to associate
//    a metric with a particular application link, or MAC interface, etc.
// PARAMETERS ::
// + name       : char*        : the name of the metric
// + nodeID     : NodeId       : the node's ID
// + layer      : GuiLayers    : protocol layer associated w/ the event
// + linkID     : int          : e.g., an application session ID
// + datatype   : GuiDataTypes : real/unsigned/integer
// + metrictype : GuiMetrics   : cumulative/average, etc.
// RETURN :: int : an identifier associated the the metric name and layer
// **/
extern int GUI_DefineMetric(const char*        name,
                            NodeId       nodeID,
                            GuiLayers    layer,
                            int          linkID,
                            GuiDataTypes datatype,
                            GuiMetrics   metrictype);

// /**
// API :: GUI_SendIntegerData
// PURPOSE :: Sends data for an integer metric.
// PARAMETERS ::
// + nodeID   : NodeId      : the node's ID
// + metricID : int         : the value returned by DefineMetric
// + value    : int         : the current value of the metric
// + time     : clocktype   : the current simulation time
// RETURN :: void : NULL
// **/
extern void GUI_SendIntegerData(NodeId      nodeID,
                                int         metricID,
                                int         value,
                                clocktype   time);

// /**
// API :: GUI_SendUnsignedData
// PURPOSE :: Sends data for an unsigned metric.
// PARAMETERS ::
// + nodeID   : NodeId      : the node's ID
// + metricID : int         : the value returned by DefineMetric
// + value    : unsigned    : the current value of the metric
// + time     : clocktype   : the current simulation time
// RETURN :: void : NULL
// **/
extern void GUI_SendUnsignedData(NodeId      nodeID,
                                 int         metricID,
                                 unsigned    value,
                                 clocktype   time);

// /**
// API :: GUI_SendRealData
// PURPOSE :: Sends data for a floating point metric.
// PARAMETERS ::
// + nodeID   : NodeId      : the node's ID
// + metricID : int         : the value returned by DefineMetric
// + value    : double      : the current value of the metric
// + time     : clocktype   : the current simulation time
// RETURN :: void : NULL
// **/
extern void GUI_SendRealData(NodeId      nodeID,
                             int         metricID,
                             double      value,
                             clocktype   time);


/*------------------------------------------------------------------
 * The following declarations are used for interactive control of
 * the Simulator via the Animator.  Reserved for kernel use.
 *------------------------------------------------------------------*/

// /**
// CONSTANT:: GUI_MAX_COMMAND_LENGTH : 1024
// DESCRIPTION :: Maximum length for a single interchange with Animator.
// **/
#define GUI_MAX_COMMAND_LENGTH 1024

// /**
// ENUM :: GuiCommands
// DESCRIPTION :: Coded commands sent from Animator to Simulator.
// **/
enum GuiCommands {
    GUI_STEP                    = 0,
    GUI_SET_COMM_INTERVAL       = 1,
    GUI_ENABLE_LAYER            = 2,
    GUI_DISABLE_LAYER           = 3,
    GUI_ENABLE_NODE             = 4,
    GUI_DISABLE_NODE            = 5,
    GUI_SET_STAT_INTERVAL       = 6,
    GUI_ENABLE_METRIC           = 7,
    GUI_DISABLE_METRIC          = 8,
    GUI_PATHLOSSTABLE           = 9,
    GUI_DYNAMIC_ReadAsString    = 10, // DYNAMIC_API
    GUI_DYNAMIC_WriteAsString   = 11, // DYNAMIC_API
    GUI_DYNAMIC_ExecuteAsString = 12, // DYNAMIC_API
    GUI_DYNAMIC_Listen          = 13, // DYNAMIC_API
    GUI_DYNAMIC_Unlisten        = 14, // DYNAMIC_API
    GUI_STATS_MANAGER_COMMAND   = 15, // Stats Manager
    GUI_ENABLE_EVENT            = 16,
    GUI_DISABLE_EVENT           = 17,
    GUI_USER_DEFINED            = 100,
    GUI_STOP                    = 1000,
    GUI_PAUSE                   = 1001,
    GUI_RESUME                  = 1002,
    GUI_SET_ANIMATION_FILTER_FREQUENCY = 1003,
    GUI_UNRECOGNIZED            = 5000
};

// /**
// ENUM :: GuiReplies
// DESCRIPTION :: Coded commands sent from Simulator to Animator.
// **/
enum GuiReplies {
    GUI_STEPPED                 = 0,
    GUI_DATA                    = 1,
    GUI_ANIMATION_COMMAND       = 2,
    GUI_ASSERTION               = 3,
    GUI_ERROR                   = 4,
    GUI_WARNING                 = 5,
    GUI_SET_EFFECT              = 6,
    GUI_STATISTICS_COMMAND      = 7,
    GUI_PATHLOSS_TABLE          = 9,
    GUI_DYNAMIC_COMMAND         = 10, // DYNAMIC_API
    GUI_STATS_MANAGER_REPLY     = 11, // Stats Manager
    GUI_FINALIZATION_COMMAND    = 12, // Finalization Command
    GUI_FINISHED                = 1000
};

// /**
// STRUCT:: GuiCommand
// DESCRIPTION :: Structure containing command received from Animator.
// **/
struct GuiCommand {
    GuiCommands type;
    std::string args;
};

// /**
// ENUM :: GuiReply
// DESCRIPTION :: Structure containing message sent to Animator.
// **/
struct GuiReply {
    GuiReplies type;
    std::string args;
};

#ifdef D_LISTENING_ENABLED
// /**
// CLASS:: GuiDynamicObjectCallback
// DESCRIPTION :: Class used when GUI is running with Dynamic API.  Reports
//                newly created objects to GUI.
// **/
class GuiDynamicObjectCallback : public D_ListenerCallback
{
    private:
        EXTERNAL_Interface* m_Iface;
        GuiEvents m_EventType;
        
    public:
        GuiDynamicObjectCallback(
            EXTERNAL_Interface* iface,
            GuiEvents eventType);

        void operator () (const std::string& newValue);
};

class GuiDynamicObjectValueCallback : public D_ListenerCallback
{
    private:
        EXTERNAL_Interface* m_Iface;
        std::string m_Path;
        
    public:
        GuiDynamicObjectValueCallback(
            EXTERNAL_Interface* iface,
            const std::string& path);

        void operator () (const std::string& newValue);
};
#endif // D_LISTNEING_ENABLED


// /**
// FUNCTION :: GUI_isAnimateOrInteractive
// PURPOSE :: Returns true if the GUI was activated on the command line.
// PARAMETERS ::
// RETURN :: bool : True if the GUI is enabled.
// **/
bool GUI_isAnimateOrInteractive ();

// /**
// FUNCTION :: GUI_EXTERNAL_Bootstrap
// PURPOSE :: Creates a connection to the GUI
// PARAMETERS ::
// + argc               : int        : number of command line parameters
// + argv               : char**     : command line parameters
// + nodeInput          : NodeInput* : the contents of the .config file
// + numberOfProcessors : int        : the number of processors in use
// + thisPartitionId    : int        : the ID of this partition
// RETURN :: void : NULL
// **/
bool GUI_EXTERNAL_Bootstrap(int        argc,
                            char*      argv [],
                            NodeInput* nodeInput,
                            int        numberOfProcessors,
                            int        thisPartitionId);

// /**
// FUNCTION :: GUI_EXTERNAL_Registration
// PURPOSE :: Registers the GUI as an external interface
// PARAMETERS ::
// + partitionData : PartitionData*          : the partition to register with
// + list          : EXTERNAL_InterfaceList* : the list to add oneself to
// RETURN :: void : NULL
// **/
void GUI_EXTERNAL_Registration(EXTERNAL_Interface* iface,
                               NodeInput* nodeInput);

// /**
// FUNCTION :: GUI_CreateReply
// PURPOSE :: Function used to replace newline characters in a string being
//          sent to the GUI.
// PARAMETERS ::
// + replyType : GuiReplies   : the type of reply
// + msg       : std::string* : the reply message
// RETURN :: void : NULL
// **/
extern GuiReply GUI_CreateReply(GuiReplies   replyType,
                                std::string* msg);

extern void GUI_ConnectToGUI(char*          hostname,
                             unsigned short port);
extern void GUI_DisconnectFromGUI(SOCKET_HANDLE socket,
                                  bool sendFinishedReply = true);

extern void GUI_WaitForCommands(PartitionData* partitionData,
                                clocktype*     nextStep,
                                clocktype      nextEvent,
                                clocktype      simulationEndTime);
extern GuiCommand GUI_ReceiveCommand(SOCKET_HANDLE socket);
extern void GUI_SendReply(SOCKET_HANDLE socket,
                          GuiReply     reply);

extern void GUI_SetLayerFilter(const char* args, // layer ID, applies to animation
                               BOOL  offOrOn);
extern void GUI_SetNodeFilter(const char* args, // node ID, applies to animation
                              BOOL  offOrOn);
extern void GUI_SetMetricFilter(const char* args, // metric ID and node ID, for stats
                                BOOL offOrOn);

extern BOOL GUI_NodeIsEnabledForAnimation(NodeId nodeID);
extern BOOL GUI_NodeIsEnabledForStatistics(NodeId nodeID);
extern BOOL GUI_LayerIsEnabledForStatistics(GuiLayers layer);
void GeneratePathLossTable(Node *node, int channelInd);

void GUI_SendStatsManagerReply(GuiReply reply);

/*
 * NAME:        GUI_SendInterfaceActivateDeactivateStatus
 * PURPOSE:     Sends activation/deactivation status of interface
 * PARAMETERS:  NodeId       nodeID: NodeId for which activation/deactivation
 *                                   status needs to be send
 *              GuiReplies   nodeStatus: whether activation or deactivation
 *                                   of node is done
 *              Int32        interfaceIndex: index of interface which needs
 *                                   to be activated/deactivated
 *                                   (default value = -1 when the node needs
 *                                   to be activated/deactivated
 *              char*        optionalMessage: Any optional message
 *                                            (default value = NULL)
 * RETURN:      none.
 */

void GUI_SendInterfaceActivateDeactivateStatus(
                        NodeId       nodeID,
                        GuiEvents    nodeStatus,
                        Int32        interfaceIndex = ANY_INTERFACE,
                        char*        optionalMessage = NULL);

// Filter format = <filter>|<button - optional>|<node - optional>
//  <filter> - create tab in Visualizaton Controls->Animation Filters
//  <button> - create button in <filter> tab
//  <node> - create menu item for <button> menu
void GUI_AddFilter(
    const char* filter,
    const char* toolTip = "",
    bool        active = false,
    clocktype   time = 0);

void GUI_DeleteObjects(const char* id, clocktype time);

void GUI_DrawLine(
    NodeId          src,
    NodeId          dst,
    const char*     color,
    const char*     id,
    const char*     label,
    float           thickness,
    unsigned short  pattern,
    int             factor,
    bool            srcArrow,
    bool            dstArrow,
    clocktype       time);

void GUI_DrawFlowLine(
    NodeId      src,
    NodeId      dst,
    const char* color,
    const char* id,
    const char* label,
    float       thickness,
    int         pattern,
    int         factor,
    int         ttl,
    clocktype   time);

void GUI_DrawShape(
    GuiVisShapes    shape,
    NodeId          node,
    double          scale,
    const char*     color,
    const char*     id,
    clocktype       time);

void GUI_DrawText(
    NodeId      node,
    const char* text,
    const char* id,
    int         interfaceIndex,
    int         order,
    clocktype   time);

void GUI_AppHopByHopFlow(
    int         appTraceProtocolType,
    NodeId      src,
    NodeId      dst,
    NodeId      hop,
    NodeId      nextHop,
    clocktype   time);

void GUI_CreateAppHopByHopFlowFilter(
    int         appTraceProtocolType,
    const char* appName,
    const char* src,
    const char* dst,
    const char* dstLabel);

bool GUI_IsAppHopByHopFlowEnabled();

void GUI_ReadAppHopByHopFlowAnimationEnabledSetting(
    const NodeInput* nodeInput);

void GUI_RealtimeIndicator(const char* rtStatus);

/*
 * NAME:        GUI_SendFinalizationStatus
 * PURPOSE:     Sends finalization status to GUI when GUI is waiting for 
 *              simulation finish command
 * PARAMETERS:  NodeId       nodeID: NodeId for which finalization satus
 *                                   needs to be send
 *              GuiLayers    layer: Layer for which finalization satus
 *                                   needs to be send 
 *                                   (default value = GUI_ANY_LAYER)
 *              Int32        modelName: Model for which finalization satus
 *                                       needs to be send (default value = 0)
 *              Int32        subCommand: Any sub-command (default value = 0)
 *              char*        optionalMessage: Any optional message
 *                                            (default value = NULL)
 * RETURN:      none.
 */

void GUI_SendFinalizationStatus(NodeId       nodeID,
                                GuiLayers    layer = GUI_ANY_LAYER,
                                Int32        modelName = 0,
                                Int32        subCommand = 0,
                                char*        optionalMessage = NULL);
/*------------------------------------------------------------------
 * GUI variables.
 *
 * For kernel use only.
 *------------------------------------------------------------------*/

extern clocktype    GUI_statReportTime;
extern clocktype    GUI_statInterval;

extern unsigned int GUI_guiSocket;
extern BOOL         GUI_guiSocketOpened;

extern MetricLayerData g_metricData[];


#ifdef PAS_INTERFACE
void PSI_EnablePSI(PartitionData* partitionData, char* nodeId);
void PSI_EnableDot11(PartitionData* partitionData, BOOL flag);
void PSI_EnableApp(PartitionData* partitionData, BOOL flag);
void PSI_EnableUdp(PartitionData* partitionData, BOOL flag);
void PSI_EnableTcp(PartitionData* partitionData, BOOL flag);
void PSI_EnableRouting(PartitionData* partitionData, BOOL flag);
void PSI_EnableMac(PartitionData* partitionData, BOOL flag);
#endif
#endif


