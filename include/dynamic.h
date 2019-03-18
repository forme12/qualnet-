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
// PACKAGE     :: DYNAMIC
// DESCRIPTION :: This file describes the implementation of the Dynamic API.
// **/

#ifndef _DYNAMIC_H_
#define _DYNAMIC_H_

#include <string>
#include <vector>
#include <map>
#include "clock.h"
#include "qualnet_error.h"
//
// #define D_LISTENING_ENABLED - Defines whether listening is
//     enabled or disabled.  This should not be changed by the user unless
//     they know what they are doing.  If enabled:
//         - All D_Objects have AddListener() and RemoveListener() functions
//         - All variables become class wrappers for their underlying data
//           type and inherit from SimpleObject.  Simple object provides a
//           Changed() function which will notify the hierarchy when the
//           object changes.
//         - The hierarchy has AddListener() and RemoveListeners() member
//           functions
//     If disabled:
//         - All objects are typedef'd to their underlying data type.  For
//           example, D_Int32 is typedef'd to a 4 byte integer.
//

#define D_LISTENING_ENABLED

// /**
// CONSTANT     :: D_INITIAL_LEVELS 10000
// DESCRIPTION  :: Define the initial number of levels that are
//     allocated by the hierarchy.  These are allocated when the first level
//     is created.
// **/


// Forward declarations
class D_Object;
class D_Variable;
class D_Hierarchy;
class D_Level;

// /**
// ENUM        :: D_Type
// DESCRIPTION :: Enumeration listing the different types of dynamic objects
// **/

enum D_Type
{
    D_VARIABLE,
    D_STATISTIC,
    D_COMMAND
};

// /**
// ENUM        :: D_ActionType
// DESCRIPTION :: Enumeration listing the different types of dynamic actions
//                used for parallel
// **/

enum D_ActionType
{
    D_CreatePath = 0,
    D_CreateObject = 1,
    D_AddLink = 2,
    D_IsObject = 3,
    D_IsReadable = 4,
    D_IsWriteable = 5,
    D_IsExecutable = 6,
    D_ReadAsString = 7,
    D_WriteAsString = 8,
    D_ExecuteAsString = 9,
    D_Listen = 10,
    D_ListenCallback = 11,
    D_RemoveListener = 12,
    D_ExceptionAction = 13,
};

#define MULTIPLE_PARTITIONS -2
#define NO_PARTITION -1

//
// typedef int D_HierarchyId - Specifies the type of the D_HierarchyId which
//     is used to identify every level in the hierarchy.
//

typedef int D_HierarchyId;

// /**
// CLASS       :: class D_Exception
// DESCRIPTION :: Base class for all Dynamic API exceptions
//
// Variables
//     error - Error message for this string
//
// Public functions
//     void GetFullErrorString(char* str) - Return a fully formatted
//         error message
//
//     char* GetError() - Return only the error condition
// **/

class D_Exception
{
    protected:
        std::string error;

    public:
        D_Exception()
        {
            error = "";
        }
        D_Exception(std::string& err)
        {
            error = err;
        }
        D_Exception(char* err)
        {
            error = err;
        }

        void GetFullErrorString(std::string& str);
        std::string& GetError()
        {
            return error;
        }
};

// /**
// CLASS       :: D_ExceptionNotEnabled
// DESCRIPTION :: Thrown when the user attempts to access the dynamic
//                hierarchy and it is not enabled
// **/

class D_ExceptionNotEnabled : public D_Exception
{
    public:
        D_ExceptionNotEnabled()
        {
            error = "the dyanmic hierarchy is not enabled";
        }
};

// /**
// CLASS       :: D_ExceptionPathExists
// DESCRIPTION :: Thrown when the user attempts to create a
//     level that already exists
// **/

class D_ExceptionPathExists : public D_Exception
{
    public:
        D_ExceptionPathExists(const std::string& path)
        {
            error = "path \"" + path + "\" already exists";
        }
};

// /**
// CLASS       :: D_ExceptionInvalidId
// DESCRIPTION :: Thrown when the user supplies an invalid id
//     to a Dynamic API function
// **/

class D_ExceptionInvalidId : public D_Exception
{
    public:
        D_ExceptionInvalidId(D_HierarchyId id)
        {
            char intStr[MAX_STRING_LENGTH];

            sprintf(intStr, "%d", id);
            error = std::string("invalid id \"") + intStr + "\"";
        }
};

// /**
// CLASS       :: D_ExceptionInvalidPath
// DESCRIPTION :: Thrown when the user supplies an invalid
//     path to a Dynamic API function
// **/

class D_ExceptionInvalidPath : public D_Exception
{
    public:
        D_ExceptionInvalidPath(const std::string& path)
        {
            error = "invalid path \"" + path + "\"";
        }
};

// /**
// CLASS       :: D_ExceptionInvalidChild
// DESCRIPTION :: Thrown when the user tries to get an
//     invalid child of a parent
// **/

class D_ExceptionInvalidChild : public D_Exception
{
    public:
        D_ExceptionInvalidChild(const std::string& path)
        {
            error = std::string("invalid child for \"" + path + "\"");
        }
};

// /**
// CLASS       :: D_ExceptionNotObject
// DESCRIPTION :: Thrown when the user attempts to
//     read/write/execute a level that does not contain an object
// **/

class D_ExceptionNotObject : public D_Exception
{
    public:
        D_ExceptionNotObject(const std::string& path)
        {
            error = "the path \"" + path + "\" is not an object";
        }
};

// /**
// CLASS       :: D_ExceptionIsObject
// DESCRIPTION :: Thrown when the user attempts to do an
//     operation on a level where the level is an object, but should be a
//     normal level
// **/

class D_ExceptionIsObject : public D_Exception
{
    public:
        D_ExceptionIsObject(const std::string& path)
        {
            error = "the path \"" + path + "\" is an object";
        }
};

