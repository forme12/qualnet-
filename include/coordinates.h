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
// PACKAGE     :: COORDINATES
// DESCRIPTION :: This file describes data structures and
// functions used for coordinates-related operations.
// **/

#ifndef COORDINATES_H
#define COORDINATES_H


#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "qualnet_error.h"
#include "clock.h"
#include "fileio.h"

// /**
// CONSTANT    :: PI  : 3.14159265358979323846264338328
// DESCRIPTION :: Defines the value of constant PI
// **/
#define PI 3.14159265358979323846264338328

// /**
// CONSTANT    :: ANGLE_RESOLUTION  :  360
// DESCRIPTION :: Defines ANGLE_RESOLUTION
// **/
#define ANGLE_RESOLUTION              360

// /**
// CONSTANT    :: IN_RADIAN  :   (PI / 180.0)
// DESCRIPTION :: Defines the constant IN_RADIAN
// **/
#define IN_RADIAN                     (PI / 180.0)

// /**
// CONSTANT    :: EARTH_RADIUS  :  6375000.0
// DESCRIPTION :: Defines the above constant EARTH_RADIUS
// **/
#define EARTH_RADIUS                  6375000.0

#ifndef MAIN_H
// /**
// MACRO       :: MAX(X, Y)
// DESCRIPTION :: Finds the maximum of two entries
// **/
#define MAX(X, Y) (((X) < (Y)) ? (Y) : (X))

// /**
// MACRO    :: MIN(X, Y)
// DESCRIPTION :: Finds the minimum of two entries
// **/
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#endif

typedef double CoordinateType;
typedef short OrientationType;

// /**
// MACRO       :: COORD_ShortestPropagationDelay(dist)
// DESCRIPTION :: Calculate the shortest propagation delay. Shortest delay
//                is assumed with light speed. Actual delay could be longer
//                if propagation medium is not eletromegnatic waves, such
//                as acoustic wave.
//
// **/
#define COORD_ShortestPropagationDelay(dist) \
        ((clocktype)((dist) * 10.0 / 3.0 + 0.5))

// /**
// ENUM        :: EarthRepresentationType
// DESCRIPTION :: Defines the type of Earth that is represented
//                Replaces coordinate_system_type
// **/

enum EarthRepresentationType
{
    FLAT,
    SPHERE,
    WGS84
};

// /**
// ENUM        :: CoordinateRepresentationType
// DESCRIPTION :: Defines the coordinate system that a coordinate is given
// in reference to
// **/
enum CoordinateRepresentationType
{
    INVALID_COORDINATE_TYPE,
    // Cartesian for when the Earth is represented as flat
    // The origin has no specific location
    UNREFERENCED_CARTESIAN,
    // the following are with respect to a non-flat Earth representation
    // Latitude-longitude-altitude representation with respect to the Earth
    // positive latitude is North
    // positive longitude is East
    // positive altitude is opposite the pull of gravity
    GEODETIC,
    // Cartesian system with origin at the center of the Earth.
    // positive z-direction points towards the North pole.
    // positive x-direction points towards the intersection of the
    // Equator and the Prime Meridian
    // positive y-direction completes the right-handed coordinate systen
    GEOCENTRIC_CARTESIAN,
    // Local Tangent Space Euclidean
    // Cartesian system where origin is a specific point on the Earth
    // (which is represented in one of the other absolute earth reference
    // systems)
    // positive-z direction is against the pull of gravity at the reference,
    // perpendicular to the plain tangential to the surface of the Earth
    // positive-y direction is tangential to the surface of the Earth
    // at the reference, towards North
    // positive-x direction is tangential to the surface of the Earth
    // at the reference, towards East
    LTSE,
    // JGIS
    // c1 and c2 are positions in meters relative to a reference
    // point with type GEODETIC stored in customDataPtr
    // positive c1 is meters in the East direction
    // positive c2 is meters in the North direction
    JGIS
};

// /**
// ENUM        :: coordinate_system_type
// DESCRIPTION :: Defines the type of coordinate system
// **/
enum coordinate_system_type {
    CARTESIAN,
    LATLONALT
};

