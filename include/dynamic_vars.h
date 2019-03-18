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
// PACKAGE     :: DYNAMIC-VARS
// DESCRIPTION :: This file describes the implementation of the Dynamic API.
// **/

#ifndef _DYNAMIC_VARS_H_
#define _DYNAMIC_VARS_H_

#include <sstream>

#include "dynamic.h"

// Forward declaration
class STAT_Statistic;

// /**
// CLASS       :: D_SimpleObject
// DESCRIPTION ::
// If listening is enabled then every object will inherit from
// D_SimpleObject.  When an object is modified, D_SimpleObject::Changed is
// called which will inform the hierarchy that the object's value has
// changed.
// **/

#ifdef D_LISTENING_ENABLED

class D_SimpleObject
{
    private:
        D_Object* object;

    public:
        D_SimpleObject() : object(NULL)
        {
            // Empty
        }

        void SetObject(D_Object* newObject)
        {
            object = newObject;
        }

        void Changed()
        {
            if (object && object->IsInHierarchy())
            {
                object->Changed();
            }
        }
};
#endif // D_LISTENING_ENABLED

// /**
// CLASS       :: D_Int32
// DESCRIPTION :: Implements a dynamically-enabled 4 byte integer
//
// Variables
//     long value - The value of this integer
//
// Functions
//     virtual BOOL IsNumeric() - Returns TRUE
//
//     virutal double GetDouble() - Return the integer as a double
//
//     virtual void WriteAsString(const char* in) - Set a new value
//         based on string input
//
//     virtual void ReadAsString(char* out) - Return the value as a
//         string
//
//     int& operator = (int newValue) - Overloaded = operator, sets a new
//         value and handles listeners
//
//     int& operator ++ () - Overloaded ++ operator, increments value and
//         handles listeners
//
//     operator int() - Overloaded int cast operator, used for getting value
//
//     operator int() const - Overloaded int cast operator for cases
//         where the int is const.
// **/

#ifdef D_LISTENING_ENABLED
class D_Int32 : public D_SimpleObject
{
    private:
        Int32 value;

    public:
        D_Int32()
        {
            value = 0;
        }

        Int32& operator = (Int32 newValue)
        {
            value = newValue;
            Changed();
            return value;
        }

        Int32& operator ++ ()
        {
            value = value + 1;
            Changed();
            return value;
        }
        Int32& operator ++(Int32)
        {
            value = value + 1;
            Changed();
            return value;
        }
        Int32& operator += (Int32 newValue)
        {
            value+=newValue;
            Changed();
            return value;
        }
        Int32& operator -= (Int32 newValue)
        {
            value-=newValue;
            Changed();
            return value;
        }
        operator Int32()
        {
            return value;
        }
        operator Int32() const
        {
            return value;
        }
};
#else // D_LISTENING_ENABLED
typedef Int32 D_Int32;
#endif // D_LISTENING_ENABLED

class D_Int32Obj : public D_Variable
{
    private:
        D_Int32* value;

    public:
        D_Int32Obj(D_Int32* newValue)
        {
            value = newValue;
#ifdef D_LISTENING_ENABLED
            value->SetObject(this);
#endif // D_LISTENING_ENABLED
        }

        virtual BOOL IsNumeric()
        {
            return TRUE;
        }
        virtual double GetDouble()
        {
            return (double) *value;
        }

        void ReadAsString(std::string& out)
        {
            std::ostringstream oss;
            oss << (Int32) *value;
            out = oss.str();
        }

        void WriteAsString(const std::string& in)
        {
            std::istringstream iss(in);
            Int32 intVal;
            iss >> intVal;
            *value = intVal;
        }
};