// /**
// CLASS       :: D_ExceptionNotImplemented
// DESCRIPTION :: Thrown when the user tries to use an
//     object in a way that is not implemnted.  For instance, trying to
//     execute a variable
// **/

class D_ExceptionNotImplemented : public D_Exception
{
    public:
        D_ExceptionNotImplemented(const std::string& path, const std::string& action)
        {
            error = "the object \"" + path + "\" does not implement " + action;
        }
};

// /**
// CLASS       :: D_ExceptionNotReadable
// DESCRIPTION :: Thrown when the user tries to write to a
//     variable that is not readable
// **/

class D_ExceptionNotReadable : public D_Exception
{
    public:
        D_ExceptionNotReadable(const std::string& path)
        {
            error = "the variable \"" + path + "\" is not readable";
        }
};

// /**
// CLASS       :: D_ExceptionNotWriteable
// DESCRIPTION :: Thrown when the user tries to write to a
//     variable that is not writeable
// **/

class D_ExceptionNotWriteable : public D_Exception
{
    public:
        D_ExceptionNotWriteable(const std::string& path)
        {
            error = "the variable \"" + path + "\" is not writeable";
        }
};

// /**
// CLASS       :: D_ExceptionNotInHierarchy
// DESCRIPTION :: Thrown when the user tries to act on an
//     object that should be in the hierarchy but is not
// **/

class D_ExceptionNotInHierarchy : public D_Exception
{
    public:
        D_ExceptionNotInHierarchy()
        {
            error = "object not in hierarchy";
        }
};

// /**
// CLASS       :: D_ExceptionIsInHierarchy
// DESCRIPTION :: Thrown when the user tries to add an
//     object to the hierarchy twice
// **/

class D_ExceptionIsInHierarchy : public D_Exception
{
    public:
        D_ExceptionIsInHierarchy(const std::string& path, const std::string& oldPath)
        {
            error = "object \"" + path + "\" is in the hierarchy at \"" + oldPath + "\"";
        }
};

// /**
// CLASS       :: D_ExceptionNoObject
// DESCRIPTION :: Thrown when an object doesn't exist, but should
// **/

class D_ExceptionNoObject : public D_Exception
{
    public:
        D_ExceptionNoObject(const std::string& where)
        {
            error = "there is no object for " + where;
        }
};

// /**
// CLASS       :: D_ExceptionNoParent
// DESCRIPTION :: Thrown when a level's parent doesn't exist
// **/

class D_ExceptionNoParent : public D_Exception
{
    public:
        D_ExceptionNoParent(const std::string& path)
        {
            error = "the path \"" + path + "\" has no parent";
        }
};

// /**
// CLASS       :: D_ExceptionUnknownListener
// DESCRIPTION :: Thrown when an unknown listener is created
// **/

class D_ExceptionUnknownListener : public D_Exception
{
    public:
        D_ExceptionUnknownListener(const std::string& listener)
        {
            error = "\"" + listener + "\" is an unknown listener type";
        }
};

// /**
// CLASS       :: D_ExceptionRemotePartition
// DESCRIPTION :: Thrown when the user attempts to
//     access something on a different partition.  Most likely through the
//     GetObject function
// **/

class D_ExceptionRemotePartition : public D_Exception
{
    public:
        D_ExceptionRemotePartition(const std::string& path)
        {
            error = "the path \"" + path + "\" is on a remote partition";
        }
};

// /**
// CLASS       :: D_Level
// DESCRIPTION :: Implements a level in the hierarchy.  All objects and
//     links are stored ss levels.
//
// Variables
//     name - The name of this level.  The last token in the fullPath.
//         Memory is allocated by SetFullPath().
//
//     fullPath - The path to this level.  Memory is allocated by
//         SetFullPath().
//
//     description - The description of this level.  The description is used
//         only for human readability.  Memory is allocated by
//         SetDescription().
//
//     id - The id of this level.  Assigned automatically by the hierarchy.
//
//     parent - This level's parent, NULL if the root level
//
//     object - An optional object (variable, statistic, or command).  NULL
//         if not present.  Each level may have only one object.
//
//     link - An optional symbolic link to a different level.  NULL if not
//         present.
//
//     children - Array of this level's children, used for hierarchy
//         maintenance.
//
//     linksToThislevel - Array of level's that link to this level, used for
//         heirarchy maintenance.
//
// Private Functions
//     void SetId(D_HierarchyId newId) - Sets the id of this level.  Should
//         only be called by the hierarchy.
//
//     void SetParent(D_Level* newParent) - Sets the parent of this
//         level
//
// Public Functions
//     D_Level() - Constructor
//
//     ~D_Level() - Deconstructor
//
//     char* GetFullPath() - Returns the full path of this level
//
//     char* GetName() - Returns the name of this level (the last token in
//         the path)
//
//     void SetFullPath(const char* newName, const char* newPath) - Set a
//         new path for this level
//
//     char* GetDescription() - Get the level's description.  Empty string
//         if none.
//
//     BOOL HasDescription() - Return true if this level has a description
//
//     void SetDescription(const char* newDescription) - Set the level's
//         description
//
//     void AddChild(D_Level* child) - Add a child level
//
//     void RemoveChild(D_Level* child) - Remove a child level
//
//     BOOL HasChild(const char* name) - Returns TRUE if the level has a
//         child of the given name
//
//     D_Level* GetChild(const char* name) - Returns the child of the given
//         name.  Throws D_ExceptionInvalidPath if not found.
//
//     D_HierarchyId GetId()- Returns the id of this level
//
//     D_Level* GetParent()- Returns the parent of this level, throws
//          D_ExceptionNoParent if there is no parent
//
//     BOOL HasParent() - Returns true if this level has a parent
//
//     BOOL IsRoot() - Returns true if this level is the root
//
//     void SetObject(D_Object* newObject) - Set an object for this
//         level.  Throws an exception if this level already has a link or
//         an object.
//
//     BOOL IsObject() - Return true if this level is an object
//
//     D_Object* GetObject() - Return this level's object if it exists.
//         NULL if it doesn't exist.
//
//     void RemoveObject() - Remove the object from this level.  Throws an
//         exception if the level does not have an object.
//
//     void SetLink(D_Level* newLink) - Set a link for this level.
//         This level must not already have a link or object.
//
//     void AddLinkToThisLevel(D_Level* link) - Add the level "link" to the
//         array of levels that link to this level
//
//     void RemoveLinkToThisLevel(D_Level* link) - Remove a the level "link"
//         from the list of levels that link to this level
//
//     BOOL IsLinkedTo() - Return TRUE if a different level links to this
//         level
//
//     D_Level* GetLink() - Return the level this level links to.  Returns
//         NULL if not a link.
//
//     D_Level* ResolveLinks() - Resolve all links for this level.  Returns
//         the level that this level links to.  Handles multiple links.
//
//     BOOL IsLink() - Return TRUE if this level is a link
// **/

