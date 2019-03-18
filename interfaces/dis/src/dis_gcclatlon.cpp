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

#include <iostream>
using std::cerr;
using std::endl;
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cassert>

#include "dis_gcclatlon.h"
#include "dis_shared.h"

extern Dis g_dis;

const double DIS_PI = 3.14159265358979323846264338328;

const double DIS_RADIANS_PER_DEGREE = (DIS_PI / 180.0);
const double DIS_DEGREES_PER_RADIAN = (180.0 / DIS_PI);

// WGS84 ellipsoid equatorial radius (a, in meters) and flattening (f).

const double WGS84_ELLIPSOID_A_VALUE = 6378137.0;
const double WGS84_ELLIPSOID_F_VALUE = (1.0 / 298.257223560);

void
DisConvertGccToLatLonAlt(
    double x,
    double y,
    double z,
    double& lat,
    double& lon,
    double& alt)
{
    // Convert from geocentric Cartesian (x, y, z, in meters) to geodetic
    // coordinates (lat, lon in degrees; alt, in meters above ellipsoid)
    // using WGS84 ellipsoid.

    if (x == 0 || y == 0 || z == 0)
    {
        cerr << "error: Can't determine latlon coordinates if"
             << " x, y, or z are zero" << endl;
        exit(EXIT_FAILURE);
    }

    // Find longitude.
    // (Done by handling each situation for the quadrants of a circle.)

    if (x > 0 && y > 0)
    {
        lon = atan(y / x) * DIS_DEGREES_PER_RADIAN;
    }
    else
    if (x < 0 && y > 0)
    {
        lon = 90.0 + (atan(-x / y) * DIS_DEGREES_PER_RADIAN);
    }
    else
    if (x > 0 && y < 0)
    {
        lon = atan(y / x) * DIS_DEGREES_PER_RADIAN;
    }
    else
    if (x < 0 && y < 0)
    {
        lon = -90.0 - (atan(x / y) * DIS_DEGREES_PER_RADIAN);
    }
    else
    {
        cerr << "error: Should not reach this statement" << endl;
        exit(EXIT_FAILURE);
    }

    // Find latitude and height above ellipsoid.

    // The non-iterative algorithm below determines an exact solution.
    // "Transformation of Geocentric to Geodetic Coordinates Without
    // Approximations"
    // Astrophysics and Space Sciences, 139 (1987), 1-4
    // Erratum: vol. 146, (No. 1, July 1988), 201
    // http://www.astro.uni.torun.pl/~kb/Papers/ASS/Geod-ASS.htm
    // http://www.astro.uni.torun.pl/~kb/Papers/geod/Geod-GK.htm#GEOD

    double a = WGS84_ELLIPSOID_A_VALUE;
    double f = WGS84_ELLIPSOID_F_VALUE;

    double r = sqrt((x * x) + (y * y));

    double b = fabs(a - (a * f));
    if (z < 0.0) { b *= -1.0; }

    double E = ((z + b) * (b / a) - a) / r;
    double F = ((z - b) * (b / a) + a) / r;
    double P = ((E * F) + 1.0) * (4.0 / 3.0);
    double Q = ((E * E) - (F * F)) * 2.0;
    double D = (P * P * P) + (Q * Q);

    double s;
    double v;

    if (D > 0.0)
    {
       s = sqrt(D) + Q;
       s = fabs(exp(log(fabs(s)) / 3.0));
       if (s < 0.0) { s *= -1.0; }

       v = (P / s) - s;
       v = -(Q + Q + (v * v * v)) / (3 * P);
    }
    else
    {
       v = 2.0 * sqrt(-P) * cos(acos(Q / P / sqrt(-P)) / 3.0);
    }

    double G = 0.5 * (E + sqrt((E * E) + v));

    double t = sqrt((G * G) + (F - (v * G)) / (G + G -E))
               - G;

    // lat is calculated first in radians, because it's necessary
    // for the alt calculation.  After that, lat is converted to
    // degrees.

    lat = atan((1.0 - (t * t))
               * a / (2 * b * t));
    alt = ((r - (a * t)) * cos(lat))
           + (z - b) * sin(lat) + g_dis.altitudeOffset;

    lat *= DIS_DEGREES_PER_RADIAN;

    assert(lat >= -90.0 && lat <= 90.0);
    assert(lon >= -180.0 && lon <= 180.0);
}

void
DisConvertLatLonAltToGcc(
    double lat,
    double lon,
    double alt,
    double& x,
    double& y,
    double& z)
{
    // Convert from geodetic (lat, lon in degrees; alt, in meters above
    // ellipsoid) to geocentric Cartesian coordinates (x, y, z, in meters)
    // using WGS84 ellipsoid.

    // Algorithm below from:
    // "Transformation of Geocentric to Geodetic Coordinates Without
    // Approximations"
    // Astrophysics and Space Sciences, 139 (1987), 1-4
    // Erratum: vol. 146, (No. 1, July 1988), 201
    // http://www.astro.uni.torun.pl/~kb/Papers/ASS/Geod-ASS.htm
    // http://www.astro.uni.torun.pl/~kb/Papers/geod/Geod-GK.htm#GEOD

    assert(lat >= -90.0 && lat <= 90.0);
    assert(lon >= -180.0 && lon <= 180.0);

    alt -= g_dis.altitudeOffset;

    // Find latitude and height above ellipsoid.

    double a = WGS84_ELLIPSOID_A_VALUE;
    double f = WGS84_ELLIPSOID_F_VALUE;

    double latRadians = lat * DIS_RADIANS_PER_DEGREE;

    double b = a * (1 - f);
    double term1 = a * cos(latRadians);
    double term2 = b * sin(latRadians);
    double C = (a * a)
               / sqrt((term1 * term1) + (term2 * term2));

    z = (((C * b * b) / (a * a)) + alt)
         * sin(latRadians);

    double r = (C + alt) * cos(latRadians);
    double lonRadians = lon * DIS_RADIANS_PER_DEGREE;

    x = r * cos(lonRadians);
    y = r * sin(lonRadians);
}