// /**
// CLASS       :: D_UInt32
// DESCRIPTION :: Implements a dynamically-enabled 4 byte unsigned integer
//
// Variables
//     unsigned int value - The value of this unsigned integer
//
// Functions
//     virtual BOOL IsNumeric() - Returns TRUE
//
//     virutal double GetDouble() - Return the integer as a double
//
//     virtual void WriteAsString(const char* in) - Set a new value
//         based on string input
//
//     virtual void ReadAsString(char* out) - Return the value as a
//         string
//
//     unsigned int& operator = (unsigned int newValue) - Overloaded =
//         operator, sets a new value and handles listeners
//
//     unsigned int& operator ++ () - Overloaded ++ operator, increments
//         value and handles listeners
//
//     operator unsigned int() - Overloaded int cast operator, used for
//         getting value
//
//     operator unsigned int() const - Overloaded int cast operator for
//         cases where the int is const.
// **/

#ifdef D_LISTENING_ENABLED
class D_UInt32 : public D_SimpleObject
{
    private:
        UInt32 value;

    public:
        D_UInt32()
        {
            value = 0;
        }

        UInt32& operator = (UInt32 newValue)
        {
            value = newValue;
            Changed();
            return value;
        }
        UInt32& operator ++ ()
        {
            value = value + 1;
            Changed();
            return value;
        }
        UInt32& operator ++(int)
        {
            value = value + 1;
            Changed();
            return value;
        }
        UInt32& operator += (UInt32 newValue)
        {
            value += newValue; 
            Changed();
            return value;
        }
        UInt32& operator -= (UInt32 newValue)
        {
            value -= newValue; 
            Changed();
            return value;
        }
        operator UInt32()
        {
            return value;
        }
        operator UInt32() const
        {
            return value;
        }
};
#else // D_LISTENING_ENABLED
typedef UInt32 D_UInt32;
#endif // D_LISTENING_ENABLED

class D_UInt32Obj : public D_Variable
{
    private:
        D_UInt32* value;

    public:
        D_UInt32Obj(D_UInt32* newValue)
        {
            value = newValue;
#ifdef D_LISTENING_ENABLED
            value->SetObject(this);
#endif // D_LISTENING_ENABLED
        }

        virtual BOOL IsNumeric()
        {
            return TRUE;
        }
        virtual double GetDouble()
        {
            return (double) *value;
        }

        void ReadAsString(std::string& out)
        {
            std::ostringstream oss;
            oss << (UInt32) *value;
            out = oss.str();
        }

        void WriteAsString(const std::string& in)
        {
            std::istringstream iss(in);
            UInt32 intVal;
            iss >> intVal;
            *value = intVal;
        }
};

// /**
// CLASS       :: D_Int64
// DESCRIPTION :: Implements a dynamically-enabled 8 byte integer
//
// Variables
//     Int64 value - The value of this integer
//
// Functions
//     virtual BOOL IsNumeric() - Returns TRUE
//
//     virutal double GetDouble() - Return the integer as a double
//
//     virtual void WriteAsString(const char* in) - Set a new value
//         based on string input
//
//     virtual void ReadAsString(char* out) - Return the value as a
//         string
//
//     Int64& operator = (Int64 newValue) - Overloaded = operator, sets a new
//         value and handles listeners
//
//     Int64& operator ++ () - Overloaded ++ operator, increments value and
//         handles listeners
//
//     operator Int64() - Overloaded int cast operator, used for getting value
//
//     operator Int64() const - Overloaded int cast operator for cases
//         where the int is const.
// **/

#ifdef D_LISTENING_ENABLED
class D_Int64 : public D_SimpleObject
{
    private:
        Int64 value;

    public:
        Int64& operator = (Int64 newValue)
        {
            value = newValue;
            Changed();
            return value;
        }

        Int64& operator ++ ()
        {
            value = value + 1;
            Changed();
            return value;
        }
        Int64& operator ++(int)
        {
            value = value + 1;
            Changed();
            return value;
        }
        Int64& operator += (Int64 add)
        {
            value += add;
            Changed();
            return value;
        }
        operator Int64()
        {
            return value;
        }
        operator Int64() const
        {
            return value;
        }
};
#else // D_LISTENING_ENABLED
typedef Int64 D_Int64;
#endif // D_LISTENING_ENABLED

class D_Int64Obj : public D_Variable
{
    private:
        D_Int64* value;