// /**
// STRUCT      :: cartesian_coordinates
// DESCRIPTION :: Defines the three  cartesian coordinates
// **/
struct cartesian_coordinates {
    CoordinateType x;
    CoordinateType y;
    CoordinateType z;
};

// /**
// STRUCT      :: latlonalt_coordinates
// DESCRIPTION :: Defines the three  latlonalt coordinates
// **/
struct latlonalt_coordinates {
    CoordinateType latitude;
    CoordinateType longitude;
    CoordinateType altitude;
};

// /**
// STRUCT      :: common_coordinates
// DESCRIPTION :: Defines the three  common coordinates
// **/
struct common_coordinates {
    CoordinateType c1;
    CoordinateType c2;
    CoordinateType c3;
};

// /**
// STRUCT      :: Coordinates
// DESCRIPTION :: Defines coordinates
// **/
// This struct represents a coordinate as a triple of 3
// CoordinateType's and a representation type. The dimensions can be
// referenced by different handles corresponding to different
// representation types.
// Coordinates were previously represented by union_coordinates
// which was the same as the anonymous union in the current coordinates
// and which was typedef'ed to Coordinates in coordinates.h
// Access into Coordinates thus remains backwards compatible
struct Coordinates {
    union {
        // handle to the dimensions corresponding to a Cartesian
        // representation
        // the members are (x,y,z)
        struct cartesian_coordinates cartesian;
        // handle to the dimensions corresponding to a LATLONALT
        // representation
        // the members are (latitude,longitude,altitude)
        struct latlonalt_coordinates latlonalt;
        // generic handle for the dimensions
        // the members are (c1,c2,c3)
        struct common_coordinates common;
    };

    // an enum indicating the coordinate system that the coordinate
    // dimensions are in reference to
    CoordinateRepresentationType type;
    void* customDataPtr;
};


// /**
// STRUCT      :: Orientation
// DESCRIPTION :: Defines the orientation structure
// **/
struct Orientation {
    OrientationType azimuth;
    OrientationType elevation;
};

// /**
// FUNCTION    :: COORD_CoordinatesAreTheSame
// PURPOSE     :: To compare two coordinates and determine
// if they are the same
// PARAMETERS  ::
// + c1 : const Coordinates : coordinates of a point
// + c2 : const Coordinates : coordinates of a point
// RETURN      :: BOOL : whether the points are the same
// **/
static //inline//
BOOL COORD_CoordinatesAreTheSame(
    const Coordinates c1,
    const Coordinates c2) {

    if (c1.common.c1 != c2.common.c1 ||
        c1.common.c2 != c2.common.c2 ||
        c1.common.c3 != c2.common.c3) {

        return FALSE;
    }
    return TRUE;
}

// /**
// FUNCTION    :: COORD_OrientationsAreTheSame
// PURPOSE     :: To compare two coordinates and determine
// if they have the same orientation
// PARAMETERS  ::
// + o1 : const Orientation : orientation of a point
// + o2 : const Orientation : orientation of a point
// RETURN      :: BOOL : whether the points have the same orientation
// **/
static //inline//
BOOL COORD_OrientationsAreTheSame(
    const Orientation o1,
    const Orientation o2) {

    if (o1.azimuth   != o2.azimuth   ||
        o1.elevation != o2.elevation) {

        return FALSE;
    }
    return TRUE;
}

// /**
// API         :: COORD_NormalizeAzimuthAngle
// PURPOSE     :: To normalize the azimuth angle
// PARAMETERS  ::
// + angle : int : azimuth angle
// RETURN      :: static int :
// **/
static //inline//
int COORD_NormalizeAzimuthAngle(int angle) {
    if (angle < 0) {
        while (angle < 0) {
            angle += ANGLE_RESOLUTION;
        }
    }
    if (angle >= ANGLE_RESOLUTION) {
        angle %= ANGLE_RESOLUTION;
    }
    assert(angle >= 0);
    assert(angle < ANGLE_RESOLUTION);

    return angle;
}