class D_Level
{
    private:
        std::string name;
        std::string description;
        D_HierarchyId id;
        D_Level* parent;
        D_Object* object;
        D_Level* link;

        std::vector<D_Level*> children;
        std::vector<D_Level*> linksToThisLevel;

        // If the level is "node" or "interface" the hash is used
        BOOL useHash;
        int hashSize;
        std::vector<D_Level*>* childrenHash;

#ifdef PARALLEL
        // which partition owns this level.
        // Value is either MULTIPLE_PARTITIONS, NO_PARTITION or >=0 (one partition)
        int m_Partition;
#endif // PARALLEL

        void SetId(D_HierarchyId newId)
        {
            id = newId;
        }

        void SetParent(D_Level* newParent)
        {
            parent = newParent;
        }

        UInt32 ChildrenHash(const std::string& name);

    protected:
        D_Object* GetObject();

    public:
        D_Level();
        ~D_Level();

        std::string GetFullPath();
        std::string& GetName()
        {
            return name;
        }
        void SetFullPath(const std::string& newName, const std::string& newPath);

        std::string& GetDescription()
        {
            return description;
        }
        BOOL HasDescription()
        {
            return description.size() > 0;
        }
        void SetDescription(const std::string& newDescription);

        void AddChild(D_Level* child);
        void RemoveChild(D_Level* child);
        BOOL HasChild(const std::string& name);
        D_Level* GetChild(const std::string& name);

        D_HierarchyId GetId()
        {
            return id;
        }

        D_Level* GetParent();
        BOOL HasParent()
        {
            return parent != NULL;
        }
        BOOL IsRoot()
        {
            return parent == NULL;
        }

        void SetObject(D_Object* newObject);
        BOOL IsObject()
        {
            return object != NULL;
        }
        void RemoveObject();

        void SetLink(D_Level* newLink);
        void AddLinkToThisLevel(D_Level* link);
        void RemoveLinkToThisLevel(D_Level* link);
        BOOL IsLinkedTo()
        {
            return linksToThisLevel.size() > 0;
        }
        D_Level* GetLink()
        {
            return link;
        }
        D_Level* ResolveLinks();
        BOOL IsLink()
        {
            return link != NULL;
        }

#ifdef PARALLEL
        void AddPartition(int partition);
        int GetPartition() { return m_Partition; }
#endif

    friend class D_Hierarchy;
    friend class D_ObjectIterator;
};

class D_Listener;

// /**
// CLASS       :: D_ListenerCallback
// DESCRIPTION :: The callback function for a listener.  This
//     class must be over-ridden to be used as a callback.  The subclass may
//     also add additional functions and member variables.
//
// Functions:
//     virtual void operator () (D_Listener* listener) = 0 - Callback
//         function.  The user must over-ride this function.
// **/

class D_ListenerCallback
{
    private:
#ifdef PARALLEL
        int callbackId;
#endif

    public:
        D_ListenerCallback()
#ifdef PARALLEL
            : callbackId(-1)
#endif
        {}
        virtual void operator () (const std::string& newValue) = 0;

#ifdef PARALLEL
        void SetCallbackId(int newCallbackId) { callbackId = newCallbackId; }
        int GetCallbackId() { return callbackId; }
#endif
};

#ifdef PARALLEL
class D_ParallelCallback : public D_ListenerCallback
{
    private:
        PartitionData* m_Partition;
        int m_RemoteCallback;
        int m_RemotePartition;

    public:
        D_ParallelCallback(PartitionData* partition, int remoteCallback, int remotePartition) : m_Partition(partition), m_RemoteCallback(remoteCallback), m_RemotePartition(remotePartition) {}

        virtual void operator () (const std::string& newValue);

        int GetRemoteCallbackId() { return m_RemoteCallback; }
};
#endif

// /**
// CLASS       :: D_Listener
// DESCRIPTION :: Listener base class.  Each listener will override this
//     class, optionally adding additional member variables and functions
//
// Variables
//     variable - The variable this listener is listening to
//
//     callback - Called when the listener condition has been met
//
//     callbackId - Used in parallel qualnet to match a listener and a
//         callback function that are on different partitions
//
//     tag - Identifies who added this listener
//
// Functions
//     D_Listener(D_ListenerCallback* newCallback) - Constructor,
//         initializes the listener with the given callback function and no
//         variable.  SetVariable() must be called for this listener to be
//         valid.
//
//     ~D_Listener() - Destructor, frees the callback class
//
//     D_Variable* GetVariable() - Return the listener's variable.  Throws
//         D_ExceptionNoObject if the listener doesn't have a variable.
//
//     virtual void Changed() - Called when the variable changes.  Calls the
//         listener's function if certain conditions have been met.  If not
//         over-ridden, always calls the function.
// **/

class D_Listener
{
    protected:
        D_Variable* variable;

    private:
        D_ListenerCallback* callback;
#ifdef PARALLEL
        int callbackId;
        const std::string tag;
#endif

