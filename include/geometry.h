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
// PACKAGE     :: GEOMETRY
// DESCRIPTION :: This file describes basic geometrical shapes and related
//                functions.  It's basically a placeholder until we pick
//                a good 3rd party package to use.  These shapes should
//                be used for buildings, weather, and other 3D shapes.
//                All math is done in Cartesian space.
// **/

#ifndef SNT_GEOMETRY_H
#define SNT_GEOMETRY_H

#include <list>
#include <vector>

#include "coordinates.h"

struct Box {
public:
    Coordinates lower; // contains the lower X,Y
    Coordinates upper; // contains the upper X,Y

    Box() {
        memset(&lower, 0, sizeof(Coordinates));
        memset(&upper, 0, sizeof(Coordinates));
    }
    Box(const Coordinates ll,
        const Coordinates ur) {
        lower = ll;
        upper = ur;
    }
    Box(const Coordinates* ll,
        const Coordinates* ur) {
        lower = *ll;
        upper = *ur;
    }
    Coordinates getLowerLeft() const { return lower; }
    Coordinates getLowerRight() const {
        Coordinates lr;
        lr.common.c1 = upper.common.c1;
        lr.common.c2 = lower.common.c2;
        return lr;
    }
    Coordinates getUpperLeft() const {
        Coordinates ul;
        ul.common.c1 = lower.common.c1;
        ul.common.c2 = upper.common.c2;
        return ul;
    }
    Coordinates getUpperRight() const { return upper; }
    double getArea(int coordinateSystem) const;

    double& x() { return lower.cartesian.x; }
    double& y() { return lower.cartesian.y; }
    double& X() { return upper.cartesian.x; }
    double& Y() { return upper.cartesian.y; }

    void setMaxX(double x) { upper.cartesian.x = x; }
    void setMaxY(double y) { upper.cartesian.y = y; }
    void setMinX(double x) { lower.cartesian.x = x; }
    void setMinY(double y) { lower.cartesian.y = y; }

    double getMaxX() const { return upper.cartesian.x; }
    double getMaxY() const { return upper.cartesian.y; }
    double getMinX() const { return lower.cartesian.x; }
    double getMinY() const { return lower.cartesian.y; }

    virtual void print() const;
};

//! Cube
// \brief A cube is a 3D bounding cube.
struct Cube : public Box {
public:
    Cube() : Box() {  }
    Cube(const Coordinates ll,
         const Coordinates ur) : Box(ll, ur) { }
    Cube(const Coordinates* ll,
         const Coordinates* ur) : Box(ll, ur) { }
    void print() const;

    double& z() { return lower.cartesian.z; }
    double& Z() { return upper.cartesian.z; }

    double getMaxZ() const { return upper.cartesian.z; }
    double getMinZ() const { return lower.cartesian.z; }

    void setMaxZ(double z) { upper.cartesian.z = z; }
    void setMinZ(double z) { lower.cartesian.z = z; }
};

struct LineSegment {
    Coordinates point1;
    Coordinates point2;
};

//! Polygon
// \brief A 2D polygon
struct Polygonl {
    int numPoints;
    std::vector<Coordinates> points;
    Box boundingBox;
};

//! Object3D
// \brief A 3D object, such as a building or weather pattern.
struct Object3D {
    int numFaces;
    std::vector<Polygonl> faces;
    Cube boundingCube;
};

//! Geometry
// \brief CartesianGeometry class is used to provide static math functions on
//        geometric shapes in Cartesian space.
class Geometry {
public:
    static bool contains(const Box*         box,
                         const Coordinates* point) {
        return COORD_PointWithinRange(CARTESIAN,
                                      &(box->lower),
                                      &(box->upper),
                                      point);
    }
    static bool contains(const Polygonl*     polygon,
                         const Coordinates* point);
    static bool contains(const Cube*        cube,
                         const Coordinates* point) {
        return (COORD_PointWithinRange(CARTESIAN,
                                      &(cube->lower),
                                      &(cube->upper),
                                      point) &&
                (cube->lower.cartesian.z <= point->cartesian.z) &&
                (cube->upper.cartesian.z >= point->cartesian.z));
    }
    static bool overlaps(const Box* rect1,
                         const Box* rect2);
    static bool intersection(const LineSegment* line1,
                             const LineSegment* line2,
                             Coordinates*       intersection);
    static bool intersection(const LineSegment* line,
                             const Box* plane,
                             Coordinates*       intersection);
    static bool intersection(const LineSegment* line,
                             const Polygonl*     shape,
                             Coordinates*       intersection);
    static std::list<Coordinates> intersections(const LineSegment* line,
                                                const Cube* cube);
    static std::list<Coordinates> intersections(const LineSegment* line,
                                                const Object3D* object);
};

#endif // SNT_GEOMETRY_H
