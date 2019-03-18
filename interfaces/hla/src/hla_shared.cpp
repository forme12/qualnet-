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

// iostream.h is needed to support DMSO RTI 1.3v6 (non-NG).

#ifdef NOT_RTI_NG
#include <iostream.h>
#else /* NOT_RTI_NG */
#include <iostream>
using namespace std;
#endif /* NOT_RTI_NG */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <csignal>
#include <cassert>
#include <cerrno>

#include "hla_shared.h"
#include "external_socket.h"

clocktype
HlaConvertDoubleToClocktype(double doubleValue)
{
    // 1.0 units of double is assumed to be 1 second.
    // 1 unit of clocktype is assumed to be 1 nanosecond.
    // Any fractional nanosecond in doubleValue is rounded to the nearest
    // 1 unit of clocktype, not truncated.

    return (clocktype) ((doubleValue * 1e9) + 0.5);
}

double
HlaConvertClocktypeToDouble(clocktype clocktypeValue)
{
    // 1 unit of clocktype is assumed to be 1 nanosecond.
    // 1.0 units of double is assumed to be 1 second.

    return ((double) clocktypeValue) / 1e9;
}

void
HlaCopyFromOffset(
    void* dst, const void* src, unsigned& offset, unsigned size)
{
    const unsigned char* uchar_src = (const unsigned char*) src;

    memcpy(dst, &uchar_src[offset], size);
    offset += size;
}

void
HlaCopyToOffset(
    void* dst, unsigned& offset, const void* src, unsigned size)
{
    unsigned char* uchar_dst = (unsigned char*) dst;

    memcpy(&uchar_dst[offset], src, size);
    offset += size;
}

void
HlaCopyFromOffsetAndNtoh(
    void* dst, const void* src, unsigned& offset, unsigned size)
{
    const unsigned char* uchar_src = (const unsigned char*) src;

    memcpy(dst, &uchar_src[offset], size);
    if (size > 1) { EXTERNAL_ntoh(dst, size); }
    offset += size;
}

void
HlaCopyToOffsetAndHton(
    void* dst, unsigned& offset, const void* src, unsigned size)
{
    unsigned char* uchar_dst = (unsigned char*) dst;

    memcpy(&uchar_dst[offset], src, size);
    if (size > 1) { EXTERNAL_hton(&uchar_dst[offset], size); }
    offset += size;
}

// This function returns a "token" from a string.  Tokens are separated by
// delimiters.  E.g., if the delimiter is "," and the string is
// "foo,bar,,baz", then the tokens are "foo", "bar", empty field, and "baz".
//
// A description of the parameters follows:
//
// If skipEmptyFields is true, then empty fields will not be returned, but
// the next available token will be returned instead.  If skipEmptyFields is
// false, the function will return empty fields.
//
// The foundEmptyField argument will initially be set to false by the
// function.  If skipEmptyFields is false, and an empty field is found, then
// the argument will be set to true.
//
// The overflowed argument will initially be set to false by the function.
// If the token parameter is non-NULL, and the length of the token (excluding
// the null terminator) is greater than tokenBufSize - 1, then overflowed will
// be set to true.
//
// If the token parameter is non-NULL:
//
//   The token will be copied to the buffer pointed to by token and
//   null-terminated.  The parameter tokenBufSize is specified by the user,
//   and is the size of the token buffer in bytes.  tokenBufSize should be
//   large enough to store both the actual characters in the token and a
//   null terminator.  Specifically:
//
//   If tokenBufSize is 0, then nothing will be done to the token parameter.
//   If tokenBufSize is > 0, as much of the token is copied to the token
//   parameter as possible, with the requirement that a null terminator will
//   always be set.
//
//   If skipEmptyFields is false, an empty field is detected, and
//   tokenBufSize is > 0, then token[0] will be set to the null terminator.
//
// If the token parameter is NULL:
//
//   The token parameter is left alone, the tokenBufSize parameter is
//   ignored, and other behavior will occur (described later).
//
// The function attempts to set the numBytesToCopyOrCopied parameter to the
// number of bytes in the token (excluding the null terminator).  If
// skipEmptyFields is false and an empty field is detected,
// numBytesToCopyOrCopied is set to 0.
//
// If the token parameter is non-NULL:
//
//   numBytesToCopyOrCopied may not exceed tokenBufSize - 1 ("- 1" allows for
//   the null terminator).
//
// If the token parameter is NULL:
//
//   numBytesToCopyOrCopied will always equal the actual size of the token
//   (excluding the null terminator).
//
// The src parameter points to the string which contains the tokens.  src is
// also set by the function to point to a new location in the string when:
//
//   (1) A token was found, or
//   (2) skipEmptyFields is false and src points to an empty field.
//
// If either case is true:
//
//   For case (1), this also means the byte after the token is either a
//   delimiter or a null terminator.  If that byte was a delimiter, then src
//   is set to point to one byte after the delimiter byte.  If that byte was
//   a null terminator, then src is set to point to the null terminator.
//
//   For case (2), src is set to point to src[1].
//
//   For both cases, the function will return a non-NULL value.  If the token
//   parameter was passed in as non-NULL, then the token parameter is
//   returned.  If the token parameter was passed in as NULL, then:
//
//   For case (1), a pointer to the first byte of the token is returned.
//
//   For case (2), a pointer to the empty field (which is the same as the
//   original value of src) is returned.
//
// If neither of the above cases is true, then one of the following must be
// true:
//
//   (1) src points to a null terminator, or
//   (2) skipEmptyFields is true and src points to only empty fields through
//       the end of the string.
//
//   The function returns NULL in either situation above.  When NULL is
//   returned, numBytesToCopyOrCopied is set to 0, and no other argument is
//   modified by the function.
//
// The delimiters parameter is not optional (the function will assert false
// if the delimiters parameter is passed in as NULL or as a string which
// consists of only a null terminator).  The delimiters argument should point
// to a string, where each character in the string indicates a delimiter.
//
// Comments on the return value:
//
// If skipEmptyFields is true, and the function returns a non-NULL value,
// this means a token has definitely been found.
//
// If skipEmptyFields is false, and the function returns a non-NULL value,
// what was returned could either be a token or an empty field.  Check the
// value of the foundEmptyField argument to determine which.
//
// If a NULL value is ever returned, that means no token was found and there
// are no more tokens in the string.

