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

#ifndef _ARCH_SPEC_H_
#define _ARCH_SPEC_H_

#include <string>
#include <math.h>
inline int roundToInt(double x)
{
    return x<0.0?(int)ceil(x-0.5):(int)floor(x+0.5);
}

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <direct.h>

#define DIR_SEP "\\"
/*
HLAASCIIchar
HLAfloat32BE
HLAfloat64BE
HLAinteger16BE
HLAinteger32BE */
typedef __int64 HLAinteger64BE;
typedef unsigned __int64 UnsignedInteger64BE;
/* HLAoctet
HLAoctetPairBE
UnsignedInteger16BE
UnsignedInteger32BE
HLAASCIIstring
HLAunicodeString
*/

inline std::string fullpath(const std::string& path)
{
    char *p = _fullpath(0, path.c_str(), 0);
    if( p )
        return std::string(p);
    else
        return std::string("");
}

#else
#include <sys/stat.h>
#define DIR_SEP "/"
typedef uint64_t UnsignedInteger64BE;
inline int _mkdir(const char* path) { return mkdir(path,0775); }
inline int Sleep(int msec) { return usleep(msec*1000); }
inline std::string fullpath(const std::string& path)
{
    char buffer[PATH_MAX+1];
    realpath( path.c_str(), buffer );
    return std::string(buffer);
}
#endif

#ifdef QT_CORE_LIB
#include <QFile>
#include <QTextStream>
#ifdef DIR_SEP
#undef DIR_SEP
#endif
#define DIR_SEP "/"
#endif

#endif

