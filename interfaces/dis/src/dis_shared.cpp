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
using namespace std;
#include <cstdio>
#include <cstdlib>
#include <climits>

#include "dis_shared.h"
#include "external_socket.h"

void
DisPrintHex(const char* buf, unsigned length)
{
     cout << "0x0000:  ";

     unsigned i;
     unsigned numBytesPrintedThisLine;
     for (i = 0, numBytesPrintedThisLine = 0; i < length; i++)
     {
         if (numBytesPrintedThisLine == 16)
         {
             cout << endl;
             printf("0x%04x:  ", i);

             numBytesPrintedThisLine = 0;
         }

         printf("%02x ", (unsigned char) buf[i]);

         numBytesPrintedThisLine++;
     }//for//

     cout << endl;
}

bool
DisConvertStringToIpAddress(const char* s, unsigned& ipAddress)
{
    unsigned short a, b, c, d;

    int retVal = sscanf(s, "%hu.%hu.%hu.%hu", &a, &b, &c, &d);
    if (retVal != 4
        || a > 255 || b > 255 || c > 255 || d > 255)
    { return false; }

    ipAddress = (a << 24) + (b << 16) + (c << 8) + d;
    return true;
}

bool
DisIsMulticastIpAddress(unsigned ipAddress)
{
    // Corresponds to >= 224.0.0.0 and < 240.0.0.0.

    return (ipAddress >= 0xe0000000 && ipAddress < 0xf0000000);
}

clocktype
DisConvertDoubleToClocktype(double doubleValue)
{
    // 1.0 units of double is assumed to be 1 second.
    // 1 unit of clocktype is assumed to be 1 nanosecond.
    // Any fractional nanosecond in doubleValue is rounded to the nearest
    // 1 unit of clocktype, not truncated.

    return (clocktype) ((doubleValue * 1e9) + 0.5);
}

double
DisConvertClocktypeToDouble(clocktype clocktypeValue)
{
    // 1 unit of clocktype is assumed to be 1 nanosecond.
    // 1.0 units of double is assumed to be 1 second.

    return ((double) clocktypeValue) / 1e9;
}

void
DisCopyFromOffset(
    void* dst, const void* src, unsigned& offset, unsigned size)
{
    const unsigned char* uchar_src = (const unsigned char*) src;

    memcpy(dst, &uchar_src[offset], size);
    offset += size;
}

void
DisCopyToOffset(
    void* dst, unsigned& offset, const void* src, unsigned size)
{
    unsigned char* uchar_dst = (unsigned char*) dst;

    memcpy(&uchar_dst[offset], src, size);
    offset += size;
}

void
DisCopyFromOffsetAndNtoh(
    void* dst, const void* src, unsigned& offset, unsigned size)
{
    const unsigned char* uchar_src = (const unsigned char*) src;

    memcpy(dst, &uchar_src[offset], size);
    if (size > 1) { EXTERNAL_ntoh(dst, size); }
    offset += size;
}

void
DisCopyToOffsetAndHton(
    void* dst, unsigned& offset, const void* src, unsigned size)
{
    unsigned char* uchar_dst = (unsigned char*) dst;

    memcpy(&uchar_dst[offset], src, size);
    if (size > 1) { EXTERNAL_hton(&uchar_dst[offset], size); }
    offset += size;
}