char*
HlaGetTokenHelper(
    bool skipEmptyFields,
    bool& foundEmptyField,
    bool& overflowed,
    char* token,
    unsigned tokenBufSize,
    unsigned& numBytesToCopyOrCopied,
    const char*& src,
    const char* delimiters)
{
    assert(src != NULL);

    assert(delimiters != NULL);
    unsigned numDelimiters = strlen(delimiters);
    assert(numDelimiters > 0);

    foundEmptyField = false;
    overflowed = false;

    if (src[0] == 0)
    {
        numBytesToCopyOrCopied = 0;
        return NULL;
    }

    // Pointer p will point to token after if statement.

    const char* p = NULL;

    if (skipEmptyFields)
    {
        // Skip past empty fields to find the first token.

        p = src;

        while (1)
        {
            unsigned i;
            for (i = 0; i < numDelimiters; i++)
            {
                if (*p == delimiters[i]) { break; }
            }

            if (i == numDelimiters)
            {
                // Token found.
                // (since the current character is not a delimiter)

                break;
            }

            p++;

            if (*p == 0)
            {
                // Couldn't find a token.

                numBytesToCopyOrCopied = 0;
                return NULL;
            }
        }//while//

        // p now points to token.
    }
    else
    {
        // Determine if there is an empty field.

        unsigned i;
        for (i = 0; i < numDelimiters; i++)
        {
            if (src[0] == delimiters[i]) { break; }
        }

        if (i < numDelimiters)
        {
            // Found an empty field.

            foundEmptyField = true;

            char* emptyField = (char*) src;
            src = &src[1];
            numBytesToCopyOrCopied = 0;

            if (token != NULL)
            {
                if (tokenBufSize > 0) { token[0] = 0; }
                return token;
            }
            else
            {
                return emptyField;
            }
        }//if//

        // There is a token.  Set p to point to token.

        p = src;
    }

    // Token has been found, and p points to it.

    const char* tokenStart = p;

    // Look for right-side delimiters.
    // After the while statement, p will point to src's null terminator or
    // a delimiter, whichever occurs first.

    while (1)
    {
        p++;

        if (*p == 0)
        {
            // Found end of string.
            // Set src to point to null terminator.

            src = p;
            break;
        }

        unsigned i;
        for (i = 0; i < numDelimiters; i++)
        {
            if (*p == delimiters[i]) { break; }
        }

        if (i < numDelimiters)
        {
            // Found delimiter on right side of token.
            // Set src to point to remainder of string (starting from the
            // byte after the first delimiter).

            src = p + 1;
            break;
        }
    }//while//

    // p points to src's null terminator or a delimiter, whichever occurred
    // first.  (p - 1 points to the last character of the token.)

    if (token != NULL)
    {
        if (tokenBufSize <= 1)
        {
            if (tokenBufSize == 1) { token[0] = 0; }
            numBytesToCopyOrCopied = 0;
            return token;
        }

        unsigned numBytesToCopy = p - tokenStart;

        if (numBytesToCopy > tokenBufSize - 1)
        {
            overflowed = true;

            numBytesToCopy = tokenBufSize - 1;
        }

        memcpy(token, tokenStart, numBytesToCopy);
        token[numBytesToCopy] = 0;

        numBytesToCopyOrCopied = numBytesToCopy;

        return token;
    }
    else
    {
        numBytesToCopyOrCopied = p - tokenStart;
        return (char*) tokenStart;
    }//if//
}

char*
HlaGetTokenOrEmptyField(
    char* token,
    unsigned tokenBufSize,
    unsigned& numBytesCopied,
    bool& overflowed,
    const char*& src,
    const char* delimiters,
    bool& foundEmptyField)
{
    return HlaGetTokenHelper(
               false,
               foundEmptyField,
               overflowed,
               token,
               tokenBufSize,
               numBytesCopied,
               src,
               delimiters);
}