// /**
// API         :: COORD_NormalizeElevationAngle
// PURPOSE     :: To normalize the elevation angle
// PARAMETERS  ::
// + angle : int : Angle of elevation
// RETURN      :: static int :
// **/
static //inline//
int COORD_NormalizeElevationAngle(int angle) {
    if (angle <= -ANGLE_RESOLUTION / 2) {
        while (angle <= -ANGLE_RESOLUTION / 2) {
            angle += ANGLE_RESOLUTION;
        }
    }
    if (angle >= ANGLE_RESOLUTION / 2) {
        while (angle >= ANGLE_RESOLUTION / 2) {
            angle -= ANGLE_RESOLUTION;
        }
    }
    assert(angle >= -ANGLE_RESOLUTION / 2);
    assert(angle < ANGLE_RESOLUTION / 2);

    return angle;
}


// /**
// API :: COORD_NormalizeAngleIndex
// PURPOSE :: To normalize the angleIndex
// PARAMETERS ::
// + angleIndex : int: angleIndex
// + angleResolution : int : angleResolution
// RETURN :: static int : Return normalized angleIndex
// **/

static //inline//
int COORD_NormalizeAngleIndex(int angleIndex, int angleResolution) {
    if (angleIndex < 0) {
        while (angleIndex < 0) {
            angleIndex += angleResolution;
        }
    }
    if (angleIndex > angleResolution) {
        angleIndex %= angleResolution;
        angleIndex = angleIndex - 1;
    }
    assert(angleIndex >= 0);
    assert(angleIndex <= angleResolution);

    return angleIndex;
}


// /**
// API         :: COORD_CalcDistance
// PURPOSE     :: To calculate the distance between two nodes(points) given
// the coordinateSystemType and the coordinates of the two points
// PARAMETERS  ::
// + coordinateSystemType : int : type of coordinate system
// + position1 : const Coordinates* : coordinates of a point
// + position2 : const Coordinates* : coordinates of a point
// + distance : CoordinateType : distance between two points
// RETURN      :: BOOL : whether the distance is calculated
//                       from position1 to position2
// **/
BOOL COORD_CalcDistance(
    int coordinateSystemType,
    const Coordinates* position1,
    const Coordinates* position2,
    CoordinateType* distance);


// /**
// API         :: COORD_CalcDistanceAndAngle
// PURPOSE     :: To calculate the Distance and Angle
// PARAMETERS  ::
// + coordinateSystemType : int : coordinateSystem Type
// + position1  : const : Coordinates*
// + position2  : const : Coordinates*
// + distance   : double* : distance
// + DOA1       : Orientation* : DOA 1
// + DOA2       : Orientation* : DOA 2
// RETURN      :: static void : None
// **/
void COORD_CalcDistanceAndAngle(
    int coordinateSystemType,
    const Coordinates* position1,
    const Coordinates* position2,
    double*      distance,
    Orientation* DOA1,
    Orientation* DOA2);

// /**
// API         :: COORD_ChangeCoordinateSystem
// PURPOSE     :: Re-calculate coordinate in a new coordinate system
// PARAMETERS  ::
// + source_type : const CoordinateRepresentationType : coordinate system of
//                                                      point to convert
// + source      : const Coordinates* const : coordinates of point to convert
// + target_type : const CoordinateRepresentationType : coordinate system to
//                                                      convert into
// + target      : Coordinates* const : coordinate in new coordinate system
// RETURN      :: static void : None
// **/
void COORD_ChangeCoordinateSystem(
    const CoordinateRepresentationType source_type,
    const Coordinates* const source,
    const CoordinateRepresentationType target_type,
    Coordinates* const target);

// /**
// API         :: COORD_ChangeCoordinateSystem
// PURPOSE     :: Re-calculate coordinate in a new coordinate system
// PARAMETERS  ::
// + source      : const Coordinates* const : coordinates of point to convert
// + target_type : const CoordinateRepresentationType : coordinate systme to
//                                                      convert into
// + target      : Coordinates* const : coordinate in new coordinate system
// RETURN      :: static void : None
// **/
void COORD_ChangeCoordinateSystem(
    const Coordinates* const source,
    const CoordinateRepresentationType target_type,
    Coordinates* const target);