    public:
        D_Listener(
            D_Variable* newVariable,
            const std::string& newTag,
            D_ListenerCallback* newCallback) :
            variable(newVariable), callback(newCallback)
#ifdef PARALLEL
            , callbackId(-1),
            tag(newTag)
#endif
            {}
        ~D_Listener();

        D_Variable* GetVariable();

#ifdef PARALLEL
        const std::string& GetTag() { return tag; }
#endif

        virtual void Changed();

        void CallCallback();
#ifdef PARALLEL
        void SetCallbackId(int newCallbackId) { callbackId = newCallbackId; }
        int GetCallbackId() { return callbackId; }
        int GetRemoteCallbackId() { return ((D_ParallelCallback*) callback)->GetRemoteCallbackId(); }
#endif
};

// /**
// CLASS       :: D_ListenerPercent
// DESCRIPTION :: Listener that invokes the callback when the
//     value has changed by a certain percentage.  This listener is
//     implemented using templates and requires that the underlying variable
//     is numeric.  Examples would be D_Int32, D_Float64, D_Clocktype, and
//     so on.
//
// Variables
//     lastVal - The last value that invoked the callback, 0 if the callback
//         was never invoked
//
//     percent - The percentage of change in the value required to invoke
//         the callback.  Typically set by the user.
//
// Functions
//     D_ListenerPercent(
//         double newPercent,
//         D_ListenerCallback* newCallback) - The constructor.  Pass it the
//             percent the variable needs to change before the callback is
//             called, and the callback.
//
//     Changed() - The virtual changed function.  Invokes the callback if
//         the listener's object has changed by percent.
// **/

class D_ListenerPercent : public D_Listener
{
    private:
        double lastVal;
        double percent;
    public:
        D_ListenerPercent(
            D_Variable* newVariable,
            const std::string& arguments,
            const std::string& tag,
            D_ListenerCallback* newCallback);

        virtual void Changed();
};

// /**
// CLASS       :: D_Object
// DESCRIPTION :: Implements an object.  This is a base class for
//     variables, statistics and commands.
//
// Variables
//     type - The type of object this is
//
//     level - The level in the hierarchy at which this object resides.
//         NULL if this object is not in the hierarchy.
//
//     BOOL readable - TRUE if this object is externally readable
//
//     BOOL writeable - TRUE if this object is externally writeable
//
//     BOOL executable - TRUE if this object is externally executable
//
//     listeners - A dynamically allocated array containing the listening
//         functions.  These functions will be called each time the object
//         changes.  Only exists if D_LISTENING_ENABLED is defined.
//
// Functions
//     D_Object(D_Type newType) - Constructor, initializes object
//
//     ~D_Object() - Destructor
//
//     D_Type GetType() - Returns the object type
//
//     D_HierarchyId GetId() - Get the ID of this object.  This is
//        technically the ID of the level this object resides in.  Throws
//        D_ExceptionNotInHierarchy if the object is not in the hierarchy.
//
//     char* GetFullPath() - Get the full path of this object.  This is
//         technically the path of the level this object resides in.  Throws
//         D_ExceptionNotInHierarchy if the object is not in the hierarchy.
//
//     BOOL IsInHierarchy() - Returns TRUE if the object is in the hierarchy
//
//     void SetLevel(D_Level* newLevel) - Set the object's level.  This may
//         only be called if the object is not already in the hierarchy.
//         Throws D_ExceptionIsInHierarchy if already in the hierarchy.
//
//     void RemoveFromHierarchy(D_Hierarchy* hierarchy) - Remove the object
//         and the level it resides in from the hierarchy.  Throws
//         D_ExceptionNotInHierarchy if the object is not in the hierarchy.
//
//     void RemoveFromLevel() - Remove this object from the level it is in.
//         This does not remove the object's level from the hierarchy.
//         Throws D_ExceptionNotInHierarchy if the object is not in the
//         hierarchy.
//
//     void SetReadable(BOOL newReadable) - Set the externaly readable
//         status for this object
//
//     virtual BOOL IsReadable() - Return TRUE if this object is readable
//
//     BOOL SetWriteable(BOOL newWriteable) - Set the externally writeable
//         status for this object
//
//     virtual BOOL IsWriteable() - Return TRUE if this object is writeable
//
//     void SetExecutable(BOOL nExecutable) - Set the externaly executable
//         status for this object
//
//     virtual BOOL IsExecutable() - Return TRUE if this object is
//         executable
//
//     virtual void ReadAsString(char* out) - Read the object's value.  If
//         readable this function must be over-ridden.
//
//     virtual void WriteAsString(const char* in) - Write to the
//         object's value.  If writeable this function must be over-ridden.
//
//     virtual void ExecuteAsString(const char* in, char* out) -
//         Execute the object.  If executable this function must be
//         over-ridden.
//
//     void AddListener(D_Listener* newListener) - Add a new listener to
//         this level.  This listener will be activated each time the object
//         changes.
//
//     void RemoveListener(D_Listener* listener) - Remove a listener from
//         this object, and frees its memory.
//
//     void Changed() - This function must be called each time the
//         object changes.  This will call all of the listening functions.
//         Calling this function is the responsibility of the developer who
//         creates a new data type.  Typically this is called from the
//         D_SimpleObject class.
// **/

class D_Object
{
    protected:
        D_Type type;
        D_Level* level;
        BOOL readable;
        BOOL writeable;
        BOOL executable;

#ifdef D_LISTENING_ENABLED
        std::vector<D_Listener*> listeners;
#endif // D_LISTENING_ENABLED

    public:
        D_Object(D_Type newType);
        ~D_Object();

        D_Type GetType()
        {
            return type;
        }

        D_HierarchyId GetId();
        std::string GetFullPath();

        BOOL IsInHierarchy() { return level != NULL; }
        void SetLevel(D_Level* newLevel);
        void RemoveFromHierarchy(D_Hierarchy* hierarchy);
        void RemoveFromLevel();