    public:
        D_Int64Obj(D_Int64* newValue)
        {
            value = newValue;
#ifdef D_LISTENING_ENABLED
            value->SetObject(this);
#endif // D_LISTENING_ENABLED
        }

        virtual BOOL IsNumeric()
        {
            return TRUE;
        }
        virtual double GetDouble()
        {
            return (double) *value;
        }

        void ReadAsString(std::string& out)
        {
            std::ostringstream oss;
            oss << (Int64) *value;
            out = oss.str();
        }

        void WriteAsString(const std::string& in)
        {
            std::istringstream iss(in);
            Int64 intVal;
            iss >> intVal;
            *value = intVal;
        }
};


// /**
// CLASS       :: D_Float32
// DESCRIPTION :: Implements a dynamically-enabled 4 byte floating-point
//     number
//
// Variables
//     Float32 value - The value of this float
//
// Functions
//     virtual BOOL IsNumeric() - Returns TRUE
//
//     virutal double GetDouble() - Return the float as a double
//
//     virtual void WriteAsString(const char* in) - Set a new value
//         based on string input
//
//     virtual void ReadAsString(char* out) - Return the value as a
//         string
//
//     Float32& operator = (Float32 newValue) - Overloaded = operator, sets a
//         new value and handles listeners
//
//     operator Float32() - Overloaded float cast operator, used for getting
//         value
//
//     operator Float32() const - Overloaded float cast operator for cases
//         where the float is const.
// **/

#ifdef D_LISTENING_ENABLED
class D_Float32 : public D_SimpleObject
{
    private:
        Float32 value;

    public:
        D_Float32()
        {
            value = 0.0;
        }

        float& operator = (Float32 newValue)
        {
            value = newValue;
            Changed();
            return value;
        }
        float& operator ++ ()
        {
            value=value+1;
            Changed();
            return value;
        }
        float& operator ++ (int)
        {
            value=value+1;
            Changed();
            return value;
        }
        float& operator += (Float32 newValue)
        {
            value+=newValue;
            Changed();
            return value;
        }
        float& operator -= (Float32 newValue)
        {
            value-=newValue;
            Changed();
            return value;
        }
        operator Float32()
        {
            return value;
        }
        operator Float32() const
        {
            return value;
        }
};
#else // D_LISTENING_ENABLED
typedef Float32 D_Float32;
#endif // D_LISTENING_ENABLED

class D_Float32Obj : public D_Variable
{
    private:
        D_Float32* value;

    public:
        D_Float32Obj(D_Float32* newValue)
        {
            value = newValue;
#ifdef D_LISTENING_ENABLED
            value->SetObject(this);
#endif // D_LISTENING_ENABLED
        }

        virtual BOOL IsNumeric()
        {
            return TRUE;
        }
        virtual double GetDouble()
        {
            return (double) *value;
        }

        void ReadAsString(std::string& out)
        {
            std::ostringstream oss;
            oss << (Float32) *value;
            out = oss.str();
        }

        void WriteAsString(const std::string& in)
        {
            std::istringstream iss(in);
            Float32 floatVal;
            iss >> floatVal;
            *value = floatVal;
        }
};

// /**
// CLASS       :: D_Float64
// DESCRIPTION :: Implements a dynamically-enabled 8 byte floating-point
//     number
//
// Variables
//     Float64 value - The value of this double
//
// Functions
//     virtual BOOL IsNumeric() - Returns TRUE
//
//     virutal Float64 GetDouble() - Return the value
//
//     virtual void WriteAsString(const char* in) - Set a new value
//         based on string input
//
//     virtual void ReadAsString(char* out) - Return the value as a
//         string
//
//     Float64& operator = (Float64 newValue) - Overloaded = operator, sets
//         a new value and handles listeners
//
//     Float64& operator += (Float64 newValue) - Overloaded += operator,
//         increments value and handles listeners
//
//     operator Float64() - Overloaded Float64 cast operator, used for
//         getting value
//
//     operator Float64() const - Overloaded Float64 cast operator for cases
//         where the Float64 is const.
// **/

