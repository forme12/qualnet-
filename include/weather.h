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
// PACKAGE     :: WEATHER
// DESCRIPTION :: The Weather package includes functions for modeling
//                of arbitrary shaped mobile weather patterns and their
//                effect on propagation.
// **/

#ifndef _QUALNET_WEATHER_H
#define _QUALNET_WEATHER_H

#include "main.h"
#include "clock.h"
#include "coordinates.h"
#include "fileio.h"
#include "node.h"
#include "3dmath.h"

#define WEATHER_DEFAULT_MOVEMENT_INTERVAL (10 * SECOND)

// /**
// ENUM        :: WeatherPolarization
// DESCRIPTION :: Specifies Horizontal or vertical Polarizations. 
//                
// **/

enum WeatherPolarization {
    HORIZONTAL_POLARIZATION,
    VERTICAL_POLARIZATION
};

enum WeatherType
{
    WEATHER_DEFAULT
};

// /**
// STRUCT      :: WeatherWaypoint
// DESCRIPTION :: Mobility waypoint for weather.
// **/
struct WeatherWaypoint {
    Coordinates firstPoint;  // the coordinates of the first point
    clocktype   arrivalTime;
};

// /**
// STRUCT      :: WeatherPattern
// DESCRIPTION :: Describes a weather pattern's shape, intensity and mobility.
// **/
struct WeatherPattern {
    Coordinates**       points;         // array of points describing a polygon
    int                 numberOfPoints;
    double              intensity;      // mm/h
    WeatherPolarization polarization; 
    WeatherWaypoint**   waypoints;
    int                 numberOfWaypoints;
    int                 currentWaypoint;
    Triangle3*          triangles;      // 3d-triangles version of the pattern
    int                 numTriangles;   // number of triangles in the pattern
};

// /**
// FUNCTION        :: WEATHER_Init
// PURPOSE         :: Creates the weather patterns and starts
//                    the mobility timer.
// PARAMETERS      ::
// + partitionData  : PartitionData* : structure shared among nodes
// + nodeInput      : NodeInput*     : structure containing contents of input file
// RETURN          :: void :
void WEATHER_Init(PartitionData* partitionData,
                  NodeInput*     nodeInput);


// /**
// FUNCTION        :: WEATHER_ProcessEvent
// PURPOSE         :: Processes a weather related event.
// PARAMETERS      ::
// + node          : Node*       : the first node in the partition
// + msg           : Message*    : an event
// RETURN          :: void :
void WEATHER_ProcessEvent(Node*    node,
                          Message* msg);


// /**
// FUNCTION        :: WEATHER_GetTotalPathloss
// PURPOSE         :: Returns the combined pathloss for all weather systems
//                    for a signal of interest.
// PARAMETERS      ::
// + partitionData : PartitionData* : Contains the weather patterns.
// + point1        : Coordinates    : Position of the first node.
// + point2        : Coordinates    : Position of the second node.
// + channelIndex  : int            : index of the channel.
// RETURN          :: double   : the combined pathloss due to weather for
//                               signals between these two points
double WEATHER_GetTotalPathloss(PartitionData* partitionData,
                                Coordinates    point1,
                                Coordinates    point2,
                                int            channelIndex);

#endif