        void SetReadable(BOOL newReadable)
        {
            readable = newReadable;
        }
        BOOL IsReadable()
        {
            return readable;
        }
        void SetWriteable(BOOL newWriteable)
        {
            writeable = newWriteable;
        }
        BOOL IsWriteable()
        {
            return writeable;
        }
        void SetExecutable(BOOL newExecutable)
        {
            executable = newExecutable;
        }
        BOOL IsExecutable()
        {
            return executable;
        }

        virtual void ReadAsString(std::string& out);
        virtual void WriteAsString(const std::string& in);
        virtual void ExecuteAsString(const std::string& in, std::string& out);

#ifdef D_LISTENING_ENABLED
        void AddListener(D_Listener* newListener);
        void RemoveListener(D_Listener* listener);
        void Changed()
        {
            unsigned int i = 0;

            // First verify that this object is in the hierarchy.  This
            // function should not be called if the object is not in the
            // hierarchy.

            ERROR_Assert(
                IsInHierarchy(),
                "Change called while not in hierarchy");

            // Loop through all listeners, update changed status
            for (i = 0; i < listeners.size(); i++)
            {
                listeners[i]->Changed();
            }
        }
#endif // D_LISTENING_ENABLED

        friend class D_Hierarchy;
};

// /**
// CLASS       :: D_Variable
// DESCRIPTION :: Implements the variable base class
//
// Functions
//     D_Variable() - Constructor, initialize object as D_VARIABLE
//
//     D_Variable(D_Type newType) - Constructor, initialize object as custom
//         type
//
//     virtual BOOL IsNumeric() - Returns TRUE if the object is a numeric
//         type.  By default returns FALSE.  Must be over-ridden for numeric
//         data types.
//
//     virtual double GetDouble() - Return the variable as a double.  If the
//         variable is not a numeric type an exception is thrown.  Must be
//         over-ridden for numeric data types.
// **/

class D_Variable : public D_Object
{
    public:
        D_Variable();
        D_Variable(D_Type newType);

        virtual BOOL IsNumeric()
        {
            return FALSE;
        }
        virtual double GetDouble();
};

// /**
// CLASS       :: D_Command
// DESCRIPTION :: A command.  The user must implement the ExecuteAsString
//     function.
// **/

class D_Command : public D_Object
{
    public:
        D_Command() : D_Object(D_COMMAND)
        {
            executable = TRUE;
        }
};