#ifdef D_LISTENING_ENABLED
class D_Float64 : public D_SimpleObject
{
    private:
        Float64 value;

    public:
        D_Float64()
        {
            value = 0.0;
        }

        Float64& operator = (Float64 newValue)
        {
            value = newValue;
            Changed();
            return value;
        }
        Float64& operator += (Float64 newValue)
        {
            value += newValue;
            Changed();
            return value;
        }
        Float64& operator -= (Float64 newValue)
        {
            value -= newValue;
            Changed();
            return value;
        }
        Float64& operator /= (Float64 newValue)
        {
            value /= newValue;
            Changed();
            return value;
        }
        Float64& operator ++ ()
        {
            value=value+1;
            Changed();
            return value;
        }
        Float64& operator ++ (int)
        {
            value=value+1;
            Changed();
            return value;
        }
        operator Float64()
        {
            return value;
        }
        operator Float64() const
        {
            return value;
        }
};
#else // D_LISTENING_ENABLED
typedef Float64 D_Float64;
#endif // D_LISTENING_ENABLED

class D_Float64Obj : public D_Variable
{
    private:
        D_Float64* value;

    public:
        D_Float64Obj(D_Float64* newValue)
        {
            value = newValue;
#ifdef D_LISTENING_ENABLED
            value->SetObject(this);
#endif // D_LISTENING_ENABLED
        }

        virtual BOOL IsNumeric()
        {
            return TRUE;
        }
        virtual double GetDouble()
        {
            return *value;
        }

        void ReadAsString(std::string& out)
        {
            std::ostringstream oss;
            oss << (Float64) *value;
            out = oss.str();
        }

        void WriteAsString(const std::string& in)
        {
            std::istringstream iss(in);
            Float64 floatVal;
            iss >> floatVal;
            *value = floatVal;
        }
};

#ifdef D_LISTENING_ENABLED
class D_Statistic : public D_SimpleObject
{
    private:
        Float64 value;
        STAT_Statistic* statistic;

    public:
        D_Statistic() : value(0.0), statistic(NULL) {}

        void SetStatistic(STAT_Statistic* newStatistic)
        { statistic = newStatistic; }

        Float64& operator = (Float64 newValue)
        {
            value = newValue;
            Changed();
            return value;
        }
        Float64& operator += (Float64 newValue)
        {
            value += newValue;
            Changed();
            return value;
        }
        Float64& operator -= (Float64 newValue)
        {
            value -= newValue;
            Changed();
            return value;
        }
        Float64& operator /= (Float64 newValue)
        {
            value /= newValue;
            Changed();
            return value;
        }
        Float64& operator ++ ()
        {
            value=value+1;
            Changed();
            return value;
        }
        Float64& operator ++ (int)
        {
            value=value+1;
            Changed();
            return value;
        }
        operator Float64()
        {
            return value;
        }
        operator Float64() const
        {
            return value;
        }

        friend class D_StatisticObj;
};
#else // D_LISTENING_ENABLED
typedef Float64 D_Statistic;
#endif // D_LISTENING_ENABLED

class D_StatisticObj : public D_Object
{
    private:
        D_Statistic* value;

    public:
        D_StatisticObj(D_Statistic* newValue) : D_Object(D_STATISTIC)
        {
            value = newValue;
#ifdef D_LISTENING_ENABLED
            value->SetObject(this);
#endif // D_LISTENING_ENABLED
        }

        STAT_Statistic* GetStatistic() { return value->statistic; }

        virtual BOOL IsNumeric()
        {
            return TRUE;
        }
        virtual double GetDouble()
        {
            return *value;
        }

        void ReadAsString(std::string& out)
        {
            std::ostringstream oss;
            oss << (Float64) *value;
            out = oss.str();
        }

        void WriteAsString(const std::string& in)
        {
            std::istringstream iss(in);
            Float64 floatVal;
            iss >> floatVal;
            *value = floatVal;
        }
};

class D_IncrementFloat64Obj : public D_Variable
{
    private: 
        Float64* value;

    public:
        D_IncrementFloat64Obj(Float64* newValue)
        {
            value = newValue;
        }

