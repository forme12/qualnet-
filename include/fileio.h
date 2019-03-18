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
// PACKAGE :: FILEIO
// DESCRIPTION ::
//      This file describes data strucutres and functions used for reading from
//      input files and printing to output files.
// **/

#ifndef FILEIO_H
#define FILEIO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
// DEFINES
//-----------------------------------------------------------------------------

// /**
// CONSTANT :: ANY_NODEID : 0xffffffff
// DESCRIPTION ::
//      Optional macro values to use when calling IO_Read...() APIs.
//      Defines any node id.
// **/
#define ANY_NODEID      0xffffffff

// /**
// CONSTANT :: ANY_ADDRESS : 0xffffffff
// DESCRIPTION ::
//      Optional macro values to use when calling IO_Read...() APIs.
//      Defines any node address.
// **/
#define ANY_ADDRESS     0xffffffff

// /**
// CONSTANT :: ANY_INSTANCE : 0xffffffff
// DESCRIPTION ::
//      Optional macro values to use when calling IO_Read...() APIs.
//      Defines any instance.
// **/
#define ANY_INSTANCE    0xffffffff

// /**
// CONSTANT :: MAX_INPUT_FILE_LINE_LENGTH : 6 * MAX_STRING_LENGTH
// DESCRIPTION ::
//      Maximum input file line length. Evaluates (6 * MAX_STRING_LENGTH)
// **/
// Original this was set to (6* MAX_STRING_LENGTH), but needs to be increased
// to accomodate the long lines
#define MAX_INPUT_FILE_LINE_LENGTH      (200 * MAX_STRING_LENGTH)

// /**
// CONSTANT :: MAX_ADDRESS_STRING_LENGTH : 80
// DESCRIPTION ::
//      Maximum length of address string.
// **/
#define MAX_ADDRESS_STRING_LENGTH       80

// /**
// CONSTANT :: MAX_NUM_CACHED_FILES : 128
// DESCRIPTION ::
//      Max number of -FILE references in an input file. (Restriction is
//      only for immediate children)
// **/
#define MAX_NUM_CACHED_FILES 128

// /**
// CONSTANT :: MATCH_NODE_ID : 4
// DESCRIPTION ::
//      Defines the matching by node id.
// **/
#define MATCH_NODE_ID   4

// /**
// CONSTANT :: MATCH_NETWORK : 6
// DESCRIPTION ::
//      Defines the matching by network.
// **/
#define MATCH_NETWORK   6

// /**
// CONSTANT :: MATCH_INTERFACE : 8
// DESCRIPTION ::
//      Defines the matching by interface.
// **/
#define MATCH_INTERFACE 8

// /**
// CONSTANT :: INPUT_ALLOCATION_UNIT : 500
// DESCRIPTION ::
//      Defines input allocation unit.
// **/
#define INPUT_ALLOCATION_UNIT 500

//-----------------------------------------------------------------------------
// STRUCTS, ENUMS, AND TYPEDEFS
//-----------------------------------------------------------------------------

// /**
// ENUMERATION ::
// DESCRIPTION ::
//    Used in dynamic changing of parameters.
// **/
enum {
    TYPE_INT32 = 0,
    TYPE_INT64 = 1,
    TYPE_UINT32 = 2,    
    TYPE_FLOAT32 = 3,
    TYPE_FLOAT64 = 4,
    TYPE_NODEADDRESS = 5,
    TYPE_STRING = 6,
    TYPE_CLOCKTYPE = 7,
    TYPE_BOOL = 8,
    TYPE_INPUTFILE = 9
};

// /**
// STRUCT :: NodeInput
// DESCRIPTION ::
//      Definition of node input structure. typedef to NodeInput in
//      include/main.h.描述的是节点输入的各信息
// **/
struct NodeInput
{
    char* ourName;// just the name, not path
    int  numLines;
    int  maxLineLen;
    char **inputStrings;
    char **timeQualifiers;
    char **qualifiers;
    char **variableNames;
    int  *instanceIds;
    char **values;
    int  numFiles;
    char *cachedFilenames[MAX_NUM_CACHED_FILES];
    NodeInput *cached[MAX_NUM_CACHED_FILES];
    NodeInput *routerModelInput;
    int maxNumLines;
    clocktype startSimClock;

    // Needed for change param avlue
    PartitionData* partition;
};

//-------------------------------------------------------------------------
// INLINED FUNCTIONS
//-------------------------------------------------------------------------

// /**
// API :: IO_ConvertIpAddressToString
// PURPOSE :: Parses IPv4 address into a dotted-decimal string.
// PARAMETERS ::
// + ipAddress      : NodeAddress   : IPv4 address to be converted into
//                                    the string.
// + addressString  : char*         : Storage for string.
// RETURN :: void :
// **/

// NOTES        This function does not distinguish between network- and
//              host-byte order.
//
//              This function should be renamed to
//              IO_ConvertIpAddressToString().
static void //inline//
IO_ConvertIpAddressToString(
    NodeAddress ipAddress,
    char *addressString)
{
    sprintf(addressString, "%u.%u.%u.%u",
            (ipAddress & 0xff000000) >> 24,
            (ipAddress & 0xff0000) >> 16,
            (ipAddress & 0xff00) >> 8,
            ipAddress & 0xff);
}

// /**
// API :: IO_FindStringPos
// PURPOSE :: Returns the index of the first subString found in s.
// PARAMETERS ::
// + s[]            : const char : Source string.
// + subString[]    : const char : Substring to earch for.
// RETURN :: int    : Index of the first subString found in s. -1, if not
//                    found.
// **/
static int //inline//
IO_FindStringPos(
    char s[],
    const char subString[])
{
    char *foundAt = strstr(s, subString);

    if (foundAt == NULL)
    {
        return -1;
    }
    else
    {
        return (int)(foundAt - &s[0]);
    }
}