// /**
// CLASS       :: D_Hierarchy
// DESCRIPTION :: Implements the dynamic hierarchy
//
// Variables
//     levels - A dynamic array containing allocated levels
//
//     D_Level* root - The root level in the hierarchy
//
//     D_HierarchyId nextId - The id for the next added level
//
//     BOOL enabled,
//     BOOL partitionEnabled,
//     BOOL nodeEnabled,
//     BOOL phyEnabled,
//     BOOL macEnabled,
//     BOOL networkEnabled,
//     BOOL transportEnabled,
//     BOOL appEnabled - Which layers are dynamically enabled.  The enabled
//         variable takes precedence over all others.
//
// Private functions
//
//     void ReadConfigFile(NodeInput* nodeInput) - Read the dynamic
//         parameters from the config file.  This will set the enabled
//         characteristics of the hierarchy.
//
//     D_Level* CreateLevel(
//         D_Level* parentLevel,
//         const char* levelName) - Create a new level below the given
//             parent level.  Throws an exception if there is already a
//             child with the given path.
//
//     D_Level* GetNextLevel(
//         D_Level* parentLevel,
//         const char* nextLevel,
//         BOOL resolveLinks = TRUE) - Get the next level after the parent
//             level.  nextLevel is the level name of the child.  If
//             resolveLinks is TRUE then links for the next level will be
//             resolved.
//
//     D_Level* GetLevelRecursive(
//         D_Level* parentLevel,
//         const char* fullPath,
//         const char* remainingPath,
//         BOOL resolveLinks = TRUE) - Get a level by recursively descending
//             the path.  If resolveLinks is TRUE then any links will be
//             resolved.  Throws an exception if the path cannot be
//             resolved.
//
//     void PrintRecursive(D_Level* level) - Print the hierarchy recursively
//         starting at the given level
//
// Public functions - Functions used for interacting with the hierarchy.
//     All functions throw D_ExceptionInvalidPath if their path parameter is
//     invalid, unless noted otherwise.
//
//     D_Hierarchy() - Constructor, initializes hierarchy with 0 allocated
//         levels
//
//     ~D_Hierarchy() - Destructor
//
//     BOOL IsEnabled() - Return TRUE if the dynamic api is enabled
//
//     void Initialize(
//         PartitionData* partition,
//         int initialNumLevels) - Initialize the hierarchy.  Reads the
//             config file, allocates levels.
//
//     BOOL CreatePartitionPath(
//         PartitionData* partition,
//         const char* name,
//         char* path) - Create a path for a dynamic object at the partition
//             level.  Returns TRUE if the path was successfully created.
//             Returns FALSE if the path was not created (already exists,
//             partition level not enabled).
//
//     BOOL CreateNodePath(
//         Node* node,
//         const char* name,
//         char* path) - Create a path for a dynamic object at the node
//             level.  Returns TRUE if the path was successfully created.
//             Returns FALSE if the path was not created (already exists,
//             node level not enabled).
//
//     BOOL CreateExternalInterfacePath(
//         const char* interfaceName,
//         const char* name,
//         char* path) - Create a path for a dynamic object at the external
//             interfacelevel.  Returns TRUE if the path was successfully
//             created.  Returns FALSE if the path was not created (already
//             exists).
//
//     BOOL CreateApplicationPath(
//         Node* node,
//         const char* app,
//         int port,
//         const char* name,
//         char* path) - Create a path for a dynamic object at the
//             application level.  Returns TRUE if the path was successfully
//             created.  Returns FALSE if the path was not created (already
//             exists, application level not enabled).
//
//     BOOL CreateTransportPath(
//         Node* node,
//         const char* protocol,
//         const char* name,
//         char* path) - Create a path for a dynamic object at the
//             transport level.  Returns TRUE if the path was successfully
//             created.  Returns FALSE if the path was not created (already
//             exists, transport level not enabled).
//
//     BOOL CreateNetworkPath(
//         Node* node,
//         const char* protocol,
//         const char* name,
//         char* path) - Create a path for a dynamic object at the
//             network level.  Returns TRUE if the path was successfully
//             created.  Returns FALSE if the path was not created (already
//             exists, network level not enabled).
//
//     BOOL CreateRoutingPath(
//         Node* node,
//         int phyIndex,
//         const char* protocol,
//         const char* name,
//         char* path) - Create a path for a dynamic object at the
//             routing level.  Returns TRUE if the path was successfully
//             created.  Returns FALSE if the path was not created (already
//             exists, routing level not enabled).
//
//     BOOL CreateMacPath(
//         Node* node,
//         int phyIndex,
//         const char* protocol,
//         const char* name,
//         char* path) - Create a path for a dynamic object at the mac
//             level.  Returns TRUE if the path was successfully created.
//             Returns FALSE if the path was not created (already exists,
//             mac level not enabled).
//
//     BOOL CreatePhyPath(
//         Node* node,
//         int phyIndex,
//         const char* protocol,
//         const char* name,
//         char* path) - Create a path for a dynamic object at the phy
//             level.  Returns TRUE if the path was successfully created.
//             Returns FALSE if the path was not created (already exists,
//             phy level not enabled).
//
//     D_Level* AddLevel(const char* path) - Add a new level to the
//         hierarchy.  All levels in the path will be created as necessary.
//         Throws D_ExceptionPathExists if the path already exists.
//
//     D_Level* AddObject(const char* path, const D_Object* object)- Add
//        a level as before, but containing an object.  Throws
//        D_ExceptionPathExists if the path already exists.
//
//     D_Level* AddLink(const char* path, const char* link) - Add a
//         level as before, but make it a link to a different level.  Throws
//         D_ExceptionPathExists if the path already exists.
//
//     void RemoveLevel(D_Level* level) - Remove a level from the
//         hierarchy.  This function works recursively, removing all
//         children of this level and all levels that link to this level.
//         Also un-links any levels that link to this level.  If this level
//         contains an object, updates the object .
//
//     void RemoveLevelByPath(const char* path) - Remove the level with the
//         given path
//
//     BOOL IsValidPath(const char* path) - Returns TRUE if the given path
//         is valid.  Does not throw D_ExceptionInvalidPath if path is not
//         valid.
//
//     int GetNumLevels() - Return the number of levels in the hierarchy
//
//     D_Level* GetLevelById(
//         D_HierarchyId id,
//         BOOL resolveLinks = TRUE) - Get a level based on its id.  If
//             resolveLinks is TRUE then any links will be resolved.  Throws
//             an exception if the id does not exist.
//
//     D_Level* GetLevelByPath(
//         const char* path,
//         BOOL resolveLinks = TRUE) - Get a level based on its path.  If
//             resolveLinks is TRUE then any links will be resolved.  Throws
//             an exception if the path does not exist.
//
//     void Print() - Prints the hierarchy to stdout
//
//     int GetNumChildren(const char* path) - Returns the number of children
//         of a path.  Throws an exception if the path is not valid.
//
//     D_Level* GetChild(D_Level* level, int child) - Return a pointer to
//         a child, where 0 <= child < GetNumChildren(path).  Throws
//         D_ExceptionInvalidChild if the child index is not valid.
//
//     char* GetChildName(const char* path, int child) - Returns the name of
//         a child, where 0 <= child < GetNumChildren(path).  The full path
//         to the child would be "path/GetChild(path, child)".  Throws
//         D_ExceptionInvalidChild if the child index is not valid.
//
//     BOOL IsObject(const char* path) - Returns true if the given path is
//        an object.  Throws anh exception if the path is not valid.
//
//     D_Object* GetObject(const std::string& path) - Return the object at the
//         given path.  Throws  D_ExceptionNotObject if the path does not
//         resolve to an object.  Throws D_ExceptionRemotePartition if the
//         object is on a different partition.
//
//     void SetReadable(const char* path, BOOL newReadable) - Set the
//         object at the following path to be readable.  Throws an exception
//         if the path is not valid.
//
//     BOOL IsReadable(const char* path) - Check if the object at the
//         following path is readable.  Throws an exception if the path is
//         not valid.
//
//     void SetWriteable(const char* path, BOOL newWriteable) - Set the
//         object at the following path to be writeable.  Throws an
//         exception if the path is not valid.
//
//     BOOL IsWriteable(const char* path) - Check if the object at the
//         following path is writeable.  Throws an exception if the path
//         is not valid.
//
//     void SetExecutable(const char* path, BOOL newExecutable) - Set the
//         object at the following path to be executable.  Throws an
//         exception if the path is not valid.
//
//     BOOL IsExecutable(const char* path) - Check if the object at the
//         following path is executable.  Throws an exception if the path
//         is not valid.
//
//     void ReadAsString(
//         const char* path,
//         char* out) - Read the value of the object with the given path.
//              Throws an exception if the path does not exist or if the
//              path does not resolve to an object.
//
//     void WriteAsString(
//         const char* path,
//         const char* in) - Write the value to the object with the given
//             path.  Throws an exception if the path does not exist or if
//             the path does not resolve to an object.
//
//     void ExecuteAsString(
//         const char* path,
//         const char* in,
//         char* out) - Execute the object with the given path.   Throws an
//              exception if the path does not exist or if the path does not
//              resolve to an object.
//
//     void AddListener(
//          const char* path,
//          D_Listener* listener) - Add a new listener to the object
//              with the given path.  Throws D_ExceptionNotObject if the
//              path does not resolve to an object.  Only present if
//              D_LISTENING_ENABLED is defined.
//
//     void RemoveListeners(const char* path) - Remove all listeners from
//         an object and free their memory.  Only present if
//         D_LISTENING_ENABLED is defined.
// **/

