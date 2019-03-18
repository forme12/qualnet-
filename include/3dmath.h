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
// PACKAGE     :: 3D_MATH
// DESCRIPTION :: This file describes data structures and functions used to model 3D weather patterns in conjunction
//                with the Weather package.
// **/

#ifndef _QUALNET_3DMATH_H
#define _QUALNET_3DMATH_H

#include <math.h>
#include <main.h>

#define MARGIN_OF_ERROR 0.001

// /**
// STRUCT      :: Vector3
// DESCRIPTION :: This is used to hold 3D points and vectors.  This will
//                eventually be added upon to create a robust class with
//                operator overloading.  For now we just need an x, y, z.
// **/
struct Vector3 {
public:

    Vector3() {
        x = 0.0; y = 0.0; z = 0.0;
    }

    Vector3(double X, double Y, double Z) {
        x = X; y = Y; z = Z;
    }

    Vector3 operator+(Vector3 vector) {
        return Vector3(vector.x + x, vector.y + y, vector.z + z);
    }

    Vector3 operator-(Vector3 vector) {
        return Vector3(x - vector.x, y - vector.y, z - vector.z);
    }

    Vector3 operator*(double num) {
        return Vector3(x * num, y * num, z * num);
    }

    Vector3 operator/(double num) {
        return Vector3(x / num, y / num, z / num);
    }

    void operator+=(Vector3 vector) {
        x += vector.x; y += vector.y; z += vector.z;
    }

    void operator-=(Vector3 vector) {
        x -= vector.x; y -= vector.y; z -= vector.z;
    }
    bool operator==(Vector3 vector)
    {
        return ((fabs(x - vector.x) < MARGIN_OF_ERROR) &&
                (fabs(y - vector.y) < MARGIN_OF_ERROR) &&
                (fabs(z - vector.z) < MARGIN_OF_ERROR));
    }
    double x;
    double y;
    double z;
};

// /**
// STRUCT      :: Triangle3
// DESCRIPTION :: This struture will hold information for one triangle.
// **/
struct Triangle3 {
    Vector3 sides[3];
};


// /**
// FUNCTION        :: MATH_CrossProduct
// PURPOSE         :: Returns a perpendicular vector from 2 given
//                    vectors by taking the cross product.
// PARAMETERS      ::
// + vector1 : Vector3 : the first vector
// + vector2 : Vector3 : the second vector
// RETURN          :: Vector3 : the cross product
Vector3 MATH_CrossProduct(Vector3 vector1,
                          Vector3 vector2);

// /**
// FUNCTION        :: MATH_Vector
// PURPOSE         :: Returns a vector between 2 points
// PARAMETERS      ::
// + point1 : Vector3 : the first point
// + point2 : Vector3 : the second point
// RETURN          :: Vector3   : a vector between the two points
Vector3 MATH_Vector(Vector3 point1,
                    Vector3 point2);

// /**
// FUNCTION        :: MATH_Magnitude
// PURPOSE         :: Returns the magnitude of a normal (or any other vector)
// PARAMETERS      ::
// + vector : Vector3 : a vector
// RETURN          :: double : the magnitude of the vector
double MATH_Magnitude(Vector3 vector);

// /**
// FUNCTION        :: MATH_Normalize
// PURPOSE         :: Returns a normalized vector (of exactly length 1)
// PARAMETERS      ::
// + vector : Vector3 : a vector
// RETURN          :: Vector3   : a normalized vector
Vector3 MATH_Normalize(Vector3 vector);

// /**
// FUNCTION        :: MATH_Normal
// PURPOSE         :: Returns the direction the polygon is facing
// PARAMETERS      ::
// + triangle : Vector3[] : an array of vectors representing a polygon
// RETURN          :: Vector3 : the direction vector
Vector3 MATH_Normal(Vector3 triangle[]);

// /**
// FUNCTION        :: MATH_PlaneDistance
// PURPOSE         :: Returns the distance the plane is from the origin
//                    (0, 0, 0).  It takes the normal to the plane, along
//                     with ANY point that lies on the plane (any corner)
// PARAMETERS      ::
// + vector : Vector3 : a vector
// + point  : Vector3 : a point
// RETURN          :: double : the plane's distance from the origin (0,0,0)
double MATH_PlaneDistance(Vector3 vector,
                         Vector3 point);

// /**
// FUNCTION        :: MATH_IntersectedPlane
// PURPOSE         :: Takes a triangle (plane) and line and returns true
//                    if they intersected
// PARAMETERS      ::
// + polygon        : Vector3[] : a polygon
// + line           : Vector3[] : a line
// + normal         : Vector3&  : a normalized vector
// + originDistance : double&    : the distance
// RETURN          :: BOOL : True if they intersect
BOOL MATH_IntersectedPlane(Vector3  polygon[],
                           Vector3  line[],
                           Vector3& normal,
                           double&   originDistance);

