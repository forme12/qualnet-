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
// PACKAGE     :: ERROR
// DESCRIPTION :: This file defines data structures and functions used in error-handling.
// **/


#ifndef QUALNET_ERROR_H
#define QUALNET_ERROR_H

#include <stdarg.h>
#include <stdio.h>

#include "types.h"  // for BOOL
#include "main.h"   // for MAX_STRING_LENGTH  and _WIN32's snprintf()

// /**
// CONSTANT    :: ERROR_ASSERTION : 0
// DESCRIPTION :: Defines the ERROR_ASSERTION constant
// **/
#define ERROR_ASSERTION 0

// /**
// CONSTANT    :: ERROR_ERROR : 1
// DESCRIPTION :: Defines the ERROR_ERROR constant
// **/
#define ERROR_ERROR     1

// /**
// CONSTANT    :: ERROR_WARNING : 2
// DESCRIPTION :: Defines the ERROR_WARNING constant
// **/
#define ERROR_WARNING   2

extern char* ERROR_Filename;
extern char* ERROR_Condition;
extern int   ERROR_LineNumber;
extern int   ERROR_Type;

#ifdef NDEBUG
#define ERROR_Assert(expr, str)
#define assert(expr)
#else

// /**
// MACRO       :: ERROR_Assert(expr, str)
// DESCRIPTION :: May be used in place of assert,to include an error message
// **/
#define ERROR_Assert(expr, str)  ((expr) || ERROR_WriteError(ERROR_ASSERTION, # expr, str, __FILE__, __LINE__))