        virtual BOOL IsNumeric()
        {
            return TRUE;
        }

        virtual double GetDouble()
        {
            return *value;
        }

        void ReadAsString(std::string& out)
        {
            char valString[MAX_STRING_LENGTH];
            sprintf(valString, "%f", (Float64) *value);
            out = valString;
        }

        void WriteAsString(const std::string& in)
        {
            *value = atof(in.c_str());
        }

        Float64* GetValue()
        {
            return value;
        }

        virtual void ExecuteAsString(const std::string& in, std::string& out)
        {
            Float64 byValue = atof(in.c_str());
            *value += byValue;           

            char valString[MAX_STRING_LENGTH];
            sprintf(valString, "%f", (Float64) *value);
            out = valString;
        }
};

// /**
// CLASS       :: D_NodeAddress
// DESCRIPTION :: Implements a dynamically-enabled 4 byte node
//     address
//
// Variables
//     NodeAddress value - The value of this node address
//
// Functions
//     virtual void WriteAsString(const char* in) - Set a new value
//         based on string input
//
//     virtual void ReadAsString(char* out) - Return the value as a
//         string
//
//     operator int() - Overloaded int cast operator, used for getting value
//
//     int& operator = (int newValue) - Overloaded = operator, sets a new
//         value and handles listeners
// **/

#ifdef D_LISTENING_ENABLED
class D_NodeAddress : public D_SimpleObject
{
    private:
        NodeAddress value;

    public:
        D_NodeAddress()
        {
            value = 0;
        }

        NodeAddress& operator = (NodeAddress newValue)
        {
            value = newValue;
            Changed();
            return value;
        }
        operator NodeAddress() { return value; }
        operator NodeAddress() const { return value; }
};
#else // D_LISTENING_ENABLED
typedef NodeAddress D_NodeAddress;
#endif // D_LISTENING_ENABLED

class D_NodeAddressObj : public D_Variable
{
    private:
        D_NodeAddress* value;

    public:
        D_NodeAddressObj(D_NodeAddress* newValue)
        {
            value = newValue;
#ifdef D_LISTENING_ENABLED
            value->SetObject(this);
#endif // D_LISTENING_ENABLED
        }

        D_NodeAddress* GetValue()
        {
            return value;
        } 

        void ReadAsString(std::string& out)
        {   
            char outStr[MAX_STRING_LENGTH];
            sprintf(outStr, "%u.%u.%u.%u",
                (*value & 0xff000000) >> 24,
                (*value & 0xff0000) >> 16,
                (*value & 0xff00) >> 8,
                *value & 0xff);
            out = outStr;
        }

        void WriteAsString(const std::string& in)
        {
            // TODO: support a real address
            std::istringstream iss(in);
            UInt32 intVal;
            iss >> intVal;
            *value = intVal;
        }
};

// /**
// CLASS       :: D_String
// DESCRIPTION :: Implements a dynamically-enabled string
//
// Variables
//     char value[MAX_STRING_LENGTH] - The value of this string
//
// Functions
//     virtual void WriteAsString(const char* in) - Set a new value
//         based on string input
//
//     virtual void ReadAsString(char* out) - Return the value as a
//         string
//
//     operator char*() - Overloaded char* cast operator, used for getting
//         value
// **/

#ifdef D_LISTENING_ENABLED
class D_String : public D_SimpleObject
{
    private:
        std::string value;

    public:
        D_String() { value = ""; }

        // A string may only be accessed as a const char*.  This prohibits
        // changing the string's value directly.  Set must be called
        // instead.

        operator const char*() const
        {
            return (const char*) value.c_str();
        }
        operator const std::string&() const
        {
            return value;
        }
        void Set(const std::string& newValue)
        {
            value = newValue;
            Changed();
        }
        void Set(const char* newValue)
        {
            value = newValue;
        }
};
#else // D_LISTENING_ENABLED
class D_String
{
    private:
        std::string value;

    public:
        D_String()
        {
            value = "";
        }

        // A string may only be accessed as a const char*.  This prohibits
        // changing the string's value directly.  Set must be called
        // instead.

