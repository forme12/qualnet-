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

#include <cmath>
#include <cassert>

#include "hla_archspec.h"
#include "hla_orientation.h"

const double HLA_PI = 3.14159265358979323846264338328;

const double HLA_RADIANS_PER_DEGREE = (HLA_PI / 180.0);
const double HLA_DEGREES_PER_RADIAN = (180.0 / HLA_PI);

void
HlaConvertRprFomOrientationToQualNetOrientation(
    double lat,
    double lon,
    float float_psiRadians,
    float float_thetaRadians,
    float float_phiRadians,
    short& azimuth,
    short& elevation)
{
    // Notes: -----------------------------------------------------------------

    // This function converts a DIS / RPR-FOM 1.0 orientation into QualNet
    // azimuth and elevation angles.
    //
    // When the entity is located exactly at the north pole, this function
    // will return an azimuth of 0 (facing north) when the entity is pointing
    // toward 180 degrees longitude.
    //
    // When the entity is located exactly at the south pole, this function
    // will return an azimuth of 0 (facing north) when the entity is pointing
    // toward 0 degrees longitude.
    //
    // This function has not been optimized at all, so the calculations are
    // more readily understood.
    // (e.g., the phi angle doesn't affect the results, some vector components
    // end up being multipled by 0, etc.)

    assert(lat >= -90.0 && lat <= 90.0);
    assert(lon >= -180.0 && lon <= 180.0);

    // Introduction: ----------------------------------------------------------

    // There are two coordinate systems considered:
    //
    // (1) the GCC coordinate system, and
    // (2) the entity coordinate system.
    //
    // Both are right-handed coordinate systems.
    //
    // The GCC coordinate system has well-defined axes.
    // In the entity coordinate system, the x-axis points in the direction the
    // entity is facing (the entity orientation).
    //
    // psi, theta, and phi are the angles by which one rotates the GCC axes
    // so that they match in direction with the entity axes.
    //
    // Start with the GCC axes, and rotate them in the following order:
    //
    // (1) psi, a right-handed rotation about the z-axis
    // (2) theta, a right-handed rotation about the new y-axis
    // (3) phi, a right-handed rotation about the new x-axis

    double psiRadians   = (double) float_psiRadians;
    double thetaRadians = (double) float_thetaRadians;
    double phiRadians   = (double) float_phiRadians;

    // Convert latitude and longitude into a unit vector.
    // If one imagines the earth as a unit sphere, the vector will point
    // to the location of the entity on the surface of the sphere.

    double latRadians = lat * HLA_RADIANS_PER_DEGREE;
    double lonRadians = lon * HLA_RADIANS_PER_DEGREE;

    double entityLocationX = cos(lonRadians) * cos(latRadians);
    double entityLocationY = sin(lonRadians) * cos(latRadians);
    double entityLocationZ = sin(latRadians);

    // Discussion of basic theory: --------------------------------------------

    // Start with a point b in the initial coordinate system.  b is represented
    // as a vector.
    //
    // Rotate the axes of the initial coordinate system by angle theta to
    // obtain a new coordinate system.
    //
    // Representation of point b in the new coordinate system is given by:
    //
    // c = ab
    //
    // where a is a rotation matrix:
    //
    // a = [ cos(theta)        sin(theta)
    //       -sin(theta)       cos(theta) ]
    //
    // and c is the same point, but in the new coordinate system.
    //
    // Note that the coordinate system changes; the point itself does not move.
    // Also, matrix a is for rotating the axes; the matrix is different when
    // rotating the vector while not changing the axes.
    //
    // Applying this discussion to three dimensions, and using psi, theta, and
    // phi as described previously, three rotation matrices can be created:
    //
    // Rx =
    // [ 1           0           0
    //   0           cos(phi)    sin(phi)
    //   0           -sin(phi)   cos(phi) ]
    //
    // Ry =
    // [ cos(theta)  0           -sin(theta)
    //   0           1           0
    //   sin(theta)  0            cos(theta) ]
    //
    // Rz =
    // [ cos(psi)    sin(psi)    0
    //   -sin(psi)   cos(psi)    0
    //   0           0           1 ]
    //
    // where
    //
    // c = ab
    // a = (Rx)(Ry)(Rz)
    //
    // b is the point as represented in the GCC coordinate system;
    // c is the point as represented in the entity coordinate system.
    //
    // Note that matrix multiplication is not commutative, so the order of
    // the factors in a is important (rotate by Rz first, so it's on the right
    // side).

    // Determine elevation angle: ---------------------------------------------

    // In the computation of the elevation angle below, the change is in the
    // opposite direction, from the entity coordinate system to the GCC system.
    // So, for:
    //
    // c = ab
    //
    // Vector b represents the entity orientation as expressed in the entity
    // coordinate system.
    // Vector c represents the entity orientation as expressed in the GCC
    // coordinate system.
    //
    // It turns out that:
    //
    // a = (Rz)'(Ry)'(Rx)'
    //
    // where Rx, Ry, and Rz are given earlier, and the ' symbol indicates the
    // transpose of each matrix.
    //
    // The ordering of the matrices is reversed, since one is going from the
    // entity coordinate system to the GCC system.  The negative of psi, theta,
    // and phi angles are used, and the transposed matrices end up being the
    // correct ones.

    double a11 = cos(psiRadians) * cos(thetaRadians);
    double a12 = -sin(psiRadians) * cos(phiRadians)
                 + cos(psiRadians) * sin(thetaRadians) * sin(phiRadians);
    double a13 = -sin(psiRadians) * -sin(phiRadians)
                 + cos(psiRadians) * sin(thetaRadians) * cos(phiRadians);

    double a21 = sin(psiRadians) * cos(thetaRadians);
    double a22 = cos(psiRadians) * cos(phiRadians)
                 + sin(psiRadians) * sin(thetaRadians) * sin(phiRadians);
    double a23 = cos(psiRadians) * -sin(phiRadians)
                 + sin(psiRadians) * sin(thetaRadians) * cos(phiRadians);

    double a31 = -sin(thetaRadians);
    double a32 = cos(thetaRadians) * sin(phiRadians);
    double a33 = cos(thetaRadians) * cos(phiRadians);

    // Vector b is chosen such that it is the unit vector pointing along the
    // positive x-axis of the entity coordinate system.  I.e., vector b points
    // in the same direction the entity is facing.

    double b1 = 1.0;
    double b2 = 0.0;
    double b3 = 0.0;

    // The values below are the components of vector c, which represent the
    // entity orientation in the GCC coordinate system.

    double entityOrientationX = a11 * b1 + a12 * b2 + a13 * b3;
    double entityOrientationY = a21 * b1 + a22 * b2 + a23 * b3;
    double entityOrientationZ = a31 * b1 + a32 * b2 + a33 * b3;

    // One now has two vectors:
    //
    // (1) an entity-position vector, and
    // (2) an entity-orientation vector.
    //
    // Note that the position vector is normal to the sphere at the point where
    // the entity is located on the sphere.
    //
    // One can determine the angle between the two vectors using dot product
    // formulas.  The computed angle is the deflection from the normal.

    double dotProduct
        = entityLocationX * entityOrientationX
          + entityLocationY * entityOrientationY
          + entityLocationZ * entityOrientationZ;

    double entityLocationMagnitude
        = sqrt(pow(entityLocationX, 2)
               + pow(entityLocationY, 2)
               + pow(entityLocationZ, 2));

    double entityOrientationMagnitude
        = sqrt(pow(entityOrientationX, 2)
               + pow(entityOrientationY, 2)
               + pow(entityOrientationZ, 2));

    double deflectionFromNormalToSphereRadians
        = acos(dotProduct / (entityLocationMagnitude
                             * entityOrientationMagnitude));

    // The elevation angle is 90 degrees minus the angle for deflection from
    // normal.  (Note that the elevation angle can range from -90 to +90
    // degrees.)

    double elevationRadians
        = (HLA_PI / 2.0) - deflectionFromNormalToSphereRadians;

    elevation = (short) HlaRint(elevationRadians * HLA_DEGREES_PER_RADIAN);

    assert(elevation >= -90 && elevation <= 90);

    // Determine azimuth angle: -----------------------------------------------

    // To determine the azimuth angle, for:
    //
    // c = ab
    //
    // b is the entity orientation as represented in the GCC coordinate system.
    //
    // c is the entity orientation as expressed in a new coordinate system.
    // This is a coordinate system where the yz plane is tangent to the sphere
    // at the point on the sphere where the entity is located.  The z-axis
    // points towards true north; the y-axis points towards east; the x-axis
    // points in the direction of the normal to the sphere.
    //
    // The rotation matrix turns is then:
    //
    // a = (Ry)(Rz)
    //
    // where longitude is used for Rz and the negative of latitude is used for
    // Ry (since right-handed rotations of the y-axis are positive).

    a11 = cos(-latRadians) * cos(lonRadians);
    a12 = cos(-latRadians) * sin(lonRadians);
    a13 = -sin(-latRadians);

    a21 = -sin(lonRadians);
    a22 = cos(lonRadians);
    a23 = 0;

    a31 = sin(-latRadians) * cos(lonRadians);
    a32 = sin(-latRadians) * sin(lonRadians);
    a33 = cos(-latRadians);

    b1 = entityOrientationX;
    b2 = entityOrientationY;
    b3 = entityOrientationZ;

    // Variable unused.
    //double c1 = a11 * b1 + a12 * b2 + a13 * b3;

    double c2 = a21 * b1 + a22 * b2 + a23 * b3;
    double c3 = a31 * b1 + a32 * b2 + a33 * b3;

    // To determine azimuth, project c against the yz plane (the plane tangent
    // to the sphere at the entity location), creating a new vector without an
    // x component.
    //
    // Determine the angle between this vector and the unit vector pointing
    // north, using dot-product formulas.

    double vectorInTangentPlaneX = 0;
    double vectorInTangentPlaneY = c2;
    double vectorInTangentPlaneZ = c3;

    double northVectorInTangentPlaneX = 0;
    double northVectorInTangentPlaneY = 0;
    double northVectorInTangentPlaneZ = 1;

    dotProduct
        = vectorInTangentPlaneX * northVectorInTangentPlaneX
          + vectorInTangentPlaneY * northVectorInTangentPlaneY
          + vectorInTangentPlaneZ * northVectorInTangentPlaneZ;

    double vectorInTangentPlaneMagnitude
        = sqrt(pow(vectorInTangentPlaneX, 2)
               + pow(vectorInTangentPlaneY, 2)
               + pow(vectorInTangentPlaneZ, 2));

    double northVectorInTangentPlaneMagnitude
        = sqrt(pow(northVectorInTangentPlaneX, 2)
               + pow(northVectorInTangentPlaneY, 2)
               + pow(northVectorInTangentPlaneZ, 2));

    double azimuthRadians
        = acos(dotProduct / (vectorInTangentPlaneMagnitude
                             * northVectorInTangentPlaneMagnitude));

    // Handle azimuth values between 180 and 360 degrees.

    if (vectorInTangentPlaneY < 0.0)
    {
        azimuthRadians = (2.0 * HLA_PI) - azimuthRadians;
    }

    azimuth = (short) HlaRint(azimuthRadians * HLA_DEGREES_PER_RADIAN);
    if (azimuth == 360) { azimuth = 0; }

    assert(azimuth >= 0 && azimuth <= 359);
}