// /**
// FUNCTION        :: MATH_DotProduct
// PURPOSE         :: Returns the dot product between 2 vectors.
// PARAMETERS      ::
// + vector1 : Vector3 : the first vector
// + vector2 : Vector3 : the second vector
// RETURN          :: double : the dot product of the two vectors
double MATH_DotProduct(Vector3 vector1,
                      Vector3 vector2);

// /**
// FUNCTION        :: MATH_AngleBetweenVectors
// PURPOSE         :: This returns the angle between 2 vectors
// PARAMETERS      ::
// + vector1 : Vector3 : the first vector
// + vector2 : Vector3 : the second vector
// RETURN          :: double :
double MATH_AngleBetweenVectors(Vector3 vector1,
                                Vector3 vector2);

// /**
// FUNCTION        :: MATH_IntersectionPoint
// PURPOSE         :: Returns an intersection point of a polygon and a line
//                    (assuming intersects the plane)
// PARAMETERS      ::
// + normal   : Vector3   : a polygon
// + line     : Vector3[] : a line
// + distance : double    : the distance between?
// RETURN          :: Vector3 :
Vector3 MATH_IntersectionPoint(Vector3 normal,
                               Vector3 line[],
                               double  distance);

// /**
// FUNCTION        :: MATH_InsidePolygon
// PURPOSE         :: Returns true if the intersection point is inside of
//                    the polygon
// PARAMETERS      ::
// + intersection : Vector3   : an intersection point
// + polygon      : Vector3[] : a polygon
// + verticeCount : int       : number of points in polygon
// RETURN          :: BOOL : True if the intersection point is in the polygon
BOOL MATH_InsidePolygon(Vector3 intersection,
                        Vector3 polygon[],
                        int     verticeCount);

// /**
// FUNCTION        :: MATH_IntersectedPolygon
// PURPOSE         :: Tests collision between a line and polygon
// PARAMETERS      ::
// + polygon      : Vector3[] : a polygon
// + line         : Vector3[] : a line
// + verticeCount : int       : number of points in polygon
// RETURN          :: BOOL : True if the polygon and line intersect
BOOL MATH_IntersectedPolygon(Vector3 polygon[],
                             Vector3 line[],
                             int     verticeCount);

// /**
// FUNCTION        :: MATH_Distance
// PURPOSE         :: Returns the distance between 2 3D points
// PARAMETERS      ::
// + point1 : Vector3 : the first point
// + point2 : Vector3 : the second point
// RETURN          :: double : the distance between the two points
double MATH_Distance(Vector3 point1,
                     Vector3 point2);

// /**
// FUNCTION        :: MATH_LineIntersects
// PURPOSE         :: Checks whether two lines intersect each other or not.
// PARAMETERS      ::
// + line1 : Vector3[] : the first line
// + line2 : Vector3[] : the second line
// RETURN          :: BOOL : True if the lines intersect
BOOL MATH_LineIntersects(Vector3 line1[],
                         Vector3 line2[]);

// /**
// FUNCTION        :: MATH_ReturnLineToLineIntersectionPoint
// PURPOSE         :: Returns the point of intersection between two lines.
// PARAMETERS      ::
// + line1 : Vector3[] : the first line
// + line2 : Vector3[] : the second line
// RETURN          :: Vector3 : the intersection point
Vector3 MATH_ReturnLineToLineIntersectionPoint(Vector3 line1[],
                                               Vector3 line2[]);

// /**
// FUNCTION        :: MATH_IsPointOnLine
// PURPOSE         :: Returns the whether the given point lies on Line or not.
// PARAMETERS      ::
// + point : Vector3   : the point which we are checking.
// + line  : Vector3[] : the line on which the point might lie.
// RETURN :: BOOL : TRUE if the point lies on line
BOOL MATH_IsPointOnLine(Vector3 point,
                        Vector3 line[]);


// /**
// FUNCTION         :: MATH_ConvertXYToLatLong
// PURPOSE          :: Converts given cartesian coordinates to Latitide and Longitude
// PARAMETERS       ::
// + x1  : double: Specifies X value on X-Axis
// + y1  : double: Specifies Y value on Y-Axis
// + latitude : double: Will store the converted latitude value
// + longitude : double: Will store the converted longitude value
// RETURN :: void : NULL
void MATH_ConvertXYToLatLong(double x1,
                             double y1,
                             double *latitude,
                             double *longitude);

#endif