        operator const char*() const
        {
            return (const char*) value.c_str();
        }
        operator const std::string&() const
        {
            return value;
        }
        void Set(const std::string& newValue)
        {
            value = newValue;
        }
        void Set(const char* newValue)
        {
            value = newValue;
        }
};
#endif // D_LISTENING_ENABLED

class D_StringObj : public D_Variable
{
    private:
        D_String* value;

    public:
        D_StringObj(D_String* newValue)
        {
            value = newValue;
#ifdef D_LISTENING_ENABLED
            value->SetObject(this);
#endif // D_LISTENING_ENABLED
        }

        virtual void WriteAsString(const std::string& in);
        virtual void ReadAsString(std::string& out);

        D_String* GetValue()
        {
            return (D_String*)value;
        }
};

// /**
// CLASS       :: D_Clocktype
// DESCRIPTION :: Implements a dynamically-enabled clocktype
//
// Variables
//     clocktype value - The value of this clocktype
//
// Functions
//     virtual BOOL IsNumeric() - Returns TRUE
//
//     virutal double GetDouble() - Return the value as a double
//
//     virtual void WriteAsString(const char* in) - Set a new value
//         based on string input
//
//     virtual void ReadAsString(char* out) - Return the value as a
//         string
//
//     operator clocktype() - Overloaded clocktype cast operator, used for
//         getting value
//
//     clocktype& operator = (clocktype newValue) - Overloaded = operator,
//         used for setting value
//
//     operator clocktype*() - Get a pointer to the clocktype
// **/

#ifdef D_LISTENING_ENABLED
class D_Clocktype : public D_SimpleObject
{
    private:
        clocktype value;

    public:
        D_Clocktype()
        {
            value = 0;
        }

        operator clocktype()
        {
            return value;
        }
        operator clocktype() const
        {
            return value;
        }
        clocktype& operator = (clocktype newValue)
        {
            value = newValue;
            Changed();
            return value;
        }
        clocktype& operator += (clocktype add)
        {
            value += add;
            Changed();
            return value;
        }
        clocktype& operator -= (clocktype add)
        {
            value -= add;
            Changed();
            return value;
        }
};
#else // D_LISTENING_ENABLED
typedef clocktype D_Clocktype;
#endif // D_LISTENING_ENABLED

class D_ClocktypeObj : public D_Variable
{
    private:
        D_Clocktype* value;

    public:
        D_ClocktypeObj(D_Clocktype* newValue)
        {
            value = newValue;
#ifdef D_LISTENING_ENABLED
            value->SetObject(this);
#endif // D_LISTENING_ENABLED
        }

        virtual BOOL IsNumeric()
        {
            return TRUE;
        }
        virtual double GetDouble()
        {
            return (double) *value;
        }

        virtual void ReadAsString(std::string& out)
        {
            std::ostringstream oss;
            oss << (clocktype) *value;
            out = oss.str();
        }

        virtual void WriteAsString(const std::string& in)
        {
            *value = TIME_ConvertToClock((char*) in.c_str());
        }

        D_Clocktype* GetValue()
        {
            return value;
        }
};

//
// class D_BOOL Implements a dynamically-enabled 4 byte integer
//
// Variables
//    int value  The value of this integer
//
// Functions
//     virtual BOOL IsNumeric() - Returns TRUE
//
//     virutal double GetDouble() - Return the integer as a double
//
//     virtual void WriteAsString(const char* in) - Set a new value
//         based on string input
//
//     virtual void ReadAsString(char* out) - Return the value as a
//         string
//
//     unsigned BOOL& operator = (unsigned int newValue) Overloaded =
//         operator, sets a new value and handles listeners
//
//     unsigned BOOL& operator ++ () - Overloaded ++ operator, increments
//         value and handles listeners
//
//     operator BOOL() Overloaded int cast operator, used for
//         getting value
//
//     operator BOOL() const - Overloaded int cast operator for
//         cases where the int is const.
//
//     operator bool operator == () - Overloaded comparision operator for 
//         compare the value with BOOL type
//
//     operator bool operator != () - Overloaded inequality operator for
//         compare the value with BOOL type