char*
DisGetAnyToken(
    char* token,
    unsigned tokenBufSize,
    unsigned& numBytesCopied,
    const char*& src)
{
    // This function searches for the first token (delimited by whitespace)
    // in the string pointed to by src.  If no tokens are found, NULL is
    // returned, and numBytesCopied and src will be left unmodified.
    //
    // If a token is found, then a maximum of tokenBufSize - 1 bytes will be
    // copied to the token pointer, the token therein will be null-terminated,
    // numBytesCopied will be set to the number of bytes copied (excluding
    // null terminator) to the token pointer, src will be set to point to the
    // byte after the first right-side delimiter or to the null terminator if
    // the end of the string was reached, and the token pointer will be
    // returned.

    assert(token != NULL);
    assert(src != NULL);

    // p traverses src.

    const char* p = src;

    // Scan past left-side white space.

    while (1)
    {
        if (*p == 0)
        {
            // Couldn't find a token.

            return NULL;
        }

        if (!isspace(*p))
        {
            // Token found.

            break;
        }

        p++;
    }

    // Token has been found.

    const char* tokenStart = p;

    // Look for right-side white space.

    while (1)
    {
        p++;

        if (*p == 0)
        {
            // Found end of string.
            // Copy pointer to null terminator to src.

            src = p;
            break;
        }

        if (isspace(*p))
        {
            // Found white space on right side of token.
            // Copy pointer to remainder of string (starting from the byte
            // after the first white space) to src.

            src = p + 1;
            break;
        }
    }//while//

    if (tokenBufSize == 0) { return token; }

    if (tokenBufSize == 1)
    {
        token[0] = 0;
        numBytesCopied = 0;
        return token;
    }

    unsigned numBytesToCopy = p - tokenStart;

    if (numBytesToCopy > tokenBufSize - 1)
    {
        numBytesToCopy = tokenBufSize - 1;
    }

    memcpy(token, tokenStart, numBytesToCopy);
    token[numBytesToCopy] = 0;

    numBytesCopied = numBytesToCopy;

    return token;
}

char*
DisGetAnyToken(
    char* token,
    unsigned tokenBufSize,
    unsigned& numBytesCopied,
    const char*& src,
    const char* delimiters)
{
    // This function is just like the earlier DisGetAnyToken() function, except
    // the delimiters are defined by the user.  Each character in the
    // delimiters argument functions as a delimiter.

    assert(token != NULL);
    assert(src != NULL);
    assert(delimiters != NULL);

    unsigned numDelimiters = (unsigned)strlen(delimiters);
    assert(numDelimiters > 0);

    // p traverses src.

    const char* p = src;

    // Scan past left-side delimiters.

    unsigned i;

    while (1)
    {
        if (*p == 0)
        {
            // Couldn't find a token.

            return NULL;
        }

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
    }

    // Token has been found.

    const char* tokenStart = p;

    // Look for right-side delimiters.

    while (1)
    {
        p++;

        if (*p == 0)
        {
            // Found end of string.
            // Copy pointer to null terminator to src.

            src = p;
            break;
        }

        for (i = 0; i < numDelimiters; i++)
        {
            if (*p == delimiters[i]) { break; }
        }

        if (i < numDelimiters)
        {
            // Found delimiter on right side of token.
            // Copy pointer to remainder of string (starting from the byte
            // after the first delimiter) to src.

            src = p + 1;
            break;
        }
    }//while//

    if (tokenBufSize == 0) { return token; }

    if (tokenBufSize == 1)
    {
        token[0] = 0;
        numBytesCopied = 0;
        return token;
    }

    unsigned numBytesToCopy = p - tokenStart;

    if (numBytesToCopy > tokenBufSize - 1)
    {
        numBytesToCopy = tokenBufSize - 1;
    }

    memcpy(token, tokenStart, numBytesToCopy);
    token[numBytesToCopy] = 0;

    numBytesCopied = numBytesToCopy;

    return token;
}

const char*
DisGetAnyToken(
    unsigned& numBytesToCopy,
    const char*& src)
{
    // This function is like the previous DisGetAnyToken() functions, except
    // if a token is found, numBytesToCopy will be set to the number of bytes
    // in the token (excluding any null terminator, of course), and a pointer
    // to the start of the token within the src string is returned.

    assert(src != NULL);

    // p traverses src.

    const char* p = src;

    // Scan past left-side white space.

    while (1)
    {
        if (*p == 0)
        {
            // Couldn't find a token.

            return NULL;
        }

        if (!isspace(*p))
        {
            // Token found.

            break;
        }

        p++;
    }

    // Token has been found.

    const char* tokenStart = p;

    // Look for right-side white space.

    while (1)
    {
        p++;

        if (*p == 0)
        {
            // Found end of string.
            // Copy pointer to null terminator to src.

            src = p;
            break;
        }

        if (isspace(*p))
        {
            // Found white space on right side of token.
            // Copy pointer to remainder of string (starting from the byte
            // after the first white space) to src.

            src = p + 1;
            break;
        }
    }//while//

    numBytesToCopy = p - tokenStart;
    return tokenStart;
}