char*
HlaGetToken(
    char* token,
    unsigned tokenBufSize,
    unsigned& numBytesCopied,
    bool& overflowed,
    const char*& src,
    const char* delimiters)
{
    bool unusedFoundEmptyField = false;

    return HlaGetTokenHelper(
               true,
               unusedFoundEmptyField,
               overflowed,
               token,
               tokenBufSize,
               numBytesCopied,
               src,
               delimiters);
}

const char*
HlaLocateTokenOrEmptyField(
    unsigned& numBytesToCopy,
    const char*& src,
    const char* delimiters,
    bool& foundEmptyField)
{
    bool unusedOverflowed = false;
    unsigned unusedTokenBufSize = 0;

    return (const char*) HlaGetTokenHelper(
                             false,
                             foundEmptyField,
                             unusedOverflowed,
                             NULL,
                             unusedTokenBufSize,
                             numBytesToCopy,
                             src,
                             delimiters);
}

const char*
HlaLocateToken(
    unsigned& numBytesToCopy,
    const char*& src,
    const char* delimiters)
{
    bool unusedFoundEmptyField = false;
    bool unusedOverflowed = false;
    unsigned unusedTokenBufSize = 0;

    return (const char*) HlaGetTokenHelper(
                             false,
                             unusedFoundEmptyField,
                             unusedOverflowed,
                             NULL,
                             unusedTokenBufSize,
                             numBytesToCopy,
                             src,
                             delimiters);
}

void
HlaTrimLeft(char *s)
{
    assert(s != NULL);

    char *p = s;
    unsigned stringLength = strlen(s);
    unsigned numBytesWhitespace = 0;

    while (numBytesWhitespace < stringLength
          && isspace(*p))
    {
        p++;
        numBytesWhitespace++;
    }

    if (numBytesWhitespace > 0)
    {
        memmove(s, s + numBytesWhitespace, stringLength - numBytesWhitespace + 1);
    }
}

void
HlaTrimRight(char *s)
{
    assert(s != NULL);

    unsigned stringLength = strlen(s);
    if (stringLength == 0) { return; }

    // p points one byte to left of terminating NULL.

    char *p;

    for (p = &s[stringLength - 1]; isspace(*p); p--)
    {
        if (p == s)
        {
            // Entire string was whitespace.

            *p = 0;
            return;
        }
    }//while//

    *(p + 1) = 0;
}

unsigned
HlaGetNumLinesInFile(const char* path)
{
    FILE* fp = fopen(path, "r");
    HlaVerify(fp != NULL, "Can't open for reading", path);

    char line[g_hlaLineBufSize];
    unsigned numLines = 0;
    while (1)
    {
        if (fgets(line, g_hlaLineBufSize, fp) == NULL) { break; }

        numLines++;

        HlaVerify(strlen(line) < g_hlaLineBufSize - 1,
                 "Exceeds permitted line length",
                 path);
    }

    fclose(fp);

    return numLines;
}

void
HlaVerify(
    bool condition,
    const char* errorString,
    const char* path,
    unsigned lineNumber)
{
    if (!condition)
    {
        HlaReportError(errorString, path, lineNumber);
    }
}

void
HlaCheckMalloc(
    const void* ptr,
    const char* path,
    unsigned lineNumber)
{
    if (ptr == NULL)
    {
        HlaReportError("Out of memory", path, lineNumber);
    }
}

void
HlaCheckNoMalloc(
    const void* ptr,
    const char* path,
    unsigned lineNumber)
{
    if (ptr != NULL)
    {
        HlaReportError(
            "Attempting to allocate memory for non-null pointer",
            path,
            lineNumber);
    }
}

void
HlaReportWarning(
    const char* warningString,
    const char* path,
    unsigned lineNumber)
{
    cerr << "HLA warning:";

    if (path != NULL)
    {
        cerr << path << ":";

        if (lineNumber > 0)
        {
            cerr << lineNumber << ":";
        }
    }

    cerr << " " << warningString << endl;
}

void
HlaReportError(
    const char* errorString,
    const char* path,
    unsigned lineNumber)
{
    cerr << "HLA error:";

    if (path != NULL)
    {
        cerr << path << ":";

        if (lineNumber > 0)
        {
            cerr << lineNumber << ":";
        }
    }

    cerr << " " << errorString << endl;

    exit(EXIT_FAILURE);
}

HlaInterfaceData::HlaInterfaceData  ()
{
    m_hla = NULL;
    m_hlaFedAmb = NULL;
    m_hlaRtiAmb = NULL;
    m_interface = NULL;
}

bool HlaRadioKey::less::operator()(const HlaRadioKey &key1, const HlaRadioKey &key2) const
{
    int cmp = strncmp(key1.markingData,
                      key2.markingData,
                      g_hlaMarkingDataBufSize);
    if (cmp < 0)
    {
        return true;
    }
    else if (cmp > 0)
    {
        return false;
    }
    else if (key1.radioIndex < key2.radioIndex)
    {
        return true;
    }
    else
    {
        return false;
    }
}