// /**
// API :: IO_GetToken
// PURPOSE :: Searches source buffer for the first %s-style token
//            encountered, and copies it to dst.
// PARAMETERS ::
// + dst    : char*         : Buffer to copy token too. If passed in as
//                            NULL, this is not used.
// + src    : const char*   : Source string.
// + next   : char **       : Storage for pointer to remainder of string.
//                            If passed in as NULL, this is not used. If
//                            a token was not found, this is not used.
//                            If token was found and next is not NULL,
//                            then pointer to remainder of string is stored
//                            in *next.
// RETURN :: char* : dst, if string was found and dst was passed in as
//                   non-NULL. Pointer to token in src, if string was found
//                   and dst was passed in as NULL. NULL, otherwise.
// **/

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
static char * //inline//
IO_GetToken(char *dst, const char *src, char **next)
{
    // a is moving across src.
    // b is moving across dst.

    const char *a = src;
    char *b = dst;
    const char *tokenInSrc = NULL;

    // Scan past left-side white space.

    while (1)
    {
        if (*a == 0)
        {
            // Couldn't find a token.
            *dst = 0;
            *next = NULL;
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

// /**
// API :: IO_GetDelimitedToken
// PURPOSE :: Searches source buffer for the first delimited token
//            encountered, and copies it to dst.
// PARAMETERS ::
// + dst    : char*         : Buffer to copy token too. If passed in as
//                            NULL, this is not used.
// + src    : const char*   : Source string.
// + delim  : const char*   : Delimiter string.
// + next   : char**        : Storage for pointer to remainder of string.
//                            If passed in as NULL, this is not used. If
//                            a token was not found, this is not used.
//                            If token was found and next is not NULL,
//                            then pointer to remainder of string is stored
//                            in *next.
// RETURN :: char* : dst, if string was found and dst was passed in as
//                   non-NULL. Pointer to token in src, if string was found
//                   and dst was passed in as NULL. NULL, otherwise.
// **/

// NOTES        The caller should make sure dst is big enough.  This
//              function does not perform "dumb" input validation, like
//              checking whether src is passed in as NULL.
//
//              This function was written as a replacement of strtok()
//              that's simpler and safer to use.  Although less
//              efficient than strtok(), it should be faster than
//              sscanf().
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
    const char *tokenInSrc = 0;

    const int numDelim = (int)strlen(delim);

    // Scan past left-side.
    while (1) {
        int i;

        if (*a == 0) {
            // Couldn't find a token.
            *dst = 0;
            *next = NULL;
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

// /**
// API :: IO_Right
// PURPOSE :: Returns a pointer to the right side of the string of length
//            "count" characters.
// PARAMETERS ::
// + s      : const char *  : String.
// + count  : unsigned      : Number of characters on the right side.
// RETURN  :: const char*   : A pointer to the right side of the string of
//                            length "count" characters. If count is 0, then
//                            a pointer to the string's terminating NULL is
//                            returned. If count is equal to or greater than
//                            the number of characters in the string, then
//                            the whole string is returned. A "character"
//                            is just a byte of char type that's not NULL.
//                            So, '\n' counts as a character.
// **/
static const char * //inline//
IO_Right(const char *s, unsigned count)
{
    unsigned len = (unsigned)strlen(s);
    const char *termNull = s + len;

    if (count == 0)
    {
        return termNull;
    }

    if (count >= len)
    {
        return s;
    }

    return termNull - count;
}

// /**
// API :: IO_Chop
// PURPOSE :: Removes the last character of string.
// PARAMETERS ::
// + s  : const char*   : String.
// RETURN :: char*      : s. If the string has a strlen() of zero, then the
//                        string is returned unmodified. A "character" is
//                        just a byte of char type that's not NULL. So, '\n'
//                        counts as a character.
// **/
static char * //inline//
IO_Chop(char *s)
{
    if (*s != 0)
    {
        s[strlen(s) - 1] = 0;
    }

    return s;
}

// /**
// API :: IO_TrimNsbpSpaces
// PURPOSE :: Changes nsbp charecters for UTF-8 encoding to spaces.
// PARAMETERS ::
// + s  : char* : String.
// RETURN :: void :
// **/

// NOTES        Behavior is as expected for empty strings or strings
//              with only white space.  This function does not validate
//              that s is non-NULL.
static void //inline//
IO_TrimNsbpSpaces(char *s)
{
    unsigned char *p = ( unsigned char* )s;
    unsigned len = strlen(s);
    unsigned count = 0;

    for (; count < len ; p++, count++)
    {
        if (*p == 160 || *p == 194){     
            *p = ' ';
        }
    }
}

// /**
// API :: IO_TrimLeft
// PURPOSE :: Strips leading white space from a string (by memmove()ing
//            string contents left).
// PARAMETERS ::
// + s  : char* : String.
// RETURN :: void :
// **/

// NOTES        Behavior is as expected for empty strings or strings
//              with only white space.  This function does not validate
//              that s is non-NULL.
static void //inline//
IO_TrimLeft(char *s)
{
    char *p = s;
    unsigned len = (unsigned)strlen(s);
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

// /**
// API :: IO_TrimRight
// PURPOSE :: Strips trailing white space from a string (by inserting
//            early NULL).
// PARAMETERS ::
// + s  : char* : String.
// RETURN :: void :
// **/

// NOTES        Behavior is as expected for empty strings or strings
//              with only white space.  This function does not validate
//              that s is non-NULL.
static void //inline//
IO_TrimRight(char *s)
{
    unsigned len = (unsigned)strlen(s);
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

// /**
// API :: IO_CompressWhiteSpace
// PURPOSE :: Compresses white space between words in the string to one
//            space in a string. White space at the very beginning and
//            very end of the string is also compressed to one space -- not
//            stripped entirely.
// PARAMETERS ::
// + s  : char* : String.
// RETURN :: void :
// **/

// NOTES        This function compresses white space between words
//              in the string to one space.
//
//              White space at the very beginning and very end of the
//              string is also compressed to one space -- not stripped
//              entirely.  Use IO_TrimLeft() and IO_TrimRight() as desired
//              before calling IO_CompressWhiteSpace().
//
//              Behavior is as expected for empty strings or strings
//              with only white space.  This function does not validate
//              that s is non-NULL.
static void //inline//
IO_CompressWhiteSpace(char *s)
{
    unsigned len = (unsigned)strlen(s);
    char *p = s + len;          // p points at terminating NULL

    if (p == s)
    {
        return;
    }

    // This function is not yet implemented.
}

// /**
// API :: IO_IsStringNonNegativeInteger
// PURPOSE :: Returns TRUE if every character in string is a digit. (Even
//            white space will cause return of FALSE)
// PARAMETERS ::
// + s  : const char*   : String.
// RETURN :: BOOL : TRUE if every character is a digit. FALSE, otherwise.
// **/
static BOOL //inline//
IO_IsStringNonNegativeInteger(const char *s)
{
    for (; *s; s++)
    {
        if (!isdigit(*s))
        {
            return FALSE;
        }
    }

    return TRUE;
}

// /**
// API :: IO_ConvertStringToLowerCase
// PURPOSE :: Runs tolower() on each character in string and converts the
//            same to lowercase.
// PARAMETERS ::
// + s[]    : char  : String.
// RETURN  :: void :
// **/
static void //inline//
IO_ConvertStringToLowerCase(char s[])
{
    int lineLength = (int)strlen(s);
    int i;

    for (i = 0; i < lineLength; i++)
    {
        s[i] = (char)tolower(s[i]);
    }
}

// /**
// API :: IO_ConvertStringToUpperCase
// PURPOSE :: Runs toupper() on each character in string and converts the
//            same to uppercase.
// PARAMETERS ::
// + s[]    : char  : String.
// RETURN  :: void :
// **/
static void //inline//
IO_ConvertStringToUpperCase(char s[])
{
    int lineLength = (int)strlen(s);
    int i;

    for (i = 0; i < lineLength; i++)
    {
        s[i] = (char)toupper(s[i]);
    }
}

// /**
// API          :: IO_CaseInsensitiveStringsAreEqual
// PURPOSE      :: Checks two strings are equal or not ignoring case.
// PARAMETERS   ::
// + s1         : const char[] : First string.
// + s2         : const char[] : Second string.
// + lengthToCompare   :  char   : Length to compare.
// RETURN       :: BOOL  : Returns TRUE if strings are equal, FALSE otherwise.
// **/
static BOOL //inline//
IO_CaseInsensitiveStringsAreEqual(
    const char s1[],
    const char s2[],
    const int lengthToCompare)
{
    int i;

    for (i = 0; i < lengthToCompare; i++)
    {
        if (tolower(s1[i]) != tolower(s2[i]))
        {
            return FALSE;
        }

        if (s1[i] == 0)
        {
            return TRUE;
        }
    }
    return TRUE;
}

// /**
// API :: IO_BlankLine
// PURPOSE :: Checks the blank line/string.
// PARAMETERS ::
// + s[]    : char  : String.
// RETURN  :: BOOL  : Returns TRUE if the string is blank. FALSE, otherwise.
// **/
static BOOL //inline//
IO_BlankLine(const char s[])
{
    unsigned i;

    for (i = 0; i < strlen(s); i++)
    {
        if ((int) s[i] > (int) (' '))
        {
            return FALSE;
        }
    }
    return TRUE;
}

// /**
// API :: IO_CommentLine
// PURPOSE :: Checks whether the line is a comment(i.e. starts with '#').
// PARAMETERS ::
// + s[]   : char : String.
// RETURN :: BOOL : Returns TRUE if the line is a comment. FALSE, otherwise.
// **/
static BOOL //inline//
IO_CommentLine(const char s[])
{
    return (BOOL) (s[0] == '#' || IO_BlankLine(s));
}

// /**
// API :: IO_FindCaseInsensitiveStringPos
// PURPOSE :: Finds the case insensitive sub string position in a string.
// PARAMETERS ::
// + s[]            : const char    : String.
// + subString[]    : const char    : Sub string
// RETURN :: int    : Returns the position of case insensitive sub string if
//                    found. -1, otherwise.
// **/
static int //inline//
IO_FindCaseInsensitiveStringPos(
    const char s[],
    const char subString[])
{
    int i;
    int subStringLength = (int)strlen(subString);

    for (i = 0; i <= (int)(strlen(s) - subStringLength); i++)
    {
        int n;

        for (n = 0; n < subStringLength
                    && tolower(s[i + n]) == tolower(subString[n]); n++)
        {
            // Still comparing.
        }

        if (n == subStringLength)
        {
            return i;
        }
    }

    return -1;  // Not found.
}

// /**
// API :: IO_FindCaseInsensitiveStringPos
// PURPOSE :: Finds the case insensitive sub string position in a string.
// PARAMETERS ::
// + s[]            : const char    : String.
// + subString[]    : const char    : Sub string
// RETURN :: int    : Returns the position of case insensitive sub string if
//                    found. -1, otherwise.
// **/


// /**
// API ::        IO_SkipToken.
// PURPOSE ::     skip the first n tokens.
// PARAMETERS::
// + token      : char* : pointer to the input string,
// + tokenSep   : char* : pointer to the token separators,
// + skip       : char* : number of skips.
// RETRUN :: char*  : pointer to the next token position.
// **/
static char* //inline//
IO_SkipToken(char* token, const char* tokenSep, int skip)
{
    int idx;

    if (token == NULL)
    {
        return NULL;
    }

    // Eliminate preceding space
    idx = (int)strspn(token, tokenSep);
    token = (idx < (signed int)strlen(token)) ? token + idx : NULL;

    while (skip > 0 && token != NULL)
    {
        token = strpbrk(token, tokenSep);

        if (token != NULL)
        {
            idx = (int)strspn(token, tokenSep);
            token = (idx < (signed int)strlen(token)) ? token + idx : NULL;
        }
        skip--;
    }
    return token;
}


//--------------------------------------------------------------------------
// PROTOTYPES FOR FUNCTIONS WITH EXTERNAL LINKAGE
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// Input files
//--------------------------------------------------------------------------
// /**
// FUNCTION :: IO_CreateNodeInput
// PURPOSE  :: Allocates a NodeInput datastructure that can then be
//             passed to IO_ReadNodeInput
//             Called for each file variable in the config file.
// PARAMETERS  ::
//  + nodeInput : NodeInput* : Pointer to node input.
//  + filename : const char* : Path to input file.
// RETURN ::   NodeInput * :  None
// **/
// NOTES        This function will excise comments, remove leading and
//              trailing white space.
NodeInput * IO_CreateNodeInput (bool allocateRouterModelInput = false);

// /**
// API :: IO_InitializeNodeInput
// PURPOSE :: Initializes a NodeInput structure
// PARAMETERS ::
// + nodeInput  : NodeInput*    : A pointer to NodeInput structure.
// RETURNS :: void :
// **/
void IO_InitializeNodeInput(NodeInput *nodeInput,
    bool allocateRouterModelInput);

// /**
// API :: IO_ReadNodeInput
// PURPOSE ::  Reads an input file into a NodeInput struct.
//             Calls IO_ReadFileParameters to first read in -FILE paramters.
//             Then calls IO_ReadFileParameters to read the rest of the
//             parameters.
// PARAMETERS ::
// + nodeInput  : NodeInput*    : Pointer to node input.
// + filename   : const char*   : Path to input file.
// RETURN :: void :
// **/
void
IO_ReadNodeInput(NodeInput *nodeInput, const char *filename);
//BackwardCompatibility Fix Start
// /**
// API :: IO_ReadNodeInputEx
// PURPOSE ::  Reads an input file into a NodeInput struct.
//               The includeComment Flag facilitate whether to include 
//               the commented line
//               lines in the nodeInput structure or not. 
//               The commented lines should only 
//               be included in Backward Compatibity for Old scenario exe.
//               This exe is responsible
//               for creating new config file and router model file.
//               These new files contains 
//               changes according to current VERSION of QualNet.
//               Calls IO_ReadFileParameters to first read in -FILE paramters.
//               Then calls IO_ReadFileParameters to read the rest of the
//               parameters.
// PARAMETERS ::
// + nodeInput            : NodeInput*   :  Pointer to node input.
// + filename            : const char*   :  Path to input file.
// + includeComment        : const char* :  When this flag is true it 
//                                          included the commented line in 
//                                          nodeInput structure also.
// RETURN :: void :
// **/
void
IO_ReadNodeInputEx(NodeInput *nodeInput,
                   const char *filename,
                   BOOL includeComment = FALSE);
//BackwardCompatibility Fix End
void
IO_ReadFile(NodeInput *nodeInput, const char *filename);

// /**
// API :: IO_ConvertFile
// PURPOSE :: Converts the contents of an old configuration file
//            to the latest version.
// PARAMETERS ::
// + nodeInput  : NodeInput*    : A pointer to node input.
// + nodeOutput : NodeInput*    : A pointer to node input. Goes through
//                                database and print current values to
//                                nodeOutput.
// + version    : char*         : Not used.
// RETURN :: BOOL : Returns TRUE if able to convert. FALSE, otherwise.
//                  Either couldn't load the database or something else bad
//                  happened, so just copy the old into the new.
// **/
BOOL
IO_ConvertFile(NodeInput *nodeInput,
                  NodeInput *nodeOutput,
                  char *version,
                  BOOL includeComments);

//---------------------------------------------------------------------------
// Input files parameter retrieval
//---------------------------------------------------------------------------

// /**
// API :: IO_ReadLine
// PURPOSE :: This API is used to retrieve a whole line
//            from input files.
// PARAMETERS ::
// + nodeId             : const NodeAddress : nodeId. Can be ANY_NODEID.
// + interfaceAddress   : const NodeAddress : IP address of interface.
//                                            Can be ANY_IP.
// + nodeInput          : const NodeInput*  : Pointer to node input.
// + index              : const char*       : Parameter name.
// + wasFound           : BOOL*             : Storage for success of seach.
// + readVal            : char*             : Storage for parameter value.
// RETURN :: void :
// **/
void
IO_ReadLine(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    const NodeInput *nodeInput,
    const char *index,
    BOOL *wasFound,
    char *readVal);

// /**
// API :: IO_ReadString
// PURPOSE :: This API is used to retrieve a string parameter
//            value from input files.
// PARAMETERS ::
// + nodeId             : const NodeAddress : nodeId. Can be ANY_NODEID.
// + interfaceAddress   : const NodeAddress : IP address of interface.
//                                            Can be ANY_IP.
// + nodeInput          : const NodeInput*  : Pointer to node input.
// + index              : const char*       : Parameter name.
// + wasFound           : BOOL*             : Storage for success of seach.
// + readVal            : char*             : Storage for parameter value.
// RETURN :: void :
// **/
void
IO_ReadString(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    const NodeInput *nodeInput,
    const char *index,
    BOOL *wasFound,
    char *readVal);

// /**
// API :: IO_ReadBool
// PURPOSE :: This API is used to retrieve a boolean parameter
//            value from input files.
// PARAMETERS ::
// + nodeId             : const NodeAddress : nodeId. Can be ANY_NODEID.
// + interfaceAddress   : const NodeAddress : IP address of interface.
//                                            Can be ANY_IP.
// + nodeInput          : const NodeInput*  : Pointer to node input.
// + index              : const char*       : Parameter name.
// + wasFound           : BOOL*             : Storage for success of seach.
// + readVal            : BOOL*             : Storage for parameter value.
// RETURN :: void :
// **/
void
IO_ReadBool(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    const NodeInput *nodeInput,
    const char *index,
    BOOL *wasFound,
    BOOL *readVal);

// /**
// API :: IO_ReadInt
// PURPOSE :: This API is used to retrieve an integer parameter
//            value from input files.
// PARAMETERS ::
// + nodeId             : const NodeAddress : nodeId. Can be ANY_NODEID.
// + interfaceAddress   : const NodeAddress : IP address of interface.
//                                            Can be ANY_IP.
// + nodeInput          : const NodeInput*  : Pointer to node input.
// + index              : const char*       : Parameter name.
// + wasFound           : BOOL*             : Storage for success of seach.
// + readVal            : int*              : Storage for parameter value.
// RETURN :: void :
// **/
void
IO_ReadInt(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    const NodeInput *nodeInput,
    const char *index,
    BOOL *wasFound,
    int *readVal);

void
IO_ReadandCheckInt(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    int *parameterValue,
    BOOL required,
    BOOL minReq,
    int minVal,
    BOOL maxReq,
    int maxVal);

// /**
// API :: IO_ReadDouble
// PURPOSE :: This API is used to retrieve a double parameter
//            value from input files.
// PARAMETERS ::
// + nodeId             : const NodeAddress : nodeId. Can be ANY_NODEID.
// + interfaceAddress   : const NodeAddress : IP address of interface.
//                                            Can be ANY_IP.
// + nodeInput          : const NodeInput*  : Pointer to node input.
// + index              : const char*       : Parameter name.
// + wasFound           : BOOL*             : Storage for success of seach.
// + readVal            : double*           : Storage for parameter value.
// RETURN :: void :
// **/
void
IO_ReadDouble(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    const NodeInput *nodeInput,
    const char *index,
    BOOL *wasFound,
    double *readVal);

void
IO_ReadInt64(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    const NodeInput *nodeInput,
    const char *index,
    BOOL *wasFound,
    Int64 *readVal);

// /**
// API :: IO_ReadFloat
// PURPOSE :: This API is used to retrieve a float parameter
//            value from input files.
// PARAMETERS ::
// + nodeId             : const NodeAddress : nodeId. Can be ANY_NODEID.
// + interfaceAddress   : const NodeAddress : IP address of interface.
//                                            Can be ANY_IP.
// + nodeInput          : const NodeInput*  : Pointer to node input.
// + index              : const char*       : Parameter name.
// + wasFound           : BOOL*             : Storage for success of seach.
// + readVal            : float*            : Storage for parameter value.
// RETURN :: void :
// **/
void
IO_ReadFloat(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    const NodeInput *nodeInput,
    const char *index,
    BOOL *wasFound,
    float *readVal);

// /**
// API :: IO_ReadTime
// PURPOSE :: This API is used to retrieve time parameter
//            value from input files.
// PARAMETERS ::
// + nodeId             : const NodeAddress : nodeId. Can be ANY_NODEID.
// + interfaceAddress   : const NodeAddress : IP address of interface.
//                                            Can be ANY_IP.
// + nodeInput          : const NodeInput*  : Pointer to node input.
// + index              : const char*       : Parameter name.
// + wasFound           : BOOL*             : Storage for success of seach.
// + readVal            : clocktype*        : Storage for parameter value.
// RETURN :: void :
// **/
void
IO_ReadTime(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    const NodeInput *nodeInput,
    const char *index,
    BOOL *wasFound,
    clocktype *readVal);

void
IO_ReadandCheckTime(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    clocktype *parameterValue,
    BOOL required,
    BOOL minReq,
    clocktype minVal,
    BOOL maxReq,
    clocktype maxVal);

// /**
// API :: IO_ReadCachedFile
// PURPOSE :: This API is used to retrieve cached file parameter
//            value from input files.
// PARAMETERS ::
// + nodeId             : const NodeAddress : nodeId. Can be ANY_NODEID.
// + interfaceAddress   : const NodeAddress : IP address of interface.
//                                            Can be ANY_IP.
// + nodeInput          : const NodeInput*  : Pointer to node input.
// + index              : const char*       : Parameter name.
// + wasFound           : BOOL*             : Storage for success of seach.
// + parameterValue     : NodeInput*        : Storage for parameter value.
// RETURN :: void :
// **/
void
IO_ReadCachedFile(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    NodeInput *parameterValue);

//---------------------------------------------------------------------------
// Input files parameter retrieval by instance qualifier
//---------------------------------------------------------------------------

// /**
// API :: IO_ReadStringInstance
// PURPOSE :: This API is used to retrieve string parameter values
//            from input files for a specific instance.
// PARAMETERS ::
// + nodeId           : const NodeAddress   : nodeId. Can be ANY_NODEID.
// + interfaceAddress : const NodeAddress   : IP address of interface.
//                                            Can be ANY_IP.
// + nodeInput        : const NodeInput*    : Pointer to node input.
// + parameterName    : const char*         : Parameter name.
// + parameterInstanceNumber   : const int  : Instance number.
// + fallbackIfNoInstanceMatch : const BOOL : Selects parameter without
//                                            instance number if
//                                            parameterInstanceNumber
//                                            cannot be matched.
// + wasFound                  : BOOL*      : Storage for success of seach.
// + parameterValue            : char*      : Storage for parameter value.
// RETURN :: void :
// **/
void
IO_ReadStringInstance(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    char *parameterValue);

// /**
// API :: IO_ReadBoolInstance
// PURPOSE :: This API is used to retrieve boolean parameter values
//            from input files for a specific instance.
// PARAMETERS ::
// + nodeId           : const NodeAddress   : nodeId. Can be ANY_NODEID.
// + interfaceAddress : const NodeAddress   : IP address of interface.
//                                            Can be ANY_IP.
// + nodeInput        : const NodeInput*    : Pointer to node input.
// + parameterName    : const char*         : Parameter name.
// + parameterInstanceNumber   : const int  : Instance number.
// + fallbackIfNoInstanceMatch : const BOOL : Selects parameter without
//                                            instance number if
//                                            parameterInstanceNumber
//                                            cannot be matched.
// + wasFound                  : BOOL*      : Storage for success of seach.
// + parameterValue            : BOOL*       : Storage for parameter value.
// RETURN :: void :
// **/
void
IO_ReadBoolInstance(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    BOOL *parameterValue);

// /**
// API :: IO_ReadIntInstance
// PURPOSE :: This API is used to retrieve integer parameter values
//            from input files for a specific instance.
// PARAMETERS ::
// + nodeId           : const NodeAddress   : nodeId. Can be ANY_NODEID.
// + interfaceAddress : const NodeAddress   : IP address of interface.
//                                            Can be ANY_IP.
// + nodeInput        : const NodeInput*    : Pointer to node input.
// + parameterName    : const char*         : Parameter name.
// + parameterInstanceNumber   : const int  : Instance number.
// + fallbackIfNoInstanceMatch : const BOOL : Selects parameter without
//                                            instance number if
//                                            parameterInstanceNumber
//                                            cannot be matched.
// + wasFound                  : BOOL*      : Storage for success of seach.
// + parameterValue            : int*       : Storage for parameter value.
// RETURN :: void :
// **/
void
IO_ReadIntInstance(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    int *parameterValue);

// /**
// API :: IO_ReadDoubleInstance
// PURPOSE :: This API is used to retrieve double parameter values
//            from input files for a specific instance.
// PARAMETERS ::
// + nodeId           : const NodeAddress   : nodeId. Can be ANY_NODEID.
// + interfaceAddress : const NodeAddress   : IP address of interface.
//                                            Can be ANY_IP.
// + nodeInput        : const NodeInput*    : Pointer to node input.
// + parameterName    : const char*         : Parameter name.
// + parameterInstanceNumber   : const int  : Instance number.
// + fallbackIfNoInstanceMatch : const BOOL : Selects parameter without
//                                            instance number if
//                                            parameterInstanceNumber
//                                            cannot be matched.
// + wasFound                  : BOOL*      : Storage for success of seach.
// + parameterValue            : double*    : Storage for parameter value.
// RETURN :: void :
// **/
void
IO_ReadDoubleInstance(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    double *parameterValue);

// /**
// API :: IO_ReadFloatInstance
// PURPOSE :: This API is used to retrieve float parameter values
//            from input files for a specific instance.
// PARAMETERS ::
// + nodeId           : const NodeAddress   : nodeId. Can be ANY_NODEID.
// + interfaceAddress : const NodeAddress   : IP address of interface.
//                                            Can be ANY_IP.
// + nodeInput        : const NodeInput*    : Pointer to node input.
// + parameterName    : const char*         : Parameter name.
// + parameterInstanceNumber   : const int  : Instance number.
// + fallbackIfNoInstanceMatch : const BOOL : Selects parameter without
//                                            instance number if
//                                            parameterInstanceNumber
//                                            cannot be matched.
// + wasFound                  : BOOL*      : Storage for success of seach.
// + parameterValue            : float*     : Storage for parameter value.
// RETURN :: void :
// **/
void
IO_ReadFloatInstance(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    float *parameterValue);

// /**
// API :: IO_ReadTimeInstance
// PURPOSE :: This API is used to retrieve time parameter values
//            from input files for a specific instance.
// PARAMETERS ::
// + nodeId           : const NodeAddress   : nodeId. Can be ANY_NODEID.
// + interfaceAddress : const NodeAddress   : IP address of interface.
//                                            Can be ANY_IP.
// + nodeInput        : const NodeInput*    : Pointer to node input.
// + parameterName    : const char*         : Parameter name.
// + parameterInstanceNumber   : const int  : Instance number.
// + fallbackIfNoInstanceMatch : const BOOL : Selects parameter without
//                                            instance number if
//                                            parameterInstanceNumber
//                                            cannot be matched.
// + wasFound                  : BOOL*      : Storage for success of seach.
// + parameterValue            : clocktype* : Storage for parameter value.
// RETURN :: void :
// **/
void
IO_ReadTimeInstance(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    clocktype *parameterValue);

// /**
// API :: IO_ReadCachedFileInstance
// PURPOSE :: This API is used to retrieve file parameter values
//            from input files for a specific instance.
// PARAMETERS ::
// + nodeId           : const NodeAddress   : nodeId. Can be ANY_NODEID.
// + interfaceAddress : const NodeAddress   : IP address of interface.
//                                            Can be ANY_IP.
// + nodeInput        : const NodeInput*    : Pointer to node input.
// + parameterName    : const char*         : Parameter name.
// + parameterInstanceNumber   : const int  : Instance number.
// + fallbackIfNoInstanceMatch : const BOOL : Selects parameter without
//                                            instance number if
//                                            parameterInstanceNumber
//                                            cannot be matched.
// + wasFound                  : BOOL*      : Storage for success of seach.
// + parameterValue            : NodeInput* : Storage for parameter value.
// RETURN :: void :
// **/
void
IO_ReadCachedFileInstance(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    NodeInput *parameterValue);

//---------------------------------------------------------------------------
// Address string parsing
//---------------------------------------------------------------------------

// /**
// API :: IO_ParseNodeIdHostOrNetworkAddress
// PURPOSE :: Parses a string for a nodeId, host address, or network
//            address.
// PARAMETERS ::
// + s[]                : const char    : String to parse.
// + outputNodeAddress  : NodeAddress*  : Storage for nodeId or IP address.
// + numHostBits        : int*          : Storage for number of host bits
//                                        (32 - number of network bits).
//                                        Defaults to zero, unless it's an
//                                        actual network address.
// + isNodeId           : BOOL*         : Storage for whether the string is
//                                        a nodeId.
// RETURN :: void :
// **/
void
IO_ParseNodeIdHostOrNetworkAddress(
    const char s[],
    NodeAddress *outputNodeAddress,
    int *numHostBits,
    BOOL *isNodeId);



// /**
// API :: IO_ParseNodeIdOrHostAddress
// PURPOSE :: Parses a string for a nodeId or host address.
// PARAMETERS ::
// + s[]                : const char    : String to parse.
// + outputNodeAddress  : NodeAddress*  : Storage for nodeId or IP address.
// + isNodeId           : BOOL*         : Storage for whether the string is
//                                        a nodeId.
// RETURN :: void :
// **/
void
IO_ParseNodeIdOrHostAddress(
    const char s[],
    NodeAddress *outputNodeAddress,
    BOOL *isNodeId);

// /**
// API :: IO_ParseNetworkAddress
// PURPOSE :: Parses a string for a network address.
// PARAMETERS ::
// + s[]                : const char    : String to parse.
// + outputNodeAddress  : NodeAddress*  : Storage for network address.
// + numHostBits        : int*          : Storage for number of host bits
//                                        (32 - number of network bits).
// RETURN :: void :
// **/
void
IO_ParseNetworkAddress(
    const char s[],
    NodeAddress *outputNodeAddress,
    int *numHostBits);

//---------------------------------------------------------------------------
// NodeInput helper functions
//---------------------------------------------------------------------------

// /**
// API :: IO_FreeNodeInput
// PURPOSE :: Frees a NodeInput struct. (Currently unused.)
// PARAMETERS ::
// + nodeInput  : NodeInput*    : Pointer to node input.
// RETURN :: void :
// **/
void
IO_FreeNodeInput(NodeInput *nodeInput);

//---------------------------------------------------------------------------
// Output functions
//---------------------------------------------------------------------------

// /**
// API :: IO_PrintStat
// PURPOSE :: Print out the relevant stat in "buf", along with the
//            node id and the layer type generating this stat.
// PARAMETERS ::
// + node               : Node*         : The node generating the stat.
// + layer              : const char*   : The layer generating the stat.
// + protocol           : const char*   : The protocol generating the stat.
// + interfaceAddress   : NodeAddress   : Interface address.
// + instanceId         : int           : Instance id.
// + buf                : const char*   : String which has the statistic to
//                                        be printed out
// RETURN :: void :
// **/
void
IO_PrintStat(
    Node *node,
    const char *layer,
    const char *protocol,
    NodeAddress interfaceAddress,
    int instanceId,
    const char *fmt,
    ...);


//---------------------------------------------------------------------------
// Application input parsing functions.
//---------------------------------------------------------------------------

// /**
// API :: IO_AppParseSourceAndDestStrings
// PURPOSE ::  Application input parsing API. Parses the source and
//             destination strings.
// PARAMETERS ::
// + node           : Node*         : A pointer to Node.
// + inputString    : const char*   : The input string.
// + sourceString   : const char*   : The source string.
// + sourceNodeId   : NodeAddress*  : A pointer to NodeAddress.
// + sourceAddr     : NodeAddress*  : A pointer to NodeAddress.
// + destString     : const char*   : Const char pointer.
// + destNodeId     : NodeAddress*  : A pointer to NodeAddress.
// + destAddr       : NodeAddress*  : A pointer to NodeAddress.
// RETURN :: void :
// **/
void IO_AppParseSourceAndDestStrings(
    Node *node,
    const char *inputString,
    const char *sourceString,
    NodeAddress *sourceNodeId,
    NodeAddress *sourceAddr,
    const char *destString,
    NodeAddress *destNodeId,
    NodeAddress *destAddr);

// /**
// API :: IO_AppParseSourceString
// PURPOSE ::  Application input parsing API. Parses the source
//             string.
// PARAMETERS ::
// + node           : Node*         : A pointer to Node.
// + inputString    : const char*   : The input string.
// + sourceString   : const char*   : The source string.
// + sourceNodeId   : NodeAddress*  : A pointer to NodeAddress.
// + sourceAddr     : NodeAddress*  : A pointer to NodeAddress.
// RETURN :: void :
// **/
void IO_AppParseSourceString(
    Node *node,
    const char *inputString,
    const char *sourceString,
    NodeAddress *sourceNodeId,
    NodeAddress *sourceAddr);

// /**
// API :: IO_AppParseDestString
// PURPOSE ::  Application input parsing API. Parses the
//             destination string.
// PARAMETERS ::
// + node           : Node*         : A pointer to Node.
// + inputString    : const char*   : The input string.
// + destString     : const char*   : Const char pointer.
// + destNodeId     : NodeAddress*  : A pointer to NodeAddress.
// + destAddr       : NodeAddress*  : A pointer to NodeAddress.
// RETURN :: void :
// **/
void IO_AppParseDestString(
    Node *node,
    const char *inputString,
    const char *destString,
    NodeAddress *destNodeId,
    NodeAddress *destAddr);

// /**
// API :: IO_AppParseHostString
// PURPOSE ::  Application input parsing API. Parses the
//             host string.
// PARAMETERS ::
// + node           : Node*         : A pointer to Node.
// + inputString    : const char*   : The input string.
// + destString     : const char*   : Const char pointer.
// + destNodeId     : NodeAddress*  : A pointer to NodeAddress.
// + destAddr       : NodeAddress*  : A pointer to NodeAddress.
// RETURN :: void :
// **/
void IO_AppParseHostString(
    Node *node,
    const char *inputString,
    const char *destString,
    NodeAddress *destNodeId,
    NodeAddress *destAddr);

// /**
// API :: IO_AppForbidSameSourceAndDest
// PURPOSE ::  Application input checking API. Checks for the same
//             source and destination node id. Calls abort() for same
//             source and destination.
// PARAMETERS ::
// + inputString    : const char*   : The input string.
// + sourceNodeId   : NodeAddress   : Source node id, read from the
//                                    application input file.
// + destNodeId     : NodeAddress   : Destination node id, read from the
//                                    application input file.
// RETURN :: void :
// **/
void IO_AppForbidSameSourceAndDest(
    const char *inputString,
    NodeAddress sourceNodeId,
    NodeAddress destNodeId);

// Used by OSPF also.

// /**
// API :: QualifierMatches
// PURPOSE :: This is an auxiliary API used by the IO_Read...()
//            set of APIs.
// PARAMETERS ::
// + nodeId             : const NodeAddress : nodeId to select for.
//                                            ANY_NODEID matches any nodeId.
// + interfaceAddress   : const NodeAddress : IP address to select for.
//                                            ANY_IP matches any IP address.
// + qualifier          : const char*       : String containing the
//                                            qualifier.
// + matchType          : int*              : Stores the type of the match,
//                                            if any, e.g. byIP, byNodeId
// RETURN :: BOOL : Returns TRUE if match found. FALSE, otherwise.
// **/
BOOL
QualifierMatches(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    char *qualifier,
    int  *matchType);

#ifdef __cplusplus
}
#endif

// ------------------------------------------------------------------------
// Overloaded API for Ipv6 compatibility
//
// NOTE:
// Have to declare Overloaded APIs separately below this __cplusplus brace
// block instead of declaring it just below the original one. If it is
// declared just below the original one inside the  __cplusplus brace
// it is been treated as c function and showing compilation error.
// ------------------------------------------------------------------------

//--------------------------------------------------------------------------
// Input files parameter retrieval - Ipv6 compatibility
//--------------------------------------------------------------------------


// /**
// API :: IO_ReadBool
// PURPOSE :: This API is used to retrieve boolean parameter values
//            from input files.  Overloaded API for Ipv6 compatibility.
// PARAMETERS ::
// + nodeId           : const NodeAddress   : nodeId. Can be ANY_NODEID.
// + interfaceAddress : const in6_addr*     : IPv6 address of interface.
// + nodeInput        : const NodeInput*    : Pointer to node input.
// + parameterName    : const char*         : Parameter name.
// + wasFound         : BOOL*               : Storage for success of seach.
// + parameterValue   : BOOL*               : Storage for parameter value.
// RETURN :: void :
// **/
void
IO_ReadBool(
    const NodeId nodeId,
    const in6_addr* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    BOOL *parameterValue);

// /**
// FUNCTION            :: IO_ReadBool
// PURPOSE             :: Reads boolean value for specified ATM address.
// PARAMETERS          :
// +nodeId             : const NodeId : NodeId for which parameter has
//                                 has to be searched.
// +interfaceAddress  : const AtmAddress* :
//                                 ATM Interface address
// +nodeInput         : const NodeInput *:
//                                 pointer to configuration inputs
// +parameterName     : const char *: Parameter to be read
// +wasFound          : BOOL *: Parameter found or not
// +parameterValue    : BOOL *: Parameter's value if found.
// RETURN             :: None :
// NOTE:
// Overloaded API IO_ReadBool()
// **/
void
IO_ReadBool(
    const NodeId nodeId,
    const AtmAddress* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    BOOL *parameterValue);

// /**
// API :: IO_ReadBool
// PURPOSE :: This API is used to retrieve boolean parameter values
//            from input files.  Overloaded API for Ipv6 compatibility.
// PARAMETERS ::
// + nodeId           : const NodeAddress   : nodeId. Can be ANY_NODEID.
// + address          : const Address*      : Address of interface.
//                                            Can be NULL.
// + nodeInput        : const NodeInput*    : Pointer to node input.
// + parameterName    : const char*         : Parameter name.
// + wasFound         : BOOL*               : Storage for success of seach.
// + parameterValue   : BOOL*               : Storage for parameter value.
// RETURN :: void :
// **/
void
IO_ReadBool(
    const NodeId nodeId,
    const Address* address,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    BOOL *parameterValue);

// /**
// API :: IO_ReadString
// PURPOSE :: This API is used to retrieve a string parameter
//            value from input files. Overloaded API for Ipv6 compatibility.
// PARAMETERS ::
// + nodeId             : const NodeId      : nodeId. Can be ANY_NODEID.
// + interfaceAddress   : const in6_addr*   : IP address of interface.
//                                            Can be NULL  .
// + nodeInput          : const NodeInput*  : Pointer to node input.
// + index              : const char*       : Parameter name.
// + wasFound           : BOOL*             : Storage for success of seach.
// + readVal            : char*             : Storage for parameter value.
// RETURN :: void :
// **/
void
IO_ReadString(
    const NodeId nodeId,
    const in6_addr* interfaceAddress,
    const NodeInput *nodeInput,
    const char *index,
    BOOL *wasFound,
    char *readVal);

// /**
// FUNCTION            :: IO_ReadString
// PURPOSE             :: Reads string value for specified ATM address.
// PARAMETERS          ::
// + nodeId             : const NodeId : NodeId for which parameter has
//                                 has to be searched.
// + interfaceAddress  : const AtmAddress* : ATM Interface address
// + nodeInput         : const NodeInput *: pointer to configuration inputs
// + parameterName     : const char *: Parameter to be read
// + wasFound          : BOOL *: Parameter found or not
// + parameterValue    : char *: Parameter's value if found.
// RETURN             :: void :
// NOTE:
// Overloaded API IO_ReadString()
// **/
void
IO_ReadString(
    const NodeId nodeId,
    const AtmAddress* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    char *parameterValue);

// /**
// API :: IO_ReadString
// PURPOSE :: This API is used to retrieve a string parameter
//            value from input files. Overloaded API for Ipv6 compatibility.
// PARAMETERS ::
// + nodeId             : const NodeId      : nodeId. Can be ANY_NODEID.
// + address            : const Address*    : IP address of interface.
//                                            Can be NULL.
// + nodeInput          : const NodeInput*  : Pointer to node input.
// + index              : const char*       : Parameter name.
// + wasFound           : BOOL*             : Storage for success of seach.
// + readVal            : char*             : Storage for parameter value.
// RETURN :: void :
// **/
void
IO_ReadString(
    const NodeAddress nodeId,
    const Address* address,
    const NodeInput *nodeInput,
    const char *index,
    BOOL *wasFound,
    char *readVal);

// /**
// API :: IO_ReadInt
// PURPOSE :: This API is used to retrieve an integer parameter
//            value from input files. Overloaded API for Ipv6 compatibility.
// PARAMETERS ::
// + nodeId             : const NodeId      : nodeId. Can be ANY_NODEID.
// + interfaceAddress   : const in6_addr*   : IPv6 address of interface.
//                                            Can be NULL.
// + nodeInput          : const NodeInput*  : Pointer to node input.
// + index              : const char*       : Parameter name.
// + wasFound           : BOOL*             : Storage for success of seach.
// + readVal            : int*              : Storage for parameter value.
// RETURN :: void :
// **/
void
IO_ReadInt(
    const NodeId nodeId,
    const in6_addr* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL* wasFound,
    int *parameterValue);

// /**
// API :: IO_ReadInt
// PURPOSE :: This API is used to retrieve an integer parameter
//            value from input files. Overloaded API for Ipv6 compatibility.
// PARAMETERS ::
// + nodeId             : const NodeId      : nodeId. Can be ANY_NODEID.
// + address            : const Address*    : Address of interface.
//                                            Can be NULL.
// + nodeInput          : const NodeInput*  : Pointer to node input.
// + index              : const char*       : Parameter name.
// + wasFound           : BOOL*             : Storage for success of seach.
// + readVal            : int*              : Storage for parameter value.
// RETURN :: void :
// **/
void
IO_ReadInt(
    const NodeId nodeId,
    const Address* address,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL* wasFound,
    int *parameterValue);

void
IO_ReadInt(
    const NodeId nodeId,
    const Address* address,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL* wasFound,
    int *parameterValue,
    BOOL required,
    BOOL minReq,
    int minVal,
    BOOL maxReq,
    int maxVal);

// /**
// FUNCTION            :: IO_ReadInt
// PURPOSE             :: Reads int value for specified ATM address.
// PARAMETERS          ::
// +nodeId             : const NodeId  :: NodeId for which parameter has
//                                 has to be searched.
// +interfaceAddress  : const AtmAddress*  :
//                                 ATM Interface address
// +nodeInput         : const NodeInput* :
//                                 pointer to configuration inputs
// +parameterName     : const char* : Parameter to be read
// +wasFound          : BOOL* : Parameter found or not
// +parameterValue    : int* : Parameter's value if found.
// RETURN             :: void : None
// NOTE:
// Overloaded API IO_ReadInt()
// **/
void
IO_ReadInt(
    const NodeId nodeId,
    const AtmAddress* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL* wasFound,
    int *parameterValue);

// /**
// API :: IO_ReadDouble
// PURPOSE :: This API is used to retrieve a double parameter
//            value from input files. Overloaded API for Ipv6 compatibility.
// PARAMETERS ::
// + nodeId             : const NodeId      : nodeId. Can be ANY_NODEID.
// + interfaceAddress   : const in6_addr*   : IPv6 address of interface.
//                                            Can be NULL.
// + nodeInput          : const NodeInput*  : Pointer to node input.
// + index              : const char*       : Parameter name.
// + wasFound           : BOOL*             : Storage for success of seach.
// + readVal            : double*           : Storage for parameter value.
// RETURN :: void :
// **/

void
IO_ReadDouble(
    const NodeId nodeId,
    const in6_addr* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    double *parameterValue);

// /**
// FUNCTION            :: IO_ReadDouble
// PURPOSE             :: Reads double value for specified ATM address.
// PARAMETERS          ::
// +nodeId             : const NodeId : NodeId for which parameter has
//                                 has to be searched.
// +interfaceAddress  : const AtmAddress* :
//                                 ATM Interface address
// +nodeInput         : const NodeInput *:
//                                 pointer to configuration inputs
// +parameterName     : const char *: Parameter to be read
// +wasFound          : BOOL *: Parameter found or not
// +parameterValue    : double *: Parameter's value if found.
// RETURN             :: None :
// NOTE:
// Overloaded API IO_ReadDouble()
// **/
void
IO_ReadDouble(
    const NodeId nodeId,
    const AtmAddress* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    double *parameterValue);

void
IO_ReadInt64(
    const NodeId nodeId,
    const in6_addr* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    Int64 *parameterValue);




// /**
// API :: IO_ReadDouble
// PURPOSE :: This API is used to retrieve a double parameter
//            value from input files. Overloaded API for Ipv6 compatibility.
// PARAMETERS ::
// + nodeId             : const NodeId      : nodeId. Can be ANY_NODEID.
// + address            : const Address*    : Address of interface.
//                                            Can be NULL.
// + nodeInput          : const NodeInput*  : Pointer to node input.
// + index              : const char*       : Parameter name.
// + wasFound           : BOOL*             : Storage for success of seach.
// + readVal            : double*           : Storage for parameter value.
// RETURN :: void :
// **/
void
IO_ReadDouble(
    const NodeId nodeId,
    const Address* address,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    double *parameterValue);

void
IO_ReadInt64(
    const NodeId nodeId,
    const Address* address,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    Int64 *parameterValue);




// /**
// API :: IO_ReadFloat
// PURPOSE :: This API is used to retrieve a float parameter
//            value from input files. Overloaded API for Ipv6 compatibility.
// PARAMETERS ::
// + nodeId             : const NodeId      : nodeId. Can be ANY_NODEID.
// + interfaceAddress   : const in6_addr*   : IPv6 address of interface.
//                                            Can be NULL.
// + nodeInput          : const NodeInput*  : Pointer to node input.
// + index              : const char*       : Parameter name.
// + wasFound           : BOOL*             : Storage for success of seach.
// + readVal            : float*            : Storage for parameter value.
// RETURN :: void :
// **/
void
IO_ReadFloat(
    const NodeId nodeId,
    const in6_addr* interfaceAddress,
    const NodeInput *nodeInput,
    const char *index,
    BOOL *wasFound,
    float *readVal);

// /**
// FUNCTION            :: IO_ReadFloat
// PURPOSE             :: Reads float value for specified ATM address.
// PARAMETERS          ::
// +nodeId             : const NodeId  : NodeId for which parameter has
//                                 has to be searched.
// +interfaceAddress  : const AtmAddress*  :
//                                 ATM Interface address
// +nodeInput         : const NodeInput* :
//                                 pointer to configuration inputs
// +parameterName     : const char* :: Parameter to be read
// +wasFound          : BOOL* : Parameter found or not
// +parameterValue    : flaot* : Parameter's value if found.
// RETURN             :: void : None :
// NOTE:
// Overloaded API IO_ReadFloat()
// **/
void
IO_ReadFloat(
    const NodeId nodeId,
    const AtmAddress* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    float *parameterValue);

// /**
// API :: IO_ReadFloat
// PURPOSE :: This API is used to retrieve a float parameter
//            value from input files. Overloaded API for Ipv6 compatibility.
// PARAMETERS ::
// + nodeId             : const NodeId      : nodeId. Can be ANY_NODEID.
// + address            : const Address*    : Address of interface.
//                                            Can be NULL.
// + nodeInput          : const NodeInput*  : Pointer to node input.
// + index              : const char*       : Parameter name.
// + wasFound           : BOOL*             : Storage for success of seach.
// + readVal            : float*            : Storage for parameter value.
// RETURN :: void :
// **/
void
IO_ReadFloat(
    const NodeId nodeId,
    const Address* address,
    const NodeInput *nodeInput,
    const char *index,
    BOOL *wasFound,
    float *readVal);

void
IO_ReadFloat(
    const NodeId nodeId,
    const Address* address,
    const NodeInput *nodeInput,
    const char *index,
    BOOL *wasFound,
    float *readVal,
    BOOL required,
    BOOL minReq,
    float minVal,
    BOOL maxReq,
    float maxVal);

// /**
// API :: IO_ReadTime
// PURPOSE :: This API is used to retrieve time parameter
//            value from input files. Overloaded API for Ipv6 compatibility.
// PARAMETERS ::
// + nodeId             : const NodeId      : nodeId. Can be ANY_NODEID.
// + interfaceAddress   : const in6_addr*   : IPv6 address of interface.
//                                            Can be NULL.
// + nodeInput          : const NodeInput*  : Pointer to node input.
// + index              : const char*       : Parameter name.
// + wasFound           : BOOL*             : Storage for success of seach.
// + readVal            : clocktype*        : Storage for parameter value.
// RETURN :: void :
// **/

void
IO_ReadTime(
    const NodeAddress nodeId,
    const in6_addr* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    clocktype *parameterValue);

// /**
// API                 :: IO_ReadTime
// PURPOSE             :: Reads time value for specified ATM address.
// PARAMETERS          ::
// +nodeId             : const NodeId  : NodeId for which parameter has
//                                 has to be searched.
// +interfaceAddress  : const AtmAddress* :
//                                ATM Interface address
// +nodeInput         : const NodeInput* :
//                                 pointer to configuration inputs
// +parameterName     : const char *parameterName : Parameter to be read
// +wasFound          : BOOL* : Parameter found or not
// +parameterValue    : clocktype* :: Parameter's value if found.
// RETURN             :: void :
// NOTE:
// Overloaded API IO_ReadTime()
// **/
void
IO_ReadTime(
    const NodeAddress nodeId,
    const AtmAddress* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    clocktype *parameterValue);

// /**
// API          :: IO_ReadTime
// PURPOSE      :: This API is used to retrieve time parameter
//               value from input files. Overloaded API for Ipv6 compatibility.
// PARAMETERS ::
// + nodeId             : const NodeId      : nodeId. Can be ANY_NODEID.
// + address            : const Address*    : Address of interface.
//                                            Can be NULL.
// + nodeInput          : const NodeInput*  : Pointer to node input.
// + index              : const char*       : Parameter name.
// + wasFound           : BOOL*             : Storage for success of seach.
// + readVal            : clocktype*        : Storage for parameter value.
// RETURN       :: void :
// **/
void
IO_ReadTime(
    const NodeAddress nodeId,
    const Address* address,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    clocktype *parameterValue);

// /**
// FUNCTION            :: IO_ReadBoolInstance
// PURPOSE             :: Reads BOOL value for specified ATM address.
// PARAMETERS          ::
// +nodeId             :const NodeId : NodeId for which parameter has
//                                 has to be searched.
// +interfaceAddress  : const AtmAddress* :
//                                 ATM Interface address
// +nodeInput         : const NodeInput *:
//                                 pointer to configuration inputs
// +parameterName     : const char *: Parameter to be read
// +parameterInstanceNUmber : const int parameterInstanceNumber::
//                                 parameter instance number
// +fallbackIfNoInstanceMatch: const BOOL fallbackIfNoInstanceMatch :: get the
//                             previous match if no match is found.
// +wasFound          : BOOL *: Parameter found or not
// +parameterValue    : BOOL *: Parameter's value if found.
// RETURN             :: None :
// NOTE:
// Overloaded API IO_ReadBoolInstance()
// **/
void
IO_ReadBoolInstance(
    const NodeId nodeId,
    const AtmAddress* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    BOOL *parameterValue);


//---------------------------------------------------------------------------
// Input files parameter retrieval by instance qualifier - Ipv6 compatibility
//---------------------------------------------------------------------------

// /**
// API :: IO_ReadStringInstance
// PURPOSE :: This API is used to retrieve string parameter values
//            from input files for a specific instance.
//            Overloaded API for Ipv6 compatibility.
// PARAMETERS ::
// + nodeId           :   const NodeId      : nodeId. Can be ANY_NODEID.
// + interfaceAddress :   const in6_addr*   : IPv6 address of interface.
//                                            Can be NULL.
// + nodeInput        : const NodeInput*    : Pointer to node input.
// + parameterName    : const char*         : Parameter name.
// + parameterInstanceNumber   : const int  : Instance number.
// + fallbackIfNoInstanceMatch : const BOOL : Selects parameter without
//                                            instance number if
//                                            parameterInstanceNumber
//                                            cannot be matched.
// + wasFound                  : BOOL*      : Storage for success of seach.
// + parameterValue            : char*      : Storage for parameter value.
// RETURN :: void :
// **/

void
IO_ReadStringInstance(
    const NodeId nodeId,
    const in6_addr* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    char *parameterValue);

// /**
// FUNCTION            :: IO_ReadStringInstance
// PURPOSE             :: Reads string value for specified ATM address.
// PARAMETERS          :
// +nodeId             : const NodeId : NodeId for which parameter has
//                                 has to be searched.
// +interfaceAddress  : const AtmAddress* :
//                                 ATM Interface address
// +nodeInput         : const NodeInput *:
//                                 pointer to configuration inputs
// +parameterName     : const char *: Parameter to be read
// +parameterInstanceNUmber : const int parameterInstanceNumber::
//                                 parameter instance number
// +fallbackIfNoInstanceMatch: const BOOL fallbackIfNoInstanceMatch :: get the
//                             previous match if no match is found.
// +wasFound          : BOOL *: Parameter found or not
// +parameterValue    : char *: Parameter's value if found.
// RETURN             : void  :None
// NOTE:
// Overloaded API IO_ReadStringInstance()
// **/
void
IO_ReadStringInstance(
    const NodeId nodeId,
    const AtmAddress* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    char *parameterValue);

// /**
// API :: IO_ReadStringInstance
// PURPOSE :: This API is used to retrieve string parameter values
//            from input files for a specific instance.
//            Overloaded API for Ipv6 compatibility.
// PARAMETERS ::
// + nodeId           :   const NodeId      : nodeId. Can be ANY_NODEID.
// + address          :   const Address*    : Address of interface.
//                                            Can be NULL.
// + nodeInput        : const NodeInput*    : Pointer to node input.
// + parameterName    : const char*         : Parameter name.
// + parameterInstanceNumber   : const int  : Instance number.
// + fallbackIfNoInstanceMatch : const BOOL : Selects parameter without
//                                            instance number if
//                                            parameterInstanceNumber
//                                            cannot be matched.
// + wasFound                  : BOOL*      : Storage for success of seach.
// + parameterValue            : char*      : Storage for parameter value.
// RETURN :: void :
// **/
void
IO_ReadStringInstance(
    const NodeId nodeId,
    const Address* address,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    char *parameterValue);

// /**
// API :: IO_ReadIntInstance
// PURPOSE :: This API is used to retrieve integer parameter values
//            from input files for a specific instance.
//            Overloaded API for Ipv6 compatibility.
// PARAMETERS ::
// + nodeId             : const NodeId      : nodeId. Can be ANY_NODEID.
// + interfaceAddress   : const in6_addr*   : IPv6 address of interface.
//                                            Can be NULL.
// + nodeInput          : const NodeInput*  : Pointer to node input.
// + parameterName      : const char*       : Parameter name.
// + parameterInstanceNumber   : const int  : Instance number.
// + fallbackIfNoInstanceMatch : const BOOL : Selects parameter without
//                                            instance number if
//                                            parameterInstanceNumber
//                                            cannot be matched.
// + wasFound                  : BOOL*      : Storage for success of seach.
// + parameterValue            : int*       : Storage for parameter value.
// RETURN :: void :
// **/

void
IO_ReadIntInstance(
    const NodeId nodeId,
    const in6_addr* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    int *parameterValue);

///**
// FUNCTION            :: IO_ReadIntInstance
// PURPOSE             :: Reads int value for specified ATM address.
// PARAMETERS          :
// +nodeId             : const NodeId : NodeId for which parameter has
//                                 has to be searched.
// +interfaceAddress  : const AtmAddress* :
//                                ATM Interface address
// +nodeInput         : const NodeInput *:
//                                 pointer to configuration inputs
// +parameterName     : const char *: Parameter to be read
// +parameterInstanceNUmber : const int :
//                                 parameter instance number
// +fallbackIfNoInstanceMatch: const BOOL : get the
//                             previous match if no match is found.
// +wasFound          : BOOL *: Parameter found or not
// +parameterValue    : int *: Parameter's value if found.
// RETURN             : void : None
// NOTE:
// Overloaded API IO_ReadIntInstance()
// **/
void
IO_ReadIntInstance(
    const NodeId nodeId,
    const AtmAddress* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    int *parameterValue);

// /**
// API :: IO_ReadIntInstance
// PURPOSE :: This API is used to retrieve integer parameter values
//            from input files for a specific instance.
//            Overloaded API for Ipv6 compatibility.
// PARAMETERS ::
// + nodeId             : const NodeId      : nodeId. Can be ANY_NODEID.
// + address            : const Address*    : Address of interface.
//                                            Can be NULL.
// + nodeInput          : const NodeInput*  : Pointer to node input.
// + parameterName      : const char*       : Parameter name.
// + parameterInstanceNumber   : const int  : Instance number.
// + fallbackIfNoInstanceMatch : const BOOL : Selects parameter without
//                                            instance number if
//                                            parameterInstanceNumber
//                                            cannot be matched.
// + wasFound                  : BOOL*      : Storage for success of seach.
// + parameterValue            : int*       : Storage for parameter value.
// RETURN :: void :
// **/
void
IO_ReadIntInstance(
    const NodeAddress nodeId,
    const Address* address,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    int *parameterValue);

// /**
// API :: IO_ReadDoubleInstance
// PURPOSE :: This API is used to retrieve double parameter values
//            from input files for a specific instance.
//            Overloaded API for Ipv6 compatibility.
// PARAMETERS ::
// + nodeId             : const NodeId      : nodeId. Can be ANY_NODEID.
// + interfaceAddress   : const in6_addr*   : IPv6 address of interface.
//                                            Can be NULL.
// + nodeInput        : const NodeInput*    : Pointer to node input.
// + parameterName    : const char*         : Parameter name.
// + parameterInstanceNumber   : const int  : Instance number.
// + fallbackIfNoInstanceMatch : const BOOL : Selects parameter without
//                                            instance number if
//                                            parameterInstanceNumber
//                                            cannot be matched.
// + wasFound                  : BOOL*      : Storage for success of seach.
// + parameterValue            : double*    : Storage for parameter value.
// RETURN :: void :
// **/
void
IO_ReadDoubleInstance(
    const NodeId nodeId,
    const in6_addr* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    double *parameterValue);


// /**
// FUNCTION            :: IO_ReadDoubleInstance
// PURPOSE             :: Reads double value for specified ATM address.
// PARAMETERS          :
// +nodeId             :const NodeId : NodeId for which parameter has
//                                 has to be searched.
// +interfaceAddress  : const AtmAddress* :
//                                 ATM Interface address
// +nodeInput         : const NodeInput *:
//                                 pointer to configuration inputs
// +parameterName     : const char *: Parameter to be read
// +parameterInstanceNUmber : const int :
//                                 parameter instance number
// +fallbackIfNoInstanceMatch: const BOOL : get the
//                             previous match if no match is found.
// +wasFound          : BOOL *: Parameter found or not
// +parameterValue    : double *: Parameter's value if found.
// RETURN :: void : None
// NOTE:
// Overloaded API IO_ReadDoubleInstance()
// **/
void
IO_ReadDoubleInstance(
    const NodeId nodeId,
    const AtmAddress* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    double *parameterValue);


// /**
// API :: IO_ReadDoubleInstance
// PURPOSE :: This API is used to retrieve double parameter values
//            from input files for a specific instance.
//            Overloaded API for Ipv6 compatibility.
// PARAMETERS ::
// + nodeId             : const NodeId      : nodeId. Can be ANY_NODEID.
// + address            : const Address*    : IPv6 address of interface.
//                                            Can be NULL.
// + nodeInput        : const NodeInput*    : Pointer to node input.
// + parameterName    : const char*         : Parameter name.
// + parameterInstanceNumber   : const int  : Instance number.
// + fallbackIfNoInstanceMatch : const BOOL : Selects parameter without
//                                            instance number if
//                                            parameterInstanceNumber
//                                            cannot be matched.
// + wasFound                  : BOOL*      : Storage for success of seach.
// + parameterValue            : double*    : Storage for parameter value.
// RETURN :: void :
// **/
void
IO_ReadDoubleInstance(
    const NodeAddress nodeId,
    const Address* address,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    double *parameterValue);

// /**
// API :: IO_ReadFloatInstance
// PURPOSE :: This API is used to retrieve float parameter values
//            from input files for a specific instance.
//            Overloaded API for Ipv6 compatibility.
// PARAMETERS ::
// + nodeId             : const NodeId      : nodeId. Can be ANY_NODEID.
// + interfaceAddress   : const in6_addr*   : IPv6 address of interface.
//                                            Can be NULL.
// + nodeInput        : const NodeInput*    : Pointer to node input.
// + parameterName    : const char*         : Parameter name.
// + parameterInstanceNumber   : const int  : Instance number.
// + fallbackIfNoInstanceMatch : const BOOL : Selects parameter without
//                                            instance number if
//                                            parameterInstanceNumber
//                                            cannot be matched.
// + wasFound                  : BOOL*      : Storage for success of seach.
// + parameterValue            : float*     : Storage for parameter value.
// RETURN :: void :
// **/
void
IO_ReadFloatInstance(
    const NodeId nodeId,
    const in6_addr* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    float *parameterValue);

//  /**
// FUNCTION            :: IO_ReadFloatInstance
// PURPOSE             :: Reads float value for specified ATM address.
// PARAMETERS          :
// +nodeId             :const NodeId : NodeId for which parameter has
//                                 has to be searched.
// +interfaceAddress  : const AtmAddress* :
//                                 ATM Interface address
// +nodeInput         : const NodeInput *:
//                                 pointer to configuration inputs
// +parameterName     : const char *: Parameter to be read
// +parameterInstanceNUmber : const int :
//                                 parameter instance number
// +fallbackIfNoInstanceMatch: const BOOL : get the
//                             previous match if no match is found.
// +wasFound          : BOOL *: Parameter found or not
// +parameterValue    : float *: Parameter's value if found.
// RETURN             : void : None
// NOTE:
// Overloaded API IO_ReadFloatInstance()
// **/
void
IO_ReadFloatInstance(
    const NodeId nodeId,
    const AtmAddress* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    float *parameterValue);

// /**
// API :: IO_ReadFloatInstance
// PURPOSE :: This API is used to retrieve float parameter values
//            from input files for a specific instance.
//            Overloaded API for Ipv6 compatibility.
// PARAMETERS ::
// + nodeId             : const NodeId      : nodeId. Can be ANY_NODEID.
// + address            : const Address*    : Address of interface.
//                                            Can be NULL.
// + nodeInput        : const NodeInput*    : Pointer to node input.
// + parameterName    : const char*         : Parameter name.
// + parameterInstanceNumber   : const int  : Instance number.
// + fallbackIfNoInstanceMatch : const BOOL : Selects parameter without
//                                            instance number if
//                                            parameterInstanceNumber
//                                            cannot be matched.
// + wasFound                  : BOOL*      : Storage for success of seach.
// + parameterValue            : float*     : Storage for parameter value.
// RETURN :: void :
// **/
void
IO_ReadFloatInstance(
    const NodeAddress nodeId,
    const Address* address,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    float *parameterValue);

// /**
// API :: IO_ReadTimeInstance
// PURPOSE :: This API is used to retrieve time parameter values
//            from input files for a specific instance.
//            Overloaded API for Ipv6 compatibility.
// PARAMETERS ::
// + nodeId             : const NodeId      : nodeId. Can be ANY_NODEID.
// + interfaceAddress   : const in6_addr*   : IPv6 address of interface.
//                                            Can be NULL.
// + nodeInput        : const NodeInput*    : Pointer to node input.
// + parameterName    : const char*         : Parameter name.
// + parameterInstanceNumber   : const int  : Instance number.
// + fallbackIfNoInstanceMatch : const BOOL : Selects parameter without
//                                            instance number if
//                                            parameterInstanceNumber
//                                            cannot be matched.
// + wasFound                  : BOOL*      : Storage for success of seach.
// + parameterValue            : clocktype* : Storage for parameter value.
// RETURN :: void :
// **/
void
IO_ReadTimeInstance(
    const NodeId nodeId,
    const in6_addr* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    clocktype *parameterValue);

// /**
// FUNCTION            :: IO_ReadTimeInstance
// PURPOSE             :: Reads clocktype value for specified ATM address.
// PARAMETERS          :
// +nodeId             : const NodeId : NodeId for which parameter has
//                                 has to be searched.
// +interfaceAddress  : const AtmAddress* :
//                                 ATM Interface address
// +nodeInput         : const NodeInput *:
//                                 pointer to configuration inputs
// +parameterName     : const char *: Parameter to be read
// +parameterInstanceNUmber : const int :
//                                 parameter instance number
// +fallbackIfNoInstanceMatch: const BOOL : get the
//                             previous match if no match is found.
// +wasFound          : BOOL *: Parameter found or not
// +parameterValue    : clocktype *: Parameter's value if found.
// RETURN             :: void : None
// NOTE:
// Overloaded API IO_ReadTimeInstance()
// **/
void
IO_ReadTimeInstance(
    const NodeId nodeId,
    const AtmAddress* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    clocktype *parameterValue);



// /**
// API :: IO_ReadTimeInstance
// PURPOSE :: This API is used to retrieve time parameter values
//            from input files for a specific instance.
//            Overloaded API for Ipv6 compatibility.
// PARAMETERS ::
// + nodeId             : const NodeId      : nodeId. Can be ANY_NODEID.
// + address            : const Address*    : Address of interface.
//                                            Can be NULL.
// + nodeInput        : const NodeInput*    : Pointer to node input.
// + parameterName    : const char*         : Parameter name.
// + parameterInstanceNumber   : const int  : Instance number.
// + fallbackIfNoInstanceMatch : const BOOL : Selects parameter without
//                                            instance number if
//                                            parameterInstanceNumber
//                                            cannot be matched.
// + wasFound                  : BOOL*      : Storage for success of seach.
// + parameterValue            : clocktype* : Storage for parameter value.
// RETURN :: void :
// **/
void
IO_ReadTimeInstance(
    const NodeAddress nodeId,
    const Address* address,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    clocktype *parameterValue);


// /**
// API :: IO_ReadCachedFile
// PURPOSE :: This API is used to retrieve cached file parameter
//            value from input files.  Overloaded API for Ipv6 compatibility.
// PARAMETERS ::
// + nodeId             : const NodeAddress : nodeId. Can be ANY_NODEID.
// + address            : const Address*    : Address of interface.
//                                            Can be NULL.
// + nodeInput          : const NodeInput*  : Pointer to node input.
// + index              : const char*       : Parameter name.
// + wasFound           : BOOL*             : Storage for success of seach.
// + parameterValue     : NodeInput*        : Storage for parameter value.
// RETURN :: void :
// **/
void
IO_ReadCachedFile(
    const NodeAddress nodeId,
    const Address* address,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    NodeInput *parameterValue);

// /**
// API :: IO_ReadCachedFileInstance
// PURPOSE :: This API is used to retrieve file parameter values
//            from input files for a specific instance.
//            Overloaded API for Ipv6 compatibility.
// PARAMETERS ::
// + nodeId           : const NodeAddress   : nodeId. Can be ANY_NODEID.
// + address          : const Address*      : Address of interface.
//                                            Can be NULL.
// + nodeInput        : const NodeInput*    : Pointer to node input.
// + parameterName    : const char*         : Parameter name.
// + parameterInstanceNumber   : const int  : Instance number.
// + fallbackIfNoInstanceMatch : const BOOL : Selects parameter without
//                                            instance number if
//                                            parameterInstanceNumber
//                                            cannot be matched.
// + wasFound                  : BOOL*      : Storage for success of seach.
// + parameterValue            : NodeInput* : Storage for parameter value.
// RETURN :: void :
// **/
void
IO_ReadCachedFileInstance(
    const NodeAddress nodeId,
    const Address* address,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    NodeInput *parameterValue);


//---------------------------------------------------------------------------
// Output functions
//---------------------------------------------------------------------------

// /**
// API :: IO_PrintStat
// PURPOSE :: Print out the relevant stat in "buf", along with the
//            node id and the layer type generating this stat.
//            Overloaded API for Ipv6 compatibility.
// PARAMETERS ::
// + node               : Node*         : The node generating the stat.
// + layer              : const char*   : The layer generating the stat.
// + protocol           : const char*   : The protocol generating the stat.
// + interfaceAddress   : const char*   : The Interface address the stat.
// + instanceId         : int           : Instance id.
// + buf                : const char*   : String which has the statistic to
//                                        be printed out
// RETURN :: void :
// **/

void
IO_PrintStat(
    Node *node,
    const char *layer,
    const char *protocol,
    const char *ipAddrString,
    int instanceId,
    const char *fmt,
    ...);

//--------------------------------------------------------------------------
// Address string parsing - Ipv6 compatibility
//--------------------------------------------------------------------------
// /**
// API :: IO_ParseNodeIdHostOrNetworkAddress
// PURPOSE :: Parses a string for a nodeId, host address, or network
//            address. Overloaded API for Ipv6 compatibility.
// PARAMETERS ::
// + s[]        : const char    : String to parse.
// + ipAddress  : in6_addr*     : Storage for ipv6address.
// + isIpAddr   : BOOL*         : Storage for whether the string is
//                                        an ipv6address.
// + nodeId     : NodeId*       : Storage for nodeId.
// RETURN :: void :
// **/
void
IO_ParseNodeIdHostOrNetworkAddress(
    const char s[],
    in6_addr* ipAddress,
    BOOL* isIpAddr,
    NodeId* nodeId);

void
IO_ParseNodeIdHostOrNetworkAddress(
    const char inputString[],
    in6_addr* ipAddress,
    BOOL* isIpAddr,
    NodeId* nodeId,
    unsigned int* prefixLenth);


//For ATM
//--------------------------------------------------------------------------
// Address string parsing - ATM compatibility
//--------------------------------------------------------------------------
// /**
// API :: IO_ParseNodeIdHostOrNetworkAddress
// PURPOSE :: Parses a string for a nodeId, host address, or network
//            address. Overloaded API for Ipv6 compatibility.
// PARAMETERS ::
// + s[]        : const char    : String to parse.
// + atmAddress  : ATM addr*     : Storage for ATMaddress.
// + isAtmAddr   : BOOL*         : Storage for whether the string is
//                                        an ATMaddress.
// + nodeId     : NodeId*       : Storage for nodeId.
// RETURN :: void :
// **/
void
IO_ParseNodeIdHostOrNetworkAddress(
   const char addressString[],
   AtmAddress* atmAddr,
   BOOL* isAtmAddr,
   NodeId* nodeId);

//-----------------------------------------------------------------------------
// FUNCTION     IO_ParseNodeIdHostOrNetworkAddress
// PURPOSE      Parses a string for a nodeId, host address, or network
//              address. Overloaded API for Ipv6 compatibility.
//              When address is given in the form of TLA:NLA:SLA, then
//              value of TLA, NLA and SLA is stored at upper three unsigned
//              block of in6_addr. To get the corresponding ip address
//              we need to create it by using tla, nla, sla value from it
// PARAMETERS   const char addressString[]
//                   String to parse.
//              BOOL *isIpAddr
//                   Storage for whether the string is an Ipv6 address.
//              NodeId* nodeId
//                   Storage for node Identifier if present in the string
// ASSUMPTION   Node Identifier
//                         zero - the string does not cointain a nodeId
//              .        > zero - the string cointain a valid nodeId
// RETURN       None
// -----------------------------------------------------------------------------
// NOTE:
// Overloaded API  IO_ParseNodeIdHostOrNetworkAddress()
//-----------------------------------------------------------------------------
void
IO_ParseNodeIdHostOrNetworkAddress(
    const char inputString[],
    Address* ipAddress,
    BOOL* isNodeId);


// /**
// API :: IO_ParseNodeIdOrHostAddress
// PURPOSE :: Parses a string for a nodeId or host address.
// PARAMETERS ::
// + s[]                : const char    : String to parse.
// + outputNodeAddress  : in6_addr*     : Storage for ipv6address.
// + isNodeId           : BOOL*         : Storage for whether the string is
//                                        a nodeId.
// RETURN :: void :
// **/
void
IO_ParseNodeIdOrHostAddress(
    const char s[],
    in6_addr* outputNodeAddress,
    NodeId* nodeId);

// /**
// API :: IO_ParseNetworkAddress
// PURPOSE :: Parses a string for a network address.
//            Overloaded API for Ipv6 compatibility.
// PARAMETERS ::
// + s[]        : const char    : String to parse.
// + tla        : unsigned int* : Storage for tla
// + nla        : unsigned int* : Storage for nla.
// + sla        : unsigned int* : Storage for sla.
// RETURN :: void :
// **/
void
IO_ParseNetworkAddress(
    const char s[],
    unsigned int *tla,
    unsigned int *nla,
    unsigned int *sla);

void
IO_ParseNetworkAddress(
    const char inputString[],
    in6_addr* ipAddress,
    unsigned int* PrefixLenth);

//---------------------------------------------------------------------------
// Application input parsing functions.
//---------------------------------------------------------------------------

// /**
// API :: IO_AppParseSourceAndDestStrings
// PURPOSE ::  Application input parsing API. Parses the source and
//             destination strings. Overloaded for Ipv6 compatibility.
// PARAMETERS ::
// + node           : Node*         : A pointer to Node.
// + inputString    : const char*   : The input string.
// + sourceString   : const char*   : The source string.
// + sourceNodeId   : NodeId*       : A pointer to NodeId.
// + sourceAddr     : Address*: A pointer to Address.
// + destString     : const char*   : Const char pointer.
// + destNodeId     : NodeId*       : A pointer to NodeId.
// + destAddr       : Address*: A pointer to Address.
// RETURN :: void :
// **/

void
IO_AppParseSourceAndDestStrings(
    Node* node,
    const char* inputString,
    const char* sourceString,
    NodeId* sourceNodeId,
    Address* sourceAddr,
    const char* destString,
    NodeId* destNodeId,
    Address* destAddr);


// /**
// API :: IO_AppParseSourceString
// PURPOSE ::  Application input parsing API. Parses the source
//             string.  Overloaded for Ipv6 compatibility.
// PARAMETERS ::
// + node           : Node*         : A pointer to Node.
// + inputString    : const char*   : The input string.
// + sourceString   : const char*   : The source string.
// + sourceNodeId   : NodeAddress*  : A pointer to NodeAddress.
// + sourceAddr     : Address* : A pointer to Address.
// + networkType    : NetworkType : used when sourceString
//                                  represents node id.
// RETURN :: void :
// **/
void
IO_AppParseSourceString(
    Node* node,
    const char* inputString,
    const char* sourceString,
    NodeAddress* sourceNodeId,
    Address* sourceAddr,
    NetworkType networkType = NETWORK_IPV4);

// /**
// API :: IO_AppParseDestString
// PURPOSE ::  Application input parsing API. Parses the
//             destination string. Overloaded for Ipv6 compatibility.
// PARAMETERS ::
// + node           : Node*         : A pointer to Node.
// + inputString    : const char*   : The input string.
// + destString     : const char*   : Const char pointer.
// + destNodeId     : NodeAddress*  : A pointer to NodeAddress.
// + destAddr       : Address*: A pointer to Address.
// + networkType    : NetworkType : used when sourceString
//                                  represents node id.
// RETURN :: void :
// **/

void
IO_AppParseDestString(
    Node *node,
    const char *inputString,
    const char *destString,
    NodeAddress *destNodeId,
    Address *destAddr,
    NetworkType networkType = NETWORK_IPV4);

// /**
// API :: QualifierMatches
// PURPOSE :: This is an auxiliary API used by the IO_Read...()
//            set of APIs. Overloaded for Ipv6 compatibility
// PARAMETERS ::
// + nodeId             : const NodeId      : nodeId to select for.
//                                            ANY_NODEID matches any nodeId.
// + interfaceAddress   : const in6_addr    : IPv6 address to select for.
//                                            NULL matches any IP address.
// + qualifier          : const char*       : String containing the
//                                            qualifier.
// + matchType          : int*              : Stores the type of the match,
//                                            if any, e.g. byIP, byNodeId
// RETURN :: BOOL : Returns TRUE if match found. FALSE, otherwise.
// **/
BOOL
QualifierMatches(
    const NodeId nodeId,
    const in6_addr* interfaceAddress,
    char *qualifier,
    int  *matchType);


// /**
// API :: QualifierMatches
// PURPOSE :: This is an auxiliary API used by the IO_Read...()
//            set of APIs. Overloaded for Ipv6 compatibility
// PARAMETERS ::
// + nodeId             : const NodeId      : nodeId to select for.
//                                            ANY_NODEID matches any nodeId.
// + interfaceAddress   : const AtmAddress* : ATM address to select for.
//                                            NULL matches any IP address.
// + qualifier          : const char*       : String containing the
//                                            qualifier.
// + matchType          : int*              : Stores the type of the match,
//                                            if any, e.g. byIP, byNodeId
// RETURN :: BOOL : Returns TRUE if match found. FALSE, otherwise.
// **/
// -----------------------------------------------------------------------------
// NOTE:
// Overloaded Function QualifierMatches()
// **/

BOOL
QualifierMatches(
    const NodeId nodeId,
    const AtmAddress* interfaceAddress,
    char *qualifier,
    int  *matchType);

//--------------------------------------------------------------------------
// Utility Functions - Ipv6 compatibility
//--------------------------------------------------------------------------

// /**
// API :: IO_ConvertIpv6StringToAddress()
// PURPOSE :: Convert IPv6 address string to in6_addr structure.
//            API for Ipv6 compatibility.
// PARAMETERS ::
// + interfaceAddr  : char*         : Storage for ipv6address string
// + ipAddress      : in6_addr*     : Storage for ipv6address.
// RETURN :: void :
// **/

void IO_ConvertStringToAddress(
    char* interfaceAddr,
    in6_addr* ipv6Address);

// /**
// API :: IO_ConvertIpAddressToString
// PURPOSE :: Parses IPv6 address into a formatted string.
//            Overloaded API for Ipv6 compatibility.
// PARAMETERS ::
// + ipAddress      : in6_addr*     : Storage for ipv6address.
// + interfaceAddr  : char*         : Storage for ipv6address string
// RETURN :: void :
// **/

void IO_ConvertIpAddressToString(
        in6_addr* ipv6Address,
        char* interfaceAddr,
        BOOL ipv4EmbeddeAddr = FALSE);

void IO_ConvertIpAddressToString(
        char* ipv6Address,
        char* interfaceAddr,
        BOOL ipv4EmbeddeAddr = FALSE);

// /**
// API :: IO_ConvertIpAddressToString
// PURPOSE :: Parses IPv6 address into a formatted string.
//            Overloaded API for Ipv6 compatibility.
// PARAMETERS ::
// + ipAddress      : Address* : IP address info
// + interfaceAddr  : char*          : Storage for ipv6address string
// RETURN :: void :
// **/

void IO_ConvertIpAddressToString(
        Address* ipv6Address,
        char* interfaceAddr);

// /**
// API :: IO_ConvertStringToNodeAddress
// PURPOSE :: This API is used to covert a string parameter
//            to NodeAdress.
// PARAMETERS ::
// + addressString      : char* : IP address string info
// + outputNodeAddress  : NodeAddress* : Storage for IP address
// RETURN :: void :
// **/
void IO_ConvertStringToNodeAddress(
        const char addressString[],
        NodeAddress* outputNodeAddress);


// /**
// API :: IO_CheckIsSameAddress
// PURPOSE :: Compares IPv4 | IPv6 address.
//            API for Ipv6 compatibility.
// PARAMETERS ::
// + addr1    : Address : Storage for IPv4 | IPv6 address
//                              with network information.
// + addr2    : Address : Storage for IPv4 | IPv6 address
//                              with network information.
// RETURN :: BOOL :
// **/

BOOL IO_CheckIsSameAddress(
    Address addr1,
    Address addr2);


// /**
// API :: IO_ReadString
// PURPOSE :: This API is used to retrieve a string parameter
//            value from input files.
// PARAMETERS ::
// + node               : Node* node : Node pointer for which string is
//                                       to be  read.
// + nodeInput          : const NodeInput*  : Pointer to node input.
// + index              : const char*       : Parameter name.
// + wasFound           : BOOL*             : Storage for success of search.
// + readVal            : char*             : Storage for parameter value.
// RETURN :: void :
// **/

void
IO_ReadString(
    Node* node,
    const NodeAddress nodeId,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    char *parameterValue);

// /**
// API :: IO_CacheFile
// PURPOSE ::  This API is used to read an auxiliary input file into a
//             NodeInput struct Called for each file variable in the
//             config file.
// PARAMETERS  ::
// + nodeInput : const NodeInput* : Pointer to node input.
// + filename : const char* : Path to input file.
// RETURN :: void : NULL
// **/

void
IO_CacheFile(NodeInput *nodeInput, const char *filename);

// /**
// API :: IO_GetMaxLen
// PURPOSE ::  This API is used to get the maximun length of a line
//          in the file.
// PARAMETERS  ::
// +    char* : fileName : Pointer to the name of the file.
// RETURN :: int : Interger with the largest line length.
// **/
int
IO_GetMaxLen(const char *fileName);

// /**
// API :: IO_GetMaxLen
// PURPOSE ::  This API is used to get the maximun length of a line
//                   in the file.
// PARAMETERS  ::
// + FILE* : fp : Pointer to a file stream.
// RETURN :: int : Interger with the largest line length.
// **/
int
IO_GetMaxLen(FILE *fp);

// /**
// API :: NI_GetMaxLen
// PURPOSE ::  This API is used to get the maximun length of a line
//                   in nodeInput.
// PARAMETERS  ::
// + NodeInput* : nodeInput : Pointer to a node input.
// RETURN :: int : Interger with the largest line length.
// **/
int NI_GetMaxLen( NodeInput *nodeInput );

// /**
// API :: NI_GetMaxLen
// PURPOSE ::  This API is used to get the maximun length of a line
//             in nodeInput.
// PARAMETERS  ::
// + const NodeInput* : nodeInput : Pointer to a node input.
// RETURN :: int : Interger with the largest line length.
// **/
int NI_GetMaxLen( const NodeInput *nodeInput );


//-------------------------------------------------------------------------//
//----------------------- OverLoaded --------------------------------------//


// /**
// API :: IO_ParseNetworkAddress
// PURPOSE :: Parses a string for a network address.
//            Overloaded API for ATM compatibility.
// PARAMETERS ::
// + s[]        : const char    : String to parse.
// + u_atmVal   : unsigned int* : Storage for icd, aid, ptp
// RETURN :: void :
// **/

void IO_ParseNetworkAddress(
         const char s[],
         unsigned int* u_atmVal);

// /**
// API          :: IO_ConvertAddrToString
// PURPOSE      :: Convert generic address to appropriate network type
//                 address string format.
// PARAMETERS   ::
// + address    : Address*    : generic address
// + addrStr    : char*       : address string
// RETURN       :: void : NULL
// **/

void IO_ConvertAddrToString(
         Address* address,
         char* addrStr);

// /**
// API          :: IO_ConvertAtmAddressToString
// PURPOSE      :: Convert Atm address to address string format.
// PARAMETERS   ::
// + addr        : AtmAddress    : Atm address
// + addrStr     : char*       : address string
// RETURN       :: void : NULL
// **/

void IO_ConvertAtmAddressToString(AtmAddress addr,
                                  char* addrStr);

// /**
// API          :: IO_InsertIntValue
// PURPOSE      :: Insert integer value for specific string in case of ATM
// PARAMETERS   ::
// + s[]        : const char    : character array
// + val        : const unsigned int : value to be inserted
// + u_atmVal   : unsigned int : atm_value need to be checked
// RETURN       :: void : NULL
// **/
void IO_InsertIntValue(
    const char s[],
    const unsigned int val,
    unsigned int* u_atmVal);


// /**
// API                :: IO_ReadCachedFileIndex
// PURPOSE            :: Return Cached file index for the given parameter name
// PARAMETERS         ::
// + nodeId           :: NodeAddress : node Id
// + interfaceAddress :: NodeAddress : Interface Address for the given node
// + nodeInput        :: unsigned int : atm_value need to be checked
// * parameterName    :: char* : name use to match the file index
// RETURN             :: int :
// **/
int IO_ReadCachedFileIndex(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound);

// /**
// API :: IO_ReadString
// PURPOSE :: This API is used to retrieve a string parameter
//            value from input files.
// PARAMETERS ::
// + node               : Node*             : node structure pointer.
// + nodeId             : const NodeId      : nodeId.
// + interfaceIndex     : int               : interface Index.
// + nodeInput          : const NodeInput*  : Pointer to node input.
// + parameterName      : const char*       : Parameter name.
// + wasFound           : BOOL*             : Storage for success of seach.
// + parameterValue     : char*             : Storage for parameter value.
// RETURN               :: void :
// Note : Overloaded API for reding the value based on interfaceIndex.
// **/
void
IO_ReadString(
    Node* node,
    const NodeAddress nodeId,
    int interfaceIndex,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    char *parameterValue);

void
IO_ReadString(
    Node* node,
    const NodeAddress nodeId,
    int interfaceIndex,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    char *parameterValue,
    BOOL requiredVal);

// /**
// API :: IO_ReadInt64
// PURPOSE :: This API is used to retrieve a Int64 parameter
//            value from input files.
// PARAMETERS ::
// + node               : Node*             : node structure pointer.
// + nodeId             : const NodeId      : nodeId.
// + interfaceIndex     : int               : interface Index.
// + nodeInput          : const NodeInput*  : Pointer to node input.
// + parameterName      : const char*       : Parameter name.
// + wasFound           : BOOL*             : Storage for success of seach.
// + parameterValue     : Int64*            : Storage for parameter value.
// RETURN               : void :
// Note : Overloaded API for reding the value based on interfaceIndex.
// **/
void
IO_ReadInt64(
    Node* node,
    const NodeAddress nodeId,
    int interfaceIndex,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    Int64 *parameterValue);

// /**
// API :: IO_ReadTime
// PURPOSE :: This API is used to retrieve a clocktype parameter
//            value from input files.
// PARAMETERS ::
// + node               : Node*             : node structure pointer.
// + nodeId             : const NodeId      : nodeId.
// + interfaceIndex     : int               : interface Index.
// + nodeInput          : const NodeInput*  : Pointer to node input.
// + parameterName      : const char*       : Parameter name.
// + wasFound           : BOOL*             : Storage for success of seach.
// + parameterValue     : clocktype*        : Storage for parameter value.
// RETURN               : void :
// Note : Overloaded API for reding the value based on interfaceIndex.
// **/
void
IO_ReadTime(
    Node* node,
    const NodeAddress nodeId,
    int interfaceIndex,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    clocktype *parameterValue);

// /**
// API :: IO_ReadInt
// PURPOSE :: This API is used to retrieve a Int parameter
//            value from input files.
// PARAMETERS ::
// + node               : Node*             : node structure pointer.
// + nodeId             : const NodeId      : nodeId.
// + interfaceIndex     : int               : interface Index.
// + nodeInput          : const NodeInput*  : Pointer to node input.
// + parameterName      : const char*       : Parameter name.
// + wasFound           : BOOL*             : Storage for success of seach.
// + parameterValue     : int*              : Storage for parameter value.
// RETURN               : void :
// Note : Overloaded API for reding the value based on interfaceIndex.
// **/
void
IO_ReadInt(
    Node* node,
    const NodeAddress nodeId,
    int interfaceIndex,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    int *parameterValue);

// /**
// API :: IO_ReadDouble
// PURPOSE :: This API is used to retrieve a double parameter
//            value from input files.
// PARAMETERS ::
// + node               : Node*             : node structure pointer.
// + nodeId             : const NodeId      : nodeId.
// + interfaceIndex     : int               : interface Index.
// + nodeInput          : const NodeInput*  : Pointer to node input.
// + parameterName      : const char*       : Parameter name.
// + wasFound           : BOOL*             : Storage for success of seach.
// + parameterValue     : double*           : Storage for parameter value.
// RETURN               : void :
// Note : Overloaded API for reding the value based on interfaceIndex.
// **/
void
IO_ReadDouble(
    Node* node,
    const NodeAddress nodeId,
    int interfaceIndex,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    double *parameterValue);

void
IO_ReadDouble(
    Node* node,
    const NodeAddress nodeId,
    int interfaceIndex,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    double *parameterValue,
    BOOL required,
    BOOL minReq,
    double minVal,
    BOOL maxReq,
    double maxVal);

// /**
// API :: IO_ReadCachedFile
// PURPOSE :: This API is used to retrieve a cached file parameter
//            value from input files.
// PARAMETERS ::
// + node               : Node*             : node structure pointer.
// + nodeId             : const NodeId      : nodeId.
// + interfaceIndex     : int               : interface Index.
// + nodeInput          : const NodeInput*  : Pointer to node input.
// + parameterName      : const char*       : Parameter name.
// + wasFound           : BOOL*             : Storage for success of seach.
// + parameterValue     : NodeInput*        : Storage for parameter value.
// RETURN               : void :
// Note : Overloaded API for reding the value based on interfaceIndex.
// **/
void
IO_ReadCachedFile(
    Node* node,
    const NodeAddress nodeId,
    int interfaceIndex,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    NodeInput *parameterValue);

// /**
// API :: IO_ReadLine
// PURPOSE :: This API is used to retrieve a whole line from input files.
// PARAMETERS ::
// + node               : Node*             : node structure pointer.
// + nodeId             : const NodeId      : nodeId.
// + interfaceIndex     : int               : interface Index.
// + nodeInput          : const NodeInput*  : Pointer to node input.
// + parameterName      : const char*       : Parameter name.
// + wasFound           : BOOL*             : Storage for success of seach.
// + parameterValue     : char*             : Storage for parameter value.
// RETURN               :: void :
// Note : Overloaded API for reding the value based on interfaceIndex.
// **/
void
IO_ReadLine(
    Node* node,
    const NodeAddress nodeId,
    int interfaceIndex,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    char *parameterValue);

// /**
// API :: IO_ReadStringInstance
// PURPOSE :: This API is used to retrieve string parameter values
//            from input files for a specific instance.
// PARAMETERS ::
// + node                      : Node*             : node structure pointer.
// + nodeId                    : const NodeId      : nodeId.
// + interfaceIndex            : int               : interface Index.
// + nodeInput                 : const NodeInput*  : Pointer to node input.
// + parameterName             : const char*       : Parameter name.
// + parameterInstanceNumber   : const int  : Instance number.
// + fallbackIfNoInstanceMatch : const BOOL : Selects parameter without
//                                            instance number if
//                                            parameterInstanceNumber
//                                            cannot be matched.
// + wasFound                  : BOOL*      : Storage for success of seach.
// + parameterValue            : char*      : Storage for parameter value.
// RETURN :: void :
// Note : Overloaded API for reding the value based on interfaceIndex.
// **/
void
IO_ReadStringInstance(
    Node* node,
    const NodeAddress nodeId,
    int interfaceIndex,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    char *parameterValue);

// /**
// API :: IO_ReadDoubleInstance
// PURPOSE :: This API is used to retrieve double parameter values
//            from input files for a specific instance.
// PARAMETERS ::
// + node                      : Node*             : node structure pointer.
// + nodeId                    : const NodeId      : nodeId.
// + interfaceIndex            : int               : interface Index.
// + nodeInput                 : const NodeInput*  : Pointer to node input.
// + parameterName             : const char*       : Parameter name.
// + parameterInstanceNumber   : const int  : Instance number.
// + fallbackIfNoInstanceMatch : const BOOL : Selects parameter without
//                                            instance number if
//                                            parameterInstanceNumber
//                                            cannot be matched.
// + wasFound                  : BOOL*      : Storage for success of seach.
// + parameterValue            : double*      : Storage for parameter value.
// RETURN :: void :
// Note : Overloaded API for reding the value based on interfaceIndex.
// **/
void
IO_ReadDoubleInstance(
    Node* node,
    const NodeAddress nodeId,
    int interfaceIndex,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    double *parameterValue);

// /**
// API :: IO_ReadIntInstance
// PURPOSE :: This API is used to retrieve int parameter values
//            from input files for a specific instance.
// PARAMETERS ::
// + node                      : Node*             : node structure pointer.
// + nodeId                    : const NodeId      : nodeId.
// + interfaceIndex            : int               : interface Index.
// + nodeInput                 : const NodeInput*  : Pointer to node input.
// + parameterName             : const char*       : Parameter name.
// + parameterInstanceNumber   : const int  : Instance number.
// + fallbackIfNoInstanceMatch : const BOOL : Selects parameter without
//                                            instance number if
//                                            parameterInstanceNumber
//                                            cannot be matched.
// + wasFound                  : BOOL*      : Storage for success of seach.
// + parameterValue            : int*      : Storage for parameter value.
// RETURN :: void :
// Note : Overloaded API for reding the value based on interfaceIndex.
// **/
void
IO_ReadIntInstance(
    Node* node,
    const NodeAddress nodeId,
    int interfaceIndex,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    int *parameterValue);

// /**
// API :: IO_ReadTimeInstance
// PURPOSE :: This API is used to retrieve clocktype parameter values
//            from input files for a specific instance.
// PARAMETERS ::
// + node                      : Node*             : node structure pointer.
// + nodeId                    : const NodeId      : nodeId.
// + interfaceIndex            : int               : interface Index.
// + nodeInput                 : const NodeInput*  : Pointer to node input.
// + parameterName             : const char*       : Parameter name.
// + parameterInstanceNumber   : const int  : Instance number.
// + fallbackIfNoInstanceMatch : const BOOL : Selects parameter without
//                                            instance number if
//                                            parameterInstanceNumber
//                                            cannot be matched.
// + wasFound                  : BOOL*      : Storage for success of seach.
// + parameterValue            : clocktype* : Storage for parameter value.
// RETURN :: void :
// Note : Overloaded API for reding the value based on interfaceIndex.
// **/
void
IO_ReadTimeInstance(
    Node* node,
    const NodeAddress nodeId,
    int interfaceIndex,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    clocktype *parameterValue);

// /**
// API :: IO_ReadCachedFileInstance
// PURPOSE :: This API is used to retrieve cached file parameter values
//            from input files for a specific instance.
// PARAMETERS ::
// + node                      : Node*             : node structure pointer.
// + nodeId                    : const NodeId      : nodeId.
// + interfaceIndex            : int               : interface Index.
// + nodeInput                 : const NodeInput*  : Pointer to node input.
// + parameterName             : const char*       : Parameter name.
// + parameterInstanceNumber   : const int  : Instance number.
// + fallbackIfNoInstanceMatch : const BOOL : Selects parameter without
//                                            instance number if
//                                            parameterInstanceNumber
//                                            cannot be matched.
// + wasFound                  : BOOL*      : Storage for success of seach.
// + parameterValue            : NodeInput* : Storage for parameter value.
// RETURN :: void :
// Note : Overloaded API for reding the value based on interfaceIndex.
// **/
void
IO_ReadCachedFileInstance(
    Node* node,
    const NodeAddress nodeId,
    int interfaceIndex,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    NodeInput *parameterValue);

// /**
// API :: IO_ReadStringUsingIpAddress
// PURPOSE :: This API is used to retrieve a string parameter
//            value from input files using the ip-address.
// PARAMETERS ::
// + node               : Node*             : node structure pointer.
// + interfaceIndex     : int               : interface Index.
// + nodeInput          : const NodeInput*  : Pointer to node input.
// + parameterName      : const char*       : Parameter name.
// + wasFound           : BOOL*             : Storage for success of seach.
// + parameterValue     : char*             : Storage for parameter value.
// RETURN               :: void :
// Note : Overloaded API for reding the value based on interfaceIndex.
// **/
void IO_ReadStringUsingIpAddress(
    Node *node,
    int interfaceIndex,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    char *parameterValue);

int
IO_FindStringInFile(const char* compareLine, const char *filename);

// /**
// API :: IO_ReadString
// PURPOSE :: This API is used to retrieve a string parameter
//            value from input files.
// PARAMETERS ::
// + nodeId             : const NodeAddress : nodeId.
// + ipv4Address        : NodeAddress       : IP address of an interface
// + ipv6Address        : in6_addr*         : IPv6 address of an interface
// + nodeInput          : const NodeInput*  : Pointer to node input.
// + parameterName      : const char*       : Parameter name.
// + wasFound           : BOOL*             : Storage for success of seach.
// + parameterValue     : char*             : Storage for parameter value.
// RETURN               :: void
// Note : Overloaded API for reding the value based on interfaceIndex.
// **/
void
IO_ReadString(
    const NodeAddress nodeId,
    NodeAddress ipv4address,
    in6_addr* ipv6Address,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    char *parameterValue);

// /**
// API :: IO_ReadString
// PURPOSE :: This API is used to retrieve a string parameter
//            value from input files.
// PARAMETERS ::
// + nodeId             : const NodeAddress : nodeId.
// + interfaceIndex     : int               : interface index
// + ipv4SubnetAddress  : const NodeAddress : IPv4 subnet address
// + ipv6SubnetAddress  : const in6_addr*   : IPv6 subnet address
// + nodeInput          : const NodeInput*  : Pointer to node input.
// + parameterName      : const char*       : Parameter name.
// + parameterValue     : char*             : Storage for parameter value.
// + wasFound           : BOOL&             : Storage for success of search.
// + matchType          : int&              : Storage for matchType.
// RETURN               :: void
// Note : Overloaded API for reding the value based on interfaceIndex.
// **/
void
IO_ReadString(
    const NodeAddress nodeId,
    int interfaceIndex,
    const NodeAddress ipv4SubnetAddress,
    const in6_addr* ipv6SubnetAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    char *parameterValue,
    BOOL& wasFound,
    int& matchType);
 //---------------------------------------------------------------------------
// FUNCTION             : IO_ReadString
// PURPOSE             :: Reads string value for specified IPv6 address.
// PARAMETERS          :
// +nodeId             :const NodeId nodeId :: NodeId for which parameter has
//                                 has to be searched.
// +address           : const Address* address :: Pointer to address structure.
//                                 address can have ip v4 or v6 address
// +nodeInput         : const NodeInput *nodeInput ::
//                                 pointer to configuration inputs
// +parameterName     : const char *parameterName :: Parameter to be read
// +wasFound          : BOOL *wasFound :: Parameter found or not
// +parameterValue    : char *parameterValue :: Parameter's value if found.
// +matc hType        : int &matchType
// RETURN             : None
// NOTE:
// Overloaded API IO_ReadString()
// -----------------------------------------------------------------------------

void
IO_ReadString(
    const NodeId nodeId,
    const Address* address,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    char *parameterValue,
    int &matchType);

#endif // _FILEIO_H_