class D_Hierarchy
{
    private:
        PartitionData* m_Partition;

        std::vector<D_Level*> levels;
        D_Level* root;
        D_HierarchyId nextId;

        BOOL enabled;
        BOOL partitionEnabled;
        BOOL nodeEnabled;
        BOOL phyEnabled;
        BOOL macEnabled;
        BOOL networkEnabled;
        BOOL transportEnabled;
        BOOL appEnabled;

#ifdef PARALLEL
        int nextCallbackId;
        std::map<int, D_ListenerCallback*> callbackMap;
#endif

#ifdef D_LISTENING_ENABLED
        std::vector<D_ListenerCallback*> objectListeners;
        std::vector<D_ListenerCallback*> objectPermissionsListeners;
        std::vector<D_ListenerCallback*> linkListeners;
        std::vector<D_ListenerCallback*> removeListeners;
#endif

        void ReadConfigFile(NodeInput* nodeInput);

        D_Level* CreateLevel(
            D_Level* parentLevel,
            const std::string& levelName
#ifdef PARALLEL
            , int partition
#endif // PARALLEL
            );

        D_Level* GetNextLevel(
            D_Level* parentLevel,
            const std::string& nextLevel,
            BOOL resolveLinks = TRUE);

        D_Level* GetLevelRecursive(
            D_Level* parentLevel,
            const std::string& fullPath,
            const std::string& remainingPath,
            BOOL resolveLinks = TRUE);

        void PrintRecursive(D_Level* level);

    protected:
        D_Object* GetObject(const std::string& path);
        D_Level* GetLevelById(
            D_HierarchyId id,
            BOOL resolveLinks = TRUE);
        D_Level* GetLevelByPath(
            const std::string& path,
            BOOL resolveLinks = TRUE);

    public:
        D_Hierarchy();
        ~D_Hierarchy();

        BOOL IsEnabled()
        {
            return enabled;
        }

        void Initialize(
            PartitionData* partition,
            int initialNumLevels);

        BOOL CreatePartitionPath(
            PartitionData* partition,
            const char* name,
            std::string& path);
        BOOL CreateNodePath(
            Node* node,
            const char* name,
            std::string& path);
        BOOL CreateNodeInterfacePath(
            Node* node,
            int interfaceIndex,
            const char* name,
            std::string& path);
        BOOL CreateNodeInterfaceIndexPath(
            Node* node,
            int interfaceIndex,
            const char* name,
            std::string& path);
        BOOL BuildNodeInterfaceIndexPath(
            Node* node,
            int interfaceIndex,
            const char* name,
            std::string& path);
        BOOL CreateNodeInterfaceChannelPath(
            Node* node,
            int interfaceIndex,
            int channelIndex,
            const char* name,
            char* path);
        BOOL CreateExternalInterfacePath(
            const char* interfaceName,
            const char* name,
            std::string& path);
        void BuildApplicationServerPathString(
            NodeId nodeId,
            const std::string& app,
            NodeId sourceNodeId,
            int port,
            const std::string& name,
            std::string& path);
        void BuildApplicationPathString(
            NodeId nodeId,
            const std::string& app,
            int port,
            const std::string& name,
            std::string& path);
        BOOL CreateApplicationPath(
            Node* node,
            const char* app,
            int port,
            const char* name,
            std::string& path);
        BOOL CreateTransportPath(
            Node* node,
            const char* protocol,
            const char* name,
            std::string& path);
        BOOL CreateNetworkPath(
            Node* node,
            const char* protocol,
            const char* name,
            std::string& path);
        BOOL CreateRoutingPath(
            Node* node,
            int phyIndex,
            const char* protocol,
            const char* name,
            std::string& path);
        BOOL CreateMacPath(
            Node* node,
            int phyIndex,
            const char* protocol,
            const char* name,
            std::string& path);
        BOOL CreatePhyPath(
            Node* node,
            int phyIndex,
            const char* protocol,
            const char* name,
            std::string& path);
        BOOL CreateNetworkPath(
            Node* node,
            int interfaceIndex,
            const char* protocol,
            const char* name,
            std::string& path);
        BOOL CreateNetworkInterfacePath(
            Node* node,
            int phyIndex,
            const char* protocol,
            const char* name,
            std::string& path,
            BOOL isParameter = FALSE);
        BOOL CreatePhyInterfacePath(
            Node* node,
            int phyIndex,
            const char* protocol,
            const char* name,
            std::string& path,
            BOOL isParameter = FALSE);
        BOOL CreateQueuePath(
            Node* node,
            int interfaceIndex,
            int queueNum,
            const char* name,
            std::string& path);
        BOOL CreateGlobalDescriptorStatPath(
            const char* statName,
            const char* appName,
            char* path);
        BOOL CreateGlobalResourceUtilizationStatPath(
            const char* statName,
            char* path);
        BOOL CreateGlobalControlTrafficStatPath(
            const char* statName,
            char* path);
        BOOL CreateGlobalDescriptorMOPPath(
            const char* statName,
            const char* appName,
            char* path);
        BOOL CreateGlobalRadioMOPPath(
            const char* statName,
            char* path);
        BOOL CreateGlobalRadioStatPath(
            const char* statName,
            char* path);
        BOOL CreateGlobalNetworkConnectivityStatPath(
            const char* statName,
            char* path);
        BOOL CreateGlobalNetworkConnectivityMOPPath(
            const char* statName,
            char* path);
        BOOL CreateGlobalResourceUtilizationMOPPath(
            const char* statName,
            char* path);
        BOOL CreateSubnetStatsPath(
            int subnetId,
            const char* statName,
            char* path);
        BOOL CreateNodeMobilityPath(
            Node* nodeId,
            const char* name,
            std::string& path);
        BOOL CreatePropagationPath(
            const char* name,
            int i,
            std::string& path);
        BOOL CreateNodeMopPath(
            int nodeId,
            const char* name,
            std::string& path);
        BOOL CreateNodeMibsPath(
            Node* node,
            char* mibsId,
            char* name,
            char* path);
        BOOL CreateNodeInterfaceIndexPath(
            Node* node,
            int interfaceIndex,
            char* name,
            std::string& path);
        void CreatePath(const std::string& path);
        D_Level* AddLevel(
            const std::string& path,
#ifdef PARALLEL
            int partition,
#endif // PARALLEL
            const std::string& newDescription = "");
        D_Level* AddObject(
            const std::string& path,
            D_Object* object,
            const std::string& newDescription = "");
        D_Level* AddLink(
            const std::string& path,
            const std::string& link,
            const std::string& newDescription = "");
        void RemoveLevel(const std::string& path);
        BOOL IsValidPath(const std::string& path);