const char*
DisGetAnyToken(
    unsigned& numBytesToCopy,
    const char*& src,
    const char* delimiters)
{
    // This function is just like the previous DisGetAnyToken() function,
    // except the delimiters are defined by the user.  Each character in the
    // delimiters argument functions as a delimiter.

    assert(src != NULL);
    assert(delimiters != NULL);

    unsigned numDelimiters = (unsigned)strlen(delimiters);
    assert(numDelimiters > 0);

    // p traverses src.

    const char* p = src;

    // Scan past left-side delimiters.

    unsigned i;

    while (1)
    {
        if (*p == 0)
        {
            // Couldn't find a token.

            return NULL;
        }

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
    }

    // Token has been found.

    const char* tokenStart = p;

    // Look for right-side delimiter.

    while (1)
    {
        p++;

        if (*p == 0)
        {
            // Found end of string.
            // Copy pointer to null terminator to src.

            src = p;
            break;
        }

        for (i = 0; i < numDelimiters; i++)
        {
            if (*p == delimiters[i]) { break; }
        }

        if (i < numDelimiters)
        {
            // Found delimiter on right side of token.
            // Copy pointer to remainder of string (starting from the byte
            // after the first delimiter) to src.

            src = p + 1;
            break;
        }
    }//while//

    numBytesToCopy = p - tokenStart;
    return tokenStart;
}

char*
DisGetToken(
    char* token,
    unsigned tokenBufSize,
    unsigned& numBytesCopied,
    const char*& src,
    const char* delimiters)
{
    // This function is just like the corresponding DisGetAnyToken() function
    // with the same parameters, except left-side delimiters are not skipped,
    // i.e., it's possible to find an "empty field" instead of a token.
    // E.g., if src is pointing to "foo,,bar,baz" and the delimiter is ",",
    // the 1st, 3rd, and 4th call to DisGetToken() will work as expected, but
    // the second call will find an empty field.
    // If an empty field is found, the token pointer is still returned, and
    // token[0] is set to the null terminator if tokenBufSize is at least 1.
    // NULL is only returned if the end of the string has been reached.

    assert(token != NULL);
    assert(src != NULL);
    assert(delimiters != NULL);

    unsigned numDelimiters = (unsigned)strlen(delimiters);
    assert(numDelimiters > 0);

    // p traverses src.

    const char* p = src;

    if (*p == 0)
    {
        // src points to end of string.

        return NULL;
    }

    unsigned i;
    for (i = 0; i < numDelimiters; i++)
    {
        if (*p == delimiters[i]) { break; }
    }

    if (i < numDelimiters)
    {
        // Found an empty field.
        // Copy pointer to remainder of string (starting from the next byte)
        // to src.

        src = p + 1;

        if (tokenBufSize > 0) { token[0] = 0; }

        return token;
    }

    // Token has been found.

    const char* tokenStart = p;

    // Look for right-side delimiters.

    while (1)
    {
        p++;

        if (*p == 0)
        {
            // Found end of string.
            // Copy pointer to null terminator to src.

            src = p;
            break;
        }

        for (i = 0; i < numDelimiters; i++)
        {
            if (*p == delimiters[i]) { break; }
        }

        if (i < numDelimiters)
        {
            // Found delimiter on right side of token.
            // Copy pointer to remainder of string (starting from the byte
            // after the first delimiter) to src.

            src = p + 1;
            break;
        }
    }//while//

    if (tokenBufSize == 0) { return token; }

    if (tokenBufSize == 1)
    {
        token[0] = 0;
        numBytesCopied = 0;
        return token;
    }

    unsigned numBytesToCopy = p - tokenStart;

    if (numBytesToCopy > tokenBufSize - 1)
    {
        numBytesToCopy = tokenBufSize - 1;
    }

    memcpy(token, tokenStart, numBytesToCopy);
    token[numBytesToCopy] = 0;

    numBytesCopied = numBytesToCopy;

    return token;
}