// /**
// FUNCTION    :: COORD_GeodeticToGeocentricCartesian
// PURPOSE     :: Convert coordinate from GEODETIC to GEOCENTRIC_CARTESIAN
// PARAMETERS  ::
// + source : const Coordinates* const : coordinate in GEODETIC
// + target : Coordinates* const : new coordinate in GEOCENTRIC_CARTESIAN
// RETURN      :: static void : None
// **/
void COORD_GeodeticToGeocentricCartesian(
    const Coordinates* const source,
    Coordinates* const target);

// /**
// FUNCTION    :: COORD_GeocentricCartesianToGeodetic
// PURPOSE     :: Convert coordinate from GEOCENTRIC_CARTESIAN to GEODETIC
// PARAMETERS  ::
// + source : const Coordinates* const : coordinate in GEOCENTRIC_CARTESIAN
// + target : Coordinates* const : new coordinate in GEODETIC
// RETURN      :: static void : None
// **/
void COORD_GeocentricCartesianToGeodetic(
    const Coordinates* const source,
    Coordinates* const target);

// /**
// FUNCTION    :: COORD_JGISToGeodetic
// PURPOSE     :: Convert coordinate from JGIS to GEODETIC
// PARAMETERS  ::
// + source : const Coordinates* const : coordinate in JGIS
// + target : Coordinates* const : new coordinate in GEODETIC
// RETURN      :: static void : None
// **/
void COORD_JGISToGeodetic(
    const Coordinates* const source,
    Coordinates* const target);

// /**
// FUNCTION    :: COORD_JGISToUnreferencedCartesian
// PURPOSE     :: Convert coordinate from JGIS to UNREFERENCED_CARTESIAN
// PARAMETERS  ::
// + source : const Coordinates* const : coordinate in JGIS
// + target : Coordinates* const : new coordinate in UNREFERENCED_CARTESIAN
// RETURN      :: static void : None
// **/
void COORD_JGISToUnreferencedCartesian(
    const Coordinates* const source,
    Coordinates* const target);


// /**
// API         :: COORD_ConvertToCoordinates
// PURPOSE     :: Read the string in "buf" and provide the corresponding
// coordinates for the string.
// PARAMETERS  ::
// + buf : char* : input string to be converted to coordinates
// + coordinates : Coordinates* : Pointer to the coordinates
// RETURN      :: static void : None
// **/
static
void COORD_ConvertToCoordinates(char *buf, Coordinates* coordinates) {
    char localBuffer[MAX_STRING_LENGTH];
    char *stringPtr;

    IO_GetDelimitedToken(localBuffer, buf, "(,) ", &stringPtr);
    if (!stringPtr)
    {
        std::string errorBuf;
        errorBuf += "Found invalid format of configured coordinates: ";
        errorBuf += buf;
        ERROR_ReportError(errorBuf.c_str());
    }
    coordinates->common.c1 = atof(localBuffer);

    IO_GetDelimitedToken(localBuffer, stringPtr, "(,) ", &stringPtr);
    if (!stringPtr)
    {
        std::string errorBuf;
        errorBuf += "Found invalid format of configured coordinates: ";
        errorBuf += buf;
        ERROR_ReportError(errorBuf.c_str());
    }
    coordinates->common.c2 = atof(localBuffer);

    //
    // If the third parameter is omitted, set it to
    // zero.
    //
    if (IO_GetDelimitedToken(localBuffer, stringPtr, "(,) ", &stringPtr) !=
        NULL)
    {
        coordinates->common.c3 = (CoordinateType)atof(localBuffer);
    }
    else {
        coordinates->common.c3 = 0.0;
    }

    return;
}