#ifdef D_LISTENING_ENABLED

class D_BOOL : public D_SimpleObject
{
 private:
    BOOL value;

 public:
    D_BOOL()
    {
        value = FALSE;
    }    
    BOOL& operator = (BOOL newValue)
    {
        value = newValue;
        Changed();
        return value;    
    }    
    BOOL& operator ++ ()
    {
        value = value + 1;
        //Changed();
        return value;
    }
    BOOL& operator += (BOOL newValue)
    {
        value+=newValue;
        //Changed();
        return value;
    }
    operator BOOL()
    {
        return value;
    }
    operator BOOL() const
    {
        return value;
    }
    
    bool operator == (BOOL compValue)
    {
        if (value == compValue) {
            return true;
        } else {            
            return false;
        }
        
    }
    bool operator != (BOOL compValue)
    {
        if (value == compValue) {
            return false;
        } else {
            return true;
        }
    }    
};
#else // D_LISTENING_ENABLED
typedef BOOL D_BOOL;
#endif // D_LISTENING_ENABLED



class D_BOOLObj : public D_Variable
{
 private:
    D_BOOL* value;
    
 public:
    D_BOOLObj(D_BOOL* newValue)
    {
        value = newValue;
#ifdef D_LISTENING_ENABLED
        value->SetObject(this);
#endif // D_LISTENING_ENABLED
    }
    
    virtual BOOL IsNumeric()
    {
        return TRUE;
    }

    virtual double GetDouble()
    {
        return (double) *value;
    }
    
    void ReadAsString(std::string& out)
    {
        char temp[MAX_STRING_LENGTH];
        sprintf(temp, "%d", (BOOL) *value);
        out = temp;
    }
    
    void WriteAsString(const std::string& in)
    {
        if (in == "YES" || in == "1")
            *value = TRUE;
        else if (in == "NO" || in == "0")
            *value = FALSE;        
    }
    
    D_BOOL* GetValue()
    {
        return value;
    }
};

//
// class D_UInt64 - Implements a dynamically-enabled 8 byte integer
//
// Variables
//     UInt64 value - The value of this integer
//
// Functions
//     virtual BOOL IsNumeric() - Returns TRUE
//
//     virutal double GetDouble() - Return the integer as a double
//
//     virtual void WriteAsString(const char* in) - Set a new value
//         based on string input
//
//     virtual void ReadAsString(char* out) - Return the value as a
//         string
//
//     UInt64& operator = (UInt64 newValue) - Overloaded = operator, sets a new
//         value and handles listeners
//
//     UInt64& operator ++ () - Overloaded ++ operator, increments value and
//         handles listeners
//
//     operator UInt64() - Overloaded int cast operator, used for getting value
//
//     operator UInt64() const - Overloaded int cast operator for cases
//         where the int is const.
//
 
#ifdef D_LISTENING_ENABLED
class D_UInt64 : public D_SimpleObject
{
    private:
        UInt64 value;

    public:
        UInt64& operator = (UInt64 newValue)
        {
            value = newValue;
            Changed();
            return value;
        }

        UInt64& operator ++ ()
        {
            value = value + 1;
            Changed();
            return value;
        }
        UInt64& operator ++ (int)
        {
            value = value + 1;
            Changed();
            return value;
        }
        UInt64& operator += (UInt64 add)
        {
            value += add;
            Changed();
            return value;
        }
        UInt64& operator -= (UInt64 add)
        {
            value -= add;
            Changed();
            return value;
        }
        operator UInt64()
        {
            return value;
        } 
        operator UInt64() const
        {
            return value;
        } 
};
#else // D_LISTENING_ENABLED
typedef UInt64 D_UInt64;
#endif // D_LISTENING_ENABLED

class D_UInt64Obj : public D_Variable
{
    private:
        D_UInt64* value;

    public:
        D_UInt64Obj(D_UInt64* newValue)
        {
            value = newValue;
#ifdef D_LISTENING_ENABLED
            value->SetObject(this);
#endif // D_LISTENING_ENABLED
        }