        int GetNumLevels()
        {
            return (int)levels.size();
        }

        void Print();

        int GetNumChildren(const std::string& path);
        D_Level* GetChild(const std::string& path, int index);
        std::string GetChildName(const std::string& path, int index);
        std::string GetChildName(const std::string& path, std::string& child);
        BOOL HasParent(const std::string& path);
        void GetParent(std::string& path);
        void ResolveLinks(std::string& path);

        BOOL IsObject(const std::string& path);

        void SetReadable(const std::string& path, BOOL newReadable);
        BOOL IsReadable(const std::string& path);

        void SetWriteable(const std::string& path, BOOL newWriteable);
        BOOL IsWriteable(const std::string& path);

        void SetExecutable(const std::string& path, BOOL newExecutable);
        BOOL IsExecutable(const std::string& path);

        std::string EncodePath(const std::string& path);
        std::string EncodeString(const std::string& str);
        void ReadAsString(
            const std::string& path,
            std::string& out);
        void WriteAsString(
            const std::string& path,
            const std::string& in);
        void ExecuteAsString(
            const std::string& path,
            const std::string& in,
            std::string& out);

#ifdef D_LISTENING_ENABLED
        void AddListener(
            const std::string& path,
            const std::string& listenerType,
            const std::string& arguments,
            const std::string& tag,
            D_ListenerCallback* callback);
        void AddObjectListener(D_ListenerCallback* callback);
        void AddObjectPermissionsListener(D_ListenerCallback* callback);
        void AddLinkListener(D_ListenerCallback* callback);
        void AddRemoveListener(D_ListenerCallback* callback);
        void RemoveListeners(
            const std::string& path,
            const std::string& tag);
#endif // D_LISTENING_ENABLED

#ifdef PARALLEL
        int GetNewCallbackId();

        // The following functions are defined in parallel.cpp
        void ParallelCreateLevel(const std::string& path);
        void ParallelAddObject(const std::string& path);
        void ParallelAddLink(const std::string& path, const std::string& link);
        BOOL ParallelIsObject(int partition, const std::string& path);
        BOOL ParallelIsReadable(int partition, const std::string& path);
        BOOL ParallelIsWriteable(int partition, const std::string& path);
        BOOL ParallelIsExecutable(int partition, const std::string& path);
        void ParallelRead(int partition, const std::string& path, std::string& out);
        void ParallelWrite(int partition, const std::string& path, const std::string& in);
        void ParallelExecute(int partition, const std::string& path, const std::string& in, std::string& out);
#ifdef D_LISTENING_ENABLED
        void ParallelListen(int partition, const std::string& path, const std::string& listenerType, const std::string& arguments, const std::string& tag, int callbackId);
        void ParallelRemoveListener(int partition, const std::string& path, const std::string& tag);
#endif
        void ProcessRemoteMessage(Message* msg);

        BOOL IsPathLocal(std::string& path);
        BOOL IsLevelLocal(D_Level* level);
#endif // PARALLEL

    friend class D_ObjectIterator;
};

// /**
// CLASS       :: class D_ObjectIterator
// DESCRIPTION :: Base class for all Dynamic API exceptions
//
// Public functions
//
//     D_ObjectIterator(D_Hierarchy *h, std::string& path) - Initialize the
//         iterator.  Example paths: "node/*/hostname", "node/1/ip/*", 
//         "stats/*", "stats/totalPacketsSent"
//
//     void SetType(D_Type type) - Matches only objects of the specified
//         type
//
//     void SetRecursive(bool recursive) - When iterating recursively, every
//         sub-object of each matching path will be returned.  Example:
//         "/node/1/" will return all dynamic objects of node 1.
//         "/node/1/*" by comparison will only return objects one level
//         beneath "node/1" such as "node/1/hostname" but not
//         "node/1/ip/print".
//
//     bool GetNext() - Prepares the next object for the GetObject()
//         function call.  Returns true if an object is available, false
//         if not.  Must be called before the first call to GetObject().
//
//     D_Object* GetObject() - Returns the next matching object.  NULL if
//         there is no matching object.
// **/

class D_ObjectIterator
{
    private:
        bool m_Recursive;
        bool m_InRecursive;
        int m_RecursiveTop;
        std::vector<int> m_RecursiveStack;

        bool m_UseType;
        D_Type m_Type;
        int m_Level;
        int m_WildcardLevel;
        D_Level* m_CurrentLevel;
        std::vector<std::string> m_Levels;
        std::vector<int> m_WildcardStack;
        D_Hierarchy* m_Hierarchy;

        bool GetNextRecursive();

    public:
        D_ObjectIterator(D_Hierarchy *h, std::string& path);
        void SetType(D_Type type);
        void SetRecursive(bool recursive) { m_Recursive = recursive; }

        bool GetNext();
        D_Object* GetObject();
};

// dynamic_vars.h is included here because it relies on the contents of
// dynamic.h.  dynamic_vars.h will never be included directly.

#include "dynamic_vars.h"

#endif // _DYNAMIC_H_