// /**
// API         :: COORD_MapCoordinateSystemToType
// PURPOSE     :: Set coordinates type field (CoordinateRepresentationType)
// based on the user-provided coordinate system (coordinate_system_type)
// PARAMETERS  ::
// + coordinateSystem : int : enum value indicating coordinate system
// + coordinates      : Coordinates* : Pointer to the coordinates
// RETURN      :: static void : None
// **/
static
void COORD_MapCoordinateSystemToType(
    int coordinateSystem, Coordinates* coordinates) {

    if (coordinateSystem== CARTESIAN) {
        coordinates->type = UNREFERENCED_CARTESIAN;
    }
    else if (coordinateSystem== LATLONALT) {
        coordinates->type = GEODETIC;
    }
    else {
        assert(0);
    }
}

// /**
// API         :: COORD_NormalizeLongitude
// PURPOSE     :: Correct the longitude value to between -180 and 180.
//                This function assumes the coordinate system is LLA.
// PARAMETERS  ::
// + coordinates : Coordinates* : Pointer to the coordinates
// RETURN      :: static void : None
// **/
static
void COORD_NormalizeLongitude(Coordinates* coordinates) {
    if (coordinates->latlonalt.longitude > 180.0) {
        coordinates->latlonalt.longitude -= 360.0;
    }
    else if (coordinates->latlonalt.longitude < -180.0) {
        coordinates->latlonalt.longitude += 360.0;
    }
}


// /**
// API         :: COORD_PointWithinRange
// PURPOSE     :: Is the point within the given range.  Assume -90 <= lat <= 90
//                and -180 <= long <= 180 for all inputs.
// PARAMETERS  ::
// + coordinateSystemType : int : Cartesian or Geodetic
// + sw    : Coordinates* : Pointer to the SW corner (0,0) if Cartesian
// + ne    : Coordinates* : Pointer to the NE corner (dimensions if Cartesian)
// + point : Coordinates* : Pointer to the coordinates
// RETURN      :: bool : True if within range
// **/
static
bool COORD_PointWithinRange(
    const int          coordinateSystemType,
    const Coordinates* sw,
    const Coordinates* ne,
    const Coordinates* point)
{
    if (coordinateSystemType == (int) CARTESIAN) {

        return ((point->cartesian.x >= sw->cartesian.x) &&
                (point->cartesian.x <= ne->cartesian.x) &&
                (point->cartesian.y >= sw->cartesian.y) &&
                (point->cartesian.y <= ne->cartesian.y));
    }
    else // coordinateSystemType == LATLONALT
    {
        // check latitude first
        if ((point->latlonalt.latitude < sw->latlonalt.latitude) ||
            (point->latlonalt.latitude > ne->latlonalt.latitude)) {

            return false;
        }

        // check longitude, assume sw and ne are <= abs(180.0)

        CoordinateType west = sw->latlonalt.longitude;
        CoordinateType east = ne->latlonalt.longitude;

        if (west > east) {

            // sw is west of the dateline, ne is east of it.

            return (((point->latlonalt.longitude >= west) &&
                     (point->latlonalt.longitude <= 180.0)) ||
                    ((point->latlonalt.longitude >= -180.0) &&
                     (point->latlonalt.longitude <= east)));
        }
        else // w < e
        {
            return ((point->latlonalt.longitude >= west) &&
                    (point->latlonalt.longitude <= east));
        }
    }
}