#define ERROR_AssertArgs(expr, ...) \
    {\
        if (! (expr))\
        {\
            char txt[MAX_STRING_LENGTH];\
            int  txtLen;\
            txtLen = snprintf(txt,\
                              MAX_STRING_LENGTH,\
                              __VA_ARGS__);\
            if (MAX_STRING_LENGTH  == txtLen)\
            {\
                (void)ERROR_ReportWarning("Assert message truncated");\
            }\
            (void)ERROR_WriteError(ERROR_ASSERTION, # expr, txt, __FILE__, __LINE__);\
        }\
    }

#ifdef assert
#undef assert
#endif // assert

// /**
// MACRO       :: assert(expr)
// DESCRIPTION :: In DEBUG mode assert macro will be replaced by
//  ERROR_WriteError with ERROR_ASSERTION type
// **/
#define assert(expr)  ((expr) || ERROR_WriteError(ERROR_ASSERTION, # expr, NULL, __FILE__, __LINE__))
#endif // NDEBUG

// /**
// MACRO       :: ERROR_ReportError(str)
// DESCRIPTION :: Function call used to report an error condition in QualNet,
//                and notify GUI of such.
// **/
#define ERROR_ReportError(str)  ERROR_WriteError(ERROR_ERROR, NULL, str, __FILE__, __LINE__)

#define ERROR_ReportErrorArgs(...) \
    {\
        char txt[MAX_STRING_LENGTH];\
        int  txtLen;\
        txtLen = snprintf(txt,\
                          MAX_STRING_LENGTH,\
                          __VA_ARGS__);\
        (void)ERROR_ReportError(txt);\
        if (MAX_STRING_LENGTH  == txtLen)\
        {\
            (void)ERROR_ReportWarning("Error message truncated");\
        }\
    }

// /**
// MACRO       :: ERROR_ReportWarning(str)
// DESCRIPTION ::
//    Function call used to report a recoverable error condition.
//    This macro in turns calls ERROR_WriteError with ERROR_WARNING type. It
//    reports a warning message in QualNet, and notify GUI of such
// **/
#define ERROR_ReportWarning(str)  ERROR_WriteError(ERROR_WARNING, NULL, str, __FILE__, __LINE__)

#define ERROR_ReportWarningArgs(...) \
    {\
        char txt[MAX_STRING_LENGTH];\
        int  txtLen;\
        txtLen = snprintf(txt,\
                          MAX_STRING_LENGTH,\
                          __VA_ARGS__);\
        (void)ERROR_ReportWarning(txt);\
        if (MAX_STRING_LENGTH  == txtLen)\
        {\
            (void)ERROR_ReportWarning("Warning message truncated");\
        }\
    }

#define DEBUG_PRINT(fmt, ...) \
    do { if (DEBUG) fprintf(stdout, fmt, __VA_ARGS__); } while (0)

// /**
// API         :: ERROR_WriteError
// PURPOSE     ::
//  Function call used to report failed assertions, errors,
//  and warnings, and notify the GUI of such.  The user should not call this
//  function directly, but should use one of the previously defined macros.
// PARAMETERS  ::
// + type      : int   : assertion, error, or warning
// + condition : char* : a string representing the failed boolean condition
// + msg       : char* : an error message
// + file      : char* : the file name in which the assertion failed
// + lineno    : int   : the line on which the assertion failed.
// RETURN      :: extern BOOL :
// **/
extern BOOL ERROR_WriteError(int   type,
                             const char* condition,
                             const char* msg,
                             const char* file,
                             int   lineno);

typedef void (*QErrorHandler)(int type, const char * errorMessage);


// /**
// API         :: ERROR_InstallHandler
// PURPOSE     ::
//  Function used to register a callback function. The callback function
//  will be invoked by ERROR_ when ERROR_WriteError () is invoked.
//  For example - logging error messages into a log file or send the error
//  messages to another application (e.g. to the Qualnet IDE that started
//  the simulation.)
// PARAMETERS  ::
// + type      : int   : assertion, error, or warning
// + condition : char* : a string representing the failed boolean condition
// + msg       : char* : an error message
// + file      : char* : the file name in which the assertion failed
// + lineno    : int   : the line on which the assertion failed.
// + functionPointer    : QErrorHandler  : pointer to a function with signature
//                                       : that takes the error type and the
//                                       : error message string.
// RETURN      :: void :
// **/
extern void ERROR_InstallHandler(QErrorHandler functionPointer);

// /**
// API        :: ERROR_ReportMissingAddon
// PURPOSE    :: Reports an error when user attempts to use a model that
//               hasn't been installed, either because the customer hasn't
//               purchased that feature, or they haven't downloaded and
//               compiled it.
// PARAMETERS ::
// + model : const char* : the name of the model/protocol being used.
// + addon : const char* : the name of the addon to which the model belongs
// RETURN  :: void :
static void ERROR_ReportMissingAddon(const char* model,
                                     const char* addon) {
    ERROR_ReportErrorArgs("The %s model requires the %s addon", model, addon);
}

// /**
// API        :: ERROR_ReportMissingInterface
// PURPOSE    :: Reports an error when user attempts to use a model that
//               hasn't been installed, either because the customer hasn't
//               purchased that feature, or they haven't downloaded and
//               compiled it.
// PARAMETERS ::
// + model : const char* : the name of the model/protocol being used.
// + iface : const char* : the name of the interface to which the model belongs
// RETURN  :: void :
static void ERROR_ReportMissingInterface(const char* model,
                                         const char* iface) {
    ERROR_ReportErrorArgs("The %s model requires the %s interface", model, iface);
}

// /**
// API        :: ERROR_ReportMissingLibrary
// PURPOSE    :: Reports an error when user attempts to use a model that
//               hasn't been installed, either because the customer hasn't
//               purchased that feature, or they haven't downloaded and
//               compiled it.
// PARAMETERS ::
// + model   : const char* : the name of the model/protocol being used.
// + library : const char* : the name of the library to which the model belongs
// RETURN  :: void :
static void ERROR_ReportMissingLibrary(const char* model,
                                       const char* library) {
    ERROR_ReportErrorArgs("The %s model requires the %s library", model, library);
}

#endif

