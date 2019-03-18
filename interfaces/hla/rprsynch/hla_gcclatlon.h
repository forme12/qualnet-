// Copyright (c) 2001-2008, Scalable Network Technologies, Inc.  All Rights Reserved.
//                          6100 Center Drive West
//                          Suite 1250 
//                          Los Angeles, CA 90045
//                          sales@scalable-networks.com
//
// This source code is licensed, not sold, and is subject to a written
// license agreement.  Among other things, no portion of this source
// code may be copied, transmitted, disclosed, displayed, distributed,
// translated, used as the basis for a derivative work, or used, in
// whole or in part, for any program or purpose other than its intended
// use in compliance with the license agreement as part of the EXata
// software.  This source code and certain of the algorithms contained
// within it are confidential trade secrets of Scalable Network
// Technologies, Inc. and may not be used as the basis for any other
// software, hardware, product or service.

#ifndef HLA_GCCLATLON_H
#define HLA_GCCLATLON_H

void
HlaConvertGccToLatLonAlt(
    double x,
    double y,
    double z,
    double& lat,
    double& lon,
    double& alt);

void
HlaConvertLatLonAltToGcc(
    double lat,
    double lon,
    double alt,
    double& x,
    double& y,
    double& z);

#endif /* HLA_GCCLATLON_H */
