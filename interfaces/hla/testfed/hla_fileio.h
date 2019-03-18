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

// This file contains only a subset of the functions available in the
// actual QualNet 3.5.2 fileio.h.

#ifndef FILEIO_H
#define FILEIO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//-----------------------------------------------------------------------------
// FUNCTION     IO_GetToken()
// PURPOSE      Searches source buffer for the first %s-style token
//              encountered, and copies it to dst.
// PARAMETERS   char *dst
//                  Buffer to copy token too.
//                  If passed in as NULL, this is not used.
//              const char *src
//                  Source string.
//              char **next
//                  Storage for pointer to remainder of string.
//                  If passed in as NULL, this is not used.
//                  If a token was not found, this is not used.
//                  If token was found and next is not NULL, then
//                  pointer to remainder of string is stored in *next.
// RETURNS      dst, if string was found and dst was passed in as
//              non-NULL.
//              Pointer to token in src, if string was found and dst was
//              passed in as NULL.
//              NULL, otherwise.
//
// NOTES        The caller should make sure dst is big enough.  This
//              function does not perform "dumb" input validation, like
//              checking whether src is passed in as NULL.
//
//              This function was written as a replacement of strtok()
//              that's simpler and safer to use.  Although less
//              efficient than strtok(), it should be faster than
//              sscanf().
//
//              A version of IO_GetToken() that accepts custom delimiters
//              can be written.  GetDelimitedToken() is the suggested
//              name.
//-----------------------------------------------------------------------------

static char * //inline//
IO_GetToken(char *dst, const char *src, char **next)
{
    // a is moving across src.
    // b is moving across dst.

    const char *a = src;
    char *b = dst;
    const char *tokenInSrc;

    // Scan past left-side white space.

    while (1)
    {
        if (*a == 0)
        {
            // Couldn't find a token.

            return NULL;
        }

        if (!isspace(*a))
        {
            break;
        }

        a++;
    }

    // Token was found.  Copy into dst until right-side white space is
    // encountered.

    if (dst == NULL)
    {
        tokenInSrc = a;
    }

    while (1)
    {
        if (dst)
        {
            *b = *a;
            b++;
        }

        a++;

        if (*a == 0 || isspace(*a))
        {
            // Found right-side white space.

            if (dst)
            {
                *b = 0;
            }

            if (next != NULL)
            {
                // Copy pointer to remainder of string into *next.

                *next = (char *) a;
            }

            if (dst)
            {
                return dst;
            }
            else
            {
                return (char *) tokenInSrc;
            }
        }
    }
}

//-----------------------------------------------------------------------------
// FUNCTION     IO_GetDelimitedToken()
// PURPOSE      Searches source buffer for the first delimited token
//              encountered, and copies it to dst.
// PARAMETERS   char *dst
//                  Buffer to copy token too.
//                  If passed in as NULL, this is not used.
//              const char *src
//                  Source string.
//              const char* delim
//                  Delimiter string.
//              char **next
//                  Storage for pointer to remainder of string.
//                  If passed in as NULL, this is not used.
//                  If a token was not found, this is not used.
//                  If token was found and next is not NULL, then
//                  pointer to remainder of string is stored in *next.
// RETURNS      dst, if string was found and dst was passed in as
//              non-NULL.
//              Pointer to token in src, if string was found and dst was
//              passed in as NULL.
//              NULL, otherwise.
//
// NOTES        The caller should make sure dst is big enough.  This
//              function does not perform "dumb" input validation, like
//              checking whether src is passed in as NULL.
//
//              This function was written as a replacement of strtok()
//              that's simpler and safer to use.  Although less
//              efficient than strtok(), it should be faster than
//              sscanf().
//-----------------------------------------------------------------------------

static //inline//
char* IO_GetDelimitedToken(
    char* dst,
    const char* src,
    const char* delim,
    char** next)
{
    // a is moving across src.
    // b is moving across dst.
    const char *a = src;
    char *b = dst;
    const char *tokenInSrc;

    const int numDelim = strlen(delim);

    // Scan past left-side.
    while (1) {
        int i;

        if (*a == 0) {
            // Couldn't find a token.
            return NULL;
        }

        for (i = 0; i < numDelim; i++) {
            if (*a == delim[i]) {
                break;
            }
        }
        if (i == numDelim) {
            // *a is not in the delim string
            break;
        }

        a++;
    }

    // Token was found.  Copy into dst until right-side white space is
    // encountered.
    if (dst == NULL) {
        tokenInSrc = a;
    }

    while (1) {
        int i;

        if (dst) {
            *b = *a;
            b++;
        }

        a++;

        if (*a == 0) {
            break;
        }
        for (i = 0; i < numDelim; i++) {
            if (*a == delim[i]) {
                break;
            }
        }
        if (i != numDelim) {
            // *a is in the delim string
            // Found right-side white space.
            break;
        }
    }

    if (dst) {
        *b = 0;
    }

    if (next != NULL) {
        // Copy pointer to remainder of string into *next.
        *next = (char*)a;
    }

    if (dst) {
        return dst;
    }
    else {
        return (char*)tokenInSrc;
    }
}

//-----------------------------------------------------------------------------
// FUNCTION     IO_TrimLeft()
// PURPOSE      Strips leading white space from a string (by
//              memmove()ing string contents left).
// PARAMETERS   char *s
//                  String.
//
// NOTES        Behavior is as expected for empty strings or strings
//              with only white space.  This function does not validate
//              that s is non-NULL.
//-----------------------------------------------------------------------------

static void //inline//
IO_TrimLeft(char *s)
{
    char *p = s;
    unsigned len = strlen(s);
    unsigned count = 0;

    for (; count < len && isspace(*p); p++, count++)
    {
        // Do nothing.
    }

    if (count > 0)
    {
        memmove(s, s + count, len - count + 1);
    }
}

//-----------------------------------------------------------------------------
// FUNCTION     IO_TrimRight()
// PURPOSE      Strips trailing white space from a string (by inserting
//              early NULL).
// PARAMETERS   char *s
//                  String.
//
// NOTES        Behavior is as expected for empty strings or strings
//              with only white space.  This function does not validate
//              that s is non-NULL.
//-----------------------------------------------------------------------------

static void //inline//
IO_TrimRight(char *s)
{
    unsigned len = strlen(s);
    char *p = s + len;          // p points at terminating NULL

    if (p == s)
    {
        return;
    }

    while (1)
    {
        p--;

        if (!isspace(*p))
        {
            *(p + 1) = 0;
            return;
        }
    }
}

#endif /* FILEIO_H */