        virtual BOOL IsNumeric()
        {
            return TRUE;
        }
        virtual double GetDouble()
        {
            return (double) *value;
        }

        void ReadAsString(std::string& out)
        {
            char temp[MAX_STRING_LENGTH];
            sprintf(temp, "%d", (int) *value);
            out = temp;
        }

        void WriteAsString(std::string& in)
        {
            *value = atoi(in.c_str());
        }

        D_UInt64* GetValue()
        {
            return value;
        }
};

// Callback functions added to increment or update various types 
class updateFloatCallback : public D_ListenerCallback
{
    private:   
        D_Float64* total;
        double currVal;            
        double lastVal;

    public:
        updateFloatCallback(D_Float64* var)
        {
            total = var;
            lastVal = 0;
            currVal = 0;
        }

        virtual void operator () (const std::string& newValue)
        {   
            currVal = atof(newValue.c_str());
            (*total) += (currVal - lastVal);
            lastVal = currVal;            
        }
};

// Callback functions added to increment or update various types 
class updateFloatStringCallback : public D_ListenerCallback
{
    private:   
        D_Hierarchy *hierarchy;
        std::string path;
        Float64 total;
        Float64 currVal;            
        Float64 lastVal;

    public:
        updateFloatStringCallback(const std::string& p, D_Hierarchy *h)
        {
            hierarchy = h;            
            path = p;
            total = 0;
            lastVal = 0;
            currVal = 0;
        }

        virtual void operator () (const std::string& newValue)
        {   
            currVal = atof(newValue.c_str());            
            total = (currVal - lastVal);
            lastVal = currVal;
            std::string currGlobalVal;
            hierarchy->ReadAsString(path, currGlobalVal);
            total += atof(currGlobalVal.c_str());
            
            std::stringstream newVal;
            newVal << total;            
            hierarchy->WriteAsString(path, newVal.str());

        }
};


class incrementFloatCallback : public D_ListenerCallback
{
    private:   
        D_Float64* total;        

    public:
        incrementFloatCallback(D_Float64* newVal)
        {
            total = newVal;            
        }

        virtual void operator () (const std::string& newValue)
        {   
            (*total) += 1;
        }
};

class incrementFloatStringCallback : public D_ListenerCallback
{
    private:   
        D_Hierarchy *hierarchy;
        std::string path;

    public:
        incrementFloatStringCallback(const std::string& p, D_Hierarchy *h)
        {
            hierarchy = h;            
            path = p;            
        }

        virtual void operator () (const std::string& newValue)
        {               
            Float64 total;
            std::string currGlobalVal;
            hierarchy->ReadAsString(path, currGlobalVal);
            total = atof(currGlobalVal.c_str()) + 1;
            
            std::stringstream newVal;
            newVal << total;            
            hierarchy->WriteAsString(path, newVal.str());
        }
};

// Call back function for channel listenable mask
class listenableChannelCallBack : public D_ListenerCallback
{
    public:
        listenableChannelCallBack()
        {          
        }
        
        virtual void operator () (const std::string& newValue)
        {
          //printf("listenableChannelCallBack\n");
        }
};

// Call back function for channel listening mask
class listeningChannelCallBack : public D_ListenerCallback
{
    public:
        listeningChannelCallBack()
        {          
        }
        
        virtual void operator () (const std::string& newValue)
        {
          //printf("listeningChannelCallBack\n");
        }
};

// Call back function for data rate
class dataRateCallBack : public D_ListenerCallback
{
    public:
        dataRateCallBack()
        {          
        }
        
        virtual void operator () (const std::string& newValue)
        {
          //printf("dataRateCallBack\n");
        }
};


// Call back function for bandwidth
class bandwidthCallBack : public D_ListenerCallback
{
    public:
        bandwidthCallBack()
        {          
        }
        
        virtual void operator () (const std::string& newValue)
        {
          //printf("bandwidthCallBack\n");
        }
};


#endif // _DYNAMIC_VARS_H_ 