const char*
DisGetToken(
    unsigned& numBytesToCopy,
    const char*& src,
    const char* delimiters)
{
    // This function is just like the corresponding DisGetAnyToken() function
    // with the same parameters, except left-side delimiters are not skipped.
    // New behavior is as described in the previous function.

    assert(src != NULL);
    assert(delimiters != NULL);

    unsigned numDelimiters = (unsigned)strlen(delimiters);
    assert(numDelimiters > 0);

    // p traverses src.

    const char* p = src;

    if (*p == 0)
    {
        // src points to end of string.

        return NULL;
    }

    unsigned i;
    for (i = 0; i < numDelimiters; i++)
    {
        if (*p == delimiters[i]) { break; }
    }

    if (i < numDelimiters)
    {
        // Found an empty field.
        // Copy pointer to remainder of string (starting from the next byte)
        // to src.

        src = p + 1;

        numBytesToCopy = 0;

        return p;
    }

    // Token has been found.

    const char* tokenStart = p;

    // Look for right-side delimiter.

    while (1)
    {
        p++;

        if (*p == 0)
        {
            // Found end of string.
            // Copy pointer to null terminator to src.

            src = p;
            break;
        }

        for (i = 0; i < numDelimiters; i++)
        {
            if (*p == delimiters[i]) { break; }
        }

        if (i < numDelimiters)
        {
            // Found delimiter on right side of token.
            // Copy pointer to remainder of string (starting from the byte
            // after the first delimiter) to src.

            src = p + 1;
            break;
        }
    }//while//

    numBytesToCopy = p - tokenStart;
    return tokenStart;
}

void
DisCopyAndNullTerminate(char* dst, const char* src, unsigned numBytesToCopy)
{
    memcpy(dst, src, numBytesToCopy);
    dst[numBytesToCopy] = 0;
}

void
DisTrimLeft(char *s)
{
    assert(s != NULL);

    char *p = s;
    unsigned stringLength = (unsigned)strlen(s);
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
DisTrimRight(char *s)
{
    assert(s != NULL);

    unsigned stringLength = (unsigned)strlen(s);
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
DisGetNumLinesInFile(const char* path)
{
    FILE* fp = fopen(path, "r");
    DisVerify(fp != NULL, "Can't open for reading", path);

    const unsigned lineBufSize = 1024;
    char line[lineBufSize];
    unsigned numLines = 0;
    while (1)
    {
        if (fgets(line, lineBufSize, fp) == NULL) { break; }

        numLines++;

        DisVerify(strlen(line) < lineBufSize - 1,
                 "Exceeds permitted line length",
                 path);
    }

    fclose(fp);

    return numLines;
}

void
DisVerify(
    bool condition,
    const char* errorString,
    const char* path,
    unsigned lineNumber)
{
    if (!condition)
    {
        DisReportError(errorString, path, lineNumber);
    }
}

void
DisCheckMalloc(
    const void* ptr,
    const char* path,
    unsigned lineNumber)
{
    if (ptr == NULL)
    {
        DisReportError("Out of memory", path, lineNumber);
    }
}

void
DisCheckNoMalloc(
    const void* ptr,
    const char* path,
    unsigned lineNumber)
{
    if (ptr != NULL)
    {
        DisReportError(
            "Attempting to allocate memory for non-null pointer",
            path,
            lineNumber);
    }
}

void
DisReportWarning(
    const char* warningString,
    const char* path,
    unsigned lineNumber)
{
    cerr << "DIS warning:";

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
DisReportError(
    const char* errorString,
    const char* path,
    unsigned lineNumber)
{
    cerr << "DIS error:";

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

bool DisRadioKey::less::operator()(const DisRadioKey &key1, const DisRadioKey &key2) const
{
    int cmp = strncmp(key1.markingString,
                      key2.markingString,
                      g_disMarkingStringBufSize);
    if (cmp < 0)
    {
        return true;
    }
    else if (cmp > 0)
    {
        return false;
    }
    else if (key1.radioId < key2.radioId)
    {
        return true;
    }
    else
    {
        return false;
    }
}