// /**
// API         :: COORD_RegionsOverlap
// PURPOSE     :: Determine whether the given regions overlap at all.
// PARAMETERS  ::
// + coordinateSystemType : int : Cartesian or Geodetic
// + sw1   : Coordinates* : Pointer to the SW corner of the first region
// + ne1   : Coordinates* : Pointer to the NE corner of the first region
// + sw2   : Coordinates* : Pointer to the SW corner of the second region
// + ne2   : Coordinates* : Pointer to the NE corner of the second region
// RETURN  :: bool : true if the regions overlap at all.
// **/
static
bool COORD_RegionsOverlap(
    const int          coordinateSystemType,
    const Coordinates* sw1,
    const Coordinates* ne1,
    const Coordinates* sw2,
    const Coordinates* ne2)
{
    if (coordinateSystemType == (int) CARTESIAN) {
        bool xOverlaps = false;
        bool yOverlaps = false;

        if (sw1->cartesian.x < sw2->cartesian.x) {
            // region 1 starts left of region 2
            xOverlaps = (ne1->cartesian.x >= sw2->cartesian.x);
    }
        else {
            // region 2 starts left of region 1
            xOverlaps = (ne2->cartesian.x >= sw1->cartesian.x);
        }

        if (!xOverlaps) {
            return false;
        }

        if (sw1->cartesian.y < sw2->cartesian.y) {
            // region 1 starts below region 2
            yOverlaps = (ne1->cartesian.y >= sw2->cartesian.y);
    }
    else {
            // region 2 starts below region 1
            yOverlaps = (ne2->cartesian.y >= sw1->cartesian.y);
        }

        return yOverlaps;
    }
    else // coordinateSystemType == LATLONALT
    {
        bool latitudeOverlaps = false;

        // check latitude first, that's easy.
        if (sw1->latlonalt.latitude < sw2->latlonalt.latitude) {
            // region 1 starts south of region 2
            latitudeOverlaps = (ne1->latlonalt.latitude >=
                                sw2->latlonalt.latitude);
        }
        else {
            // region 2 starts south of region 1
            latitudeOverlaps = (ne2->latlonalt.latitude >=
                                sw1->latlonalt.latitude);
    }

        if (!latitudeOverlaps) {
            return false;
        }

        // check longitude
        // if either west is between the other east/west, return true
        // there must be a more concise way of expressing this.

        CoordinateType west1 = sw1->latlonalt.longitude;
        CoordinateType west2 = sw2->latlonalt.longitude;
        CoordinateType east1 = ne1->latlonalt.longitude;
        CoordinateType east2 = ne2->latlonalt.longitude;

        // check whether west2 is between west1 and east1
        if (west1 <= east1) {
            if ((west2 >= west1) && (west2 <= east1)) {
                return true;
            }
        }
        else { // region1 crosses the date line
            if (west2 >= west1) {
                // they must overlap because west2 is still west of the
                // date line, but east1 is east of it.
                return true;
            }
            else { // west2 < west1
                if (west2 <= east1) {
                    // here west2 is east of the date line, but west of east1
                    return true;
                }
            }
        }

        // now check whether west1 is between west2 and east2
        if (west2 <= east2) {
            if ((west1 >= west2) && (west1 <= east2)) {
                return true;
}
        }
        else { // region1 crosses the date line
            if (west1 >= west2) {
                // they must overlap because west1 is still west of the
                // date line, but east2 is east of it.
                return true;
            }
            else { // west1 < west2
                if (west1 <= east2) {
                    // here west1 is east of the date line, but west of east2
                    return true;
                }
            }
        }
    }

    // if we get this far, all the checks failed.
    return false;
}

// /**
// API         :: COORD_LongitudeDelta
// PURPOSE     :: Convenience function for geodetic that, given two longitudes,
//                returns the difference (in degrees) in the shorter direction.
// PARAMETERS  ::
// + long1 : CoordinateType : coordinate 1
// + long2 : CoordinateType : coordinate 2
// RETURN      :: void : 
// **/
static
CoordinateType COORD_LongitudeDelta(
    const CoordinateType long1,
    const CoordinateType long2) {

    // Assuming values are in range -180.0 .. 180.
    CoordinateType delta = long1 - long2;
    if (delta > 180.0) {
        // invert
        delta -= 360.0;
    }
    else if (delta < -180.0) {
        delta += 360.0;
    }
    return delta;
}
// /**
// API         :: COORD_PrintCoordinates
// PURPOSE     :: Prints the coordinates in a human readable format.
// PARAMETERS  ::
// + coordinateSystemType : int : Cartesian or Geodetic
// + point : Coordinates* : Pointer to the coordinates
// RETURN      :: void : 
// **/
void COORD_PrintCoordinates(
    const Coordinates* point,
    const int          coordinateSystemType);

#endif /*COORDINATES_H*/
