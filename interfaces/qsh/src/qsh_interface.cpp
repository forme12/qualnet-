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
// DESCRIPTION :: Implements the Dynamic API
// **/

#include "api.h"
#include "partition.h"
#include "WallClock.h"
#include "qsh_interface.h"
#include "dynamic.h"

// #define DEBUG
// #define TEST
// #define TIMING

// globals used for testing
//D_Hierarchy h(3);
#ifdef TEST
D_Int32 i;
D_Int32 i2;
D_Int32 i3;
D_Int32 i4;
D_Int32 i5;
D_String s;
#endif // TEST

clocktype QshTimeFunction(EXTERNAL_Interface* iface)
{
    QshData* data = (QshData*) iface->data;

    return data->horizon;
}

void QshSimulationHorizonFunction(EXTERNAL_Interface* iface)
{
    QshData* data = (QshData*) iface->data;

    // Increase the interface's horizon if possible
    if (data->horizon > iface->horizon)
    {
        iface->horizon = data->horizon;
    }
}


void D_QshChangeModeCommand::ExecuteAsString(
    const std::string& in,
    std::string& out)
{
    QshData* data = (QshData*) iface->data;
    D_Hierarchy* h = &iface->partition->dynamicHierarchy;

    if (in == "real-time")
    {
        // If we were previously using time-managed mode, remove the
        // horizon variable
        if (strcmp(data->mode, "time-managed") == 0)
        {
            std::string path = "/qsh/horizon";
            h->RemoveLevel(path);
        }

        data->mode.Set(in);
        out = "success";

        // Set time management back to real-time
        EXTERNAL_SetTimeManagementRealTime(
            iface,
            0);
    }
    else if (in == "time-managed")
    {
        // If we were not previously using time-managed mode, add the
        // horizon variable
        if (strcmp(data->mode, "time-managed") != 0)
        {
            std::string path;

            // Add the qsh horizon variable to the hierarchy
            if (h->CreateExternalInterfacePath("qsh", "horizon", path))
            {
                h->AddObject(
                    path,
                    new D_ClocktypeObj(&data->horizon));
                h->SetWriteable(path, FALSE);
            }

            data->mode.Set(in);
            out = "success";

            // Register time-managed functions
            EXTERNAL_RegisterFunction(
                iface,
                EXTERNAL_TIME,
                (EXTERNAL_Function) QshTimeFunction);
            EXTERNAL_RegisterFunction(
                iface,
                EXTERNAL_SIMULATION_HORIZON,
                (EXTERNAL_Function) QshSimulationHorizonFunction);
        }
    }
    else if (in == "none")
    {
        // If we were previously using time-managed mode, remove the
        // horizon variable
        if (strcmp(data->mode, "time-managed") == 0)
        {
            std::string path = "/qsh/horizon";
            h->RemoveLevel(path);
        }

        data->mode.Set(in);
        out = "success";

        // Unregister time functions
        EXTERNAL_RegisterFunction(
            iface,
            EXTERNAL_TIME,
            NULL);
        EXTERNAL_RegisterFunction(
            iface,
            EXTERNAL_SIMULATION_HORIZON,
            NULL);
    }
    else
    {
        out = std::string("FAILURE -- Unknown mode \"") + in +  "\" (must be \"real-time\", \"time-managed\" or \"none\")";
    }
}

QshData::QshData(EXTERNAL_Interface* newIface) : changeMode(newIface)
{
    cwd = "/";
    horizon = 0;
    mode.Set("");
    EXTERNAL_SocketInit(&listenSocket, FALSE, FALSE);
    EXTERNAL_SocketInit(&s, FALSE, FALSE);
}

void QshInitialize(
    EXTERNAL_Interface* iface,
    NodeInput* nodeInput)
{
    // Only p0 initializes
    if (iface->partition->partitionId != 0)
    {
        return;
    }

    iface->partition->wallClock->disallowPause();

    EXTERNAL_SocketErrorType err;
    QshData* data;
    D_Hierarchy* h = &iface->partition->dynamicHierarchy;

    // Create Qsh specific data
    data = new QshData(iface);
    iface->data = (void*) data;

    data->mode.Set("real-time");

    std::string path;

    if (h->CreateExternalInterfacePath("qsh", "mode", path))
    {
        h->AddObject(path, new D_StringObj(&data->mode));
        h->SetWriteable(path, FALSE);
    }

    if (h->CreateExternalInterfacePath("qsh", "changeMode", path))
    {
        h->AddObject(path, &data->changeMode);
    }

    printf("Listening for connection from qsh...\n");
    err = EXTERNAL_SocketListen(
        &data->listenSocket,
        5132,
        &data->s);
    if (err != EXTERNAL_NoSocketError)
    {
        ERROR_ReportError("Qsh error opening socket");
    }

    QshSendPrompt(iface);

#ifdef TEST
    char str[MAX_STRING_LENGTH];

    try
    {
        h->AddLevel("/node/1", "Node 1");
        h->AddObject("/node/1/position", &i);
        h->AddLink("position", "node/1/position");
        h->AddObject("node/1/orientation", &i2);
        h->AddObject("node/1/interface/192.0.0.1/status", &i3);
        h->AddLink("interface/192.0.0.1", "node/1/interface/192.0.0.1");
        h->AddLink("subnet/N8-192.0.0.0/192.0.0.1", "node/1/interface/192.0.0.1");
        h->AddLevel("/entity", "FCS entities");
        h->AddLink("/entity/A101", "node/1", "FCS entity A101");
        h->AddLevel("/entity/A101/fcs/blah");
        h->AddObject("/entity/A101/radio", &i5);
        h->AddObject("/node/1/hostname", &s);
        s.WriteAsString("Node 1");
        i5.SetWriteable(FALSE);

        // error -- path exists
        h->AddObject("node/1", &i4);
    }
    catch (D_Exception &e)
    {
        e.GetFullErrorString(str);
        ERROR_ReportWarning(str);
    }

    try
    {
        // error -- part of path is object
        h->AddLevel("node/1/position/oops");
    }
    catch (D_Exception &e)
    {
        e.GetFullErrorString(str);
        ERROR_ReportWarning(str);
    }

    try
    {
        // error -- already added to hierarchy
        h->AddObject("okdir", &i);
    }
    catch (D_Exception &e)
    {
        e.GetFullErrorString(str);
        ERROR_ReportWarning(str);
    }

    i2 = 99;
    i5 = 321;

    try
    {
        h->WriteAsString("node/1/position", "5");
        h->WriteAsString("node/1/interface/192.0.0.1/status", "1");

        // error -- not writeable
        h->WriteAsString("entity/A101/radio", "77");
    }
    catch (D_Exception &e)
    {
        e.GetFullErrorString(str);
        ERROR_ReportWarning(str);
    }

    try
    {
        // error -- not executable
        h->ExecuteAsString("node/1/interface/192.0.0.1/status", "1", str);
    }
    catch (D_Exception &e)
    {
        e.GetFullErrorString(str);
        ERROR_ReportWarning(str);
    }

    h->Print();

#ifdef TIMING
    clocktype start = iface->partition->wallClock->getRealTime ();
    clocktype finish;
    for (int i = 0; i < 1000000; i++)
    {
        h->ReadAsString("node/1/interface/192.0.0.1/status", str);
        h->WriteAsString("node/1/position", "5");
        h->WriteAsString("node/1/interface/192.0.0.1/status", "1");
        h->ReadAsString("entity/A101/radio", str);
        h->ReadAsString("/qsh/mode", str);
    }
    finish = iface->partition->wallClock->getRealTime ();
    printf("took %f seconds\n", (double) (finish - start) / SECOND);
#endif // TIMING
#endif // TEST
}

void QshSendPrompt(EXTERNAL_Interface* iface)
{
    QshData* data;
    char prompt[MAX_STRING_LENGTH];

    data = (QshData*) iface->data;

    // Create the prompt
    sprintf(
        prompt,
        "{%fs %d%%} <%s> ('?' = help) ",
        (double) EXTERNAL_QuerySimulationTime(iface) / SECOND,
        (int) ((double) EXTERNAL_QuerySimulationTime(iface)
               / iface->partition->maxSimClock * 100.0),
        data->cwd.c_str());

    // Forward the prompt
    EXTERNAL_ForwardData(
        iface,
        NULL,
        prompt,
        (int)(strlen(prompt) + 1));

    EXTERNAL_ForwardData(
        iface,
        NULL,
        (void*) "\\\\prompt\\\\",
        (int)(strlen("\\\\prompt\\\\") + 1));

#ifdef DEBUG
    printf("Qsh sent prompt\n");
#endif // DEBUG
}

void QshFormPath(
    D_Hierarchy* h,
    const char* input,
	std::string& path,
    BOOL resolveLastLink = true)
{
    char token[MAX_STRING_LENGTH];
    char* nextToken;
    std::string newPath;

    // If the input starts with a '/' this is an absolute path and we should
    // start parsing at the root level.  Else start parsing at the current
    // level.
    if (input[0] == '/')
    {
        path = "/";
    }

    // If this is not an absolute path then create the new directory by
    // repeatedly resolving each token.  This is necessary because of
    // possible symbolic links.
    IO_GetDelimitedToken(token, input, "/", &nextToken);
    while (token[0] != 0)
    {
        // Handle ..
        if (strcmp(token, "..") == 0)
        {
            // If this level has a parent, then go to the parent.  If this
            // level does not have a parent (the root level) then stay at
            // the root level
            if (h->HasParent(path))
            {
                h->GetParent(path);
            }
        }
        else
        {
            // Get the child.  GetChild will throw an exception if token
            // is not a valid child.
            std::string tokenString = token;
            newPath = h->GetChildName(path, tokenString);
            path = newPath;

            // Resolve all links if resolveLastLink is true, or if this is
            // not the last link
            if (resolveLastLink || (nextToken[0] != 0))
            {
                h->ResolveLinks(path);
            }
        }

        IO_GetDelimitedToken(token, nextToken, "/", &nextToken);
    }
}

void QshFormSimplePath(
    const char* input,
    std::string& path)
{
    std::string oldPath;

    oldPath = path;
    if (input[0] == '/')
    {
        path = input;
    }
    else if (oldPath == "/")
    {
        path = std::string("/") + input;
    }
    else
    {
        path = oldPath + "/" + input;
    }
}

void QshParseLs(
    EXTERNAL_Interface* iface,
    char* input,
    char* args,
    std::string& cwd)
{
    QshData* data = (QshData*) iface->data;
    D_Hierarchy* h = &iface->partition->dynamicHierarchy;
    int numChildren;
    int i;
    char output[MAX_STRING_LENGTH];
	std::string path;
    char dir;
    char read;
    char write;
    char execute;
    char link[MAX_STRING_LENGTH];
    char description[MAX_STRING_LENGTH];
    D_Level* level;
    D_Level* child;
    BOOL recursive = FALSE;

    if (strcmp(input, "-R") == 0)
    {
        if (args == NULL)
        {
            path = cwd;
        }
        else
        {
            path = cwd;
            QshFormPath(h, args, path);
        }

        recursive = TRUE;
    }
    else
    {
        // Get the path of the directory we are changing to
        path = cwd;
        QshFormPath(h, input, path);

        if (args != NULL && strcmp(args, "-R") == 0)
        {
            recursive = TRUE;
        }
    }

    // Make sure that the path exists, and that the level is not an object
    if (h->IsObject(path))
    {
        throw D_ExceptionIsObject(path.c_str());
    }

    // If a directory argument was supplied then print out the path
    if (input[0] != 0)
    {
        if (recursive)
        {
            sprintf(output, "%s:\n", path.c_str());
        }
        else
        {
            sprintf(output, "%s:\n", input);
        }

        EXTERNAL_ForwardData(
            iface,
            NULL,
            output,
            (int)(strlen(output) + 1));
    }

    numChildren = h->GetNumChildren(path);
    for (i = 0; i < numChildren; i++)
    {
        child = h->GetChild(path, i);
        printf("%s child %d %s\n", path.c_str(), i, child->GetFullPath().c_str());

        // By default this level does nothing
        read = '-';
        write = '-';
        execute = '-';
        dir = '-';
        strcpy(link, "");
        strcpy(description, "");

        // If it is a link then be sure to print out the link information
        if (child->IsLink())
        {
            sprintf(link, " -> %s", child->GetLink()->GetFullPath().c_str());
        }

        // Check if the level has a description then print it
        if (child->HasDescription())
        {
            sprintf(description, " (%s)", child->GetDescription().c_str());
        }

        // Now resolve all links
        child = child->ResolveLinks();

        // If the new level has a description and the old one didn't, then
        // print the new level's description
        if (child->HasDescription() && description[0] == 0)
        {
            sprintf(description, " (%s)", child->GetDescription().c_str());
        }

        if (h->IsObject(child->GetFullPath()))
        {
            if (h->IsReadable(child->GetFullPath()))
            {
                read = 'r';
            }
            if (h->IsWriteable(child->GetFullPath()))
            {
                write = 'w';
            }
            if (h->IsExecutable(child->GetFullPath()))
            {
                execute = 'x';
            }
        }
        else
        {
            dir = 'd';
        }

        sprintf(
            output,
            "%c%c%c%c    %s%s%s\n",
            dir,
            read,
            write,
            execute,
            h->GetChildName(path, i).c_str(),
            link,
            description);

        EXTERNAL_ForwardData(
            iface,
            NULL,
            output,
            (int)(strlen(output) + 1));
    }

    // If -R is passed then do a recursive ls
    if (recursive)
    {
        // Send an additional '\n'
        sprintf(output, "\n");
        EXTERNAL_ForwardData(
            iface,
            NULL,
            output,
            (int)(strlen(output) + 1));

        // Loop through all children in this level
        numChildren = h->GetNumChildren(path);
        for (i = 0; i < numChildren; i++)
        {
            child = h->GetChild(path, i);

            // Do not recursively ls links
            if (child->IsLink())
            {
                continue;
            }

            // If the level is not an object, recursively ls
            if (!child->IsObject())
            {
                QshParseLs(iface, "-R", "", child->GetFullPath());
            }
        }
    }
}

void QshParsePwd(
    EXTERNAL_Interface* iface,
    char* input)
{
    QshData* data = (QshData*) iface->data;
    char output[MAX_STRING_LENGTH];

    sprintf(output, "%s\n", data->cwd.c_str());

    EXTERNAL_ForwardData(
        iface,
        NULL,
        output,
        (int)(strlen(output) + 1));
}

void QshParseCd(
    EXTERNAL_Interface* iface,
    char* input)
{
    QshData* data = (QshData*) iface->data;
    D_Hierarchy* h = &iface->partition->dynamicHierarchy;
	std::string newCwd;
    BOOL isObject = TRUE;

    // If the path is blank, then cd /
    if (input[0] == 0)
    {
        data->cwd = "/";
        return;
    }

    // Get the path of the directory we are changing to
    newCwd = data->cwd;
    QshFormPath(h, input, newCwd);

    // Make sure that the path exists, and that the level is not an object
    if (h->IsObject(newCwd))
    {
        throw D_ExceptionIsObject(newCwd);
    }

    // If no exception, set the new path
    data->cwd = newCwd;
}

void QshParseRead(
    EXTERNAL_Interface* iface,
    char* input)
{
    QshData* data = (QshData*) iface->data;
    D_Hierarchy* h = &iface->partition->dynamicHierarchy;
    char output[MAX_STRING_LENGTH];
	std::string value;
	std::string path;

    // Get the path of the directory we are changing to
    path = data->cwd;
    QshFormPath(h, input, path);

    // Read the value, and forward it to the interface
    h->ReadAsString(path, value);
    sprintf(output, "%s:\n%s\n", input, value.c_str());
    EXTERNAL_ForwardData(
        iface,
        NULL,
        output,
        (int)(strlen(output) + 1));
}

void QshParseWrite(
    EXTERNAL_Interface* iface,
    char* path,
	std::string& input)
{
    QshData* data = (QshData*) iface->data;
    D_Hierarchy* h = &iface->partition->dynamicHierarchy;
    char output[MAX_STRING_LENGTH];
	std::string value;
	std::string fullPath;

    // Get the path of the directory we are changing to
    fullPath = data->cwd;
    QshFormPath(h, path, fullPath);

    // Make sure that the path exists, and that the level is an object
    if (!h->IsObject(fullPath))
    {
        throw D_ExceptionNotObject(fullPath.c_str());
    }

    // Read the value, and forward it to the interface
    h->WriteAsString(fullPath, input);
    h->ReadAsString(fullPath, value);
    sprintf(output, "%s:\n%s\n", path, value.c_str());
    EXTERNAL_ForwardData(
        iface,
        NULL,
        output,
        (int)(strlen(output) + 1));
}

void QshParseCommand(
    EXTERNAL_Interface* iface,
	char* command,
	std::string& args)
{
	std::string fullPath;
    std::string output;
	std::string out;
    QshData* data = (QshData*) iface->data;
    D_Hierarchy* h = &iface->partition->dynamicHierarchy;

    // Get the path of the command we are trying to execute.  If the path is
    // invalid, then it is an unknown command.
    try
    {
        fullPath = data->cwd;
        QshFormPath(h, command, fullPath);

        if (!h->IsObject(fullPath))
        {
            // If the full path is invalid or not an object, then it is an
            // unknown command
            output = std::string("Unknown command \"") + command + "\"\n";
        }
        else
        {
            // Execute the command at the given path
            h->ExecuteAsString(fullPath, args, out);
            output = fullPath + " " + args + ":\n" + out + "\n";
        }
    }
    catch (D_ExceptionInvalidPath)
    {
        output = std::string("Unknown command \"") + command + "\"\n";
    }

    // Send back the results
    EXTERNAL_ForwardData(
        iface,
        NULL,
        (void*) output.c_str(),
        output.size() + 1);
}

#ifdef D_LISTENING_ENABLED
void QshParseListen(
    EXTERNAL_Interface* iface,
    char* input,
    char* args)
{
    std::string path;
    char str[MAX_STRING_LENGTH];
    QshData* data = (QshData*) iface->data;
    D_Hierarchy* h = &iface->partition->dynamicHierarchy;
    std::string percent;
    char* percentCh;

    // Get the path of the directory we are changing to
    path = data->cwd;
    QshFormPath(h, input, path);

    // Make sure a directory was supplied
    if (path == data->cwd)
    {
        throw D_Exception("Must supply argument for listen");
    }

    // Check if the user supplied an additional argument, check if it is a
    // percent.  If not throw an exception.
    try
    {
        if (args != NULL && args[0] != 0)
        {
            h->AddListener(
                path,
                "percent",
                args,
                "qsh",
                new QshListenerCallback(path, iface));
        }
        else
        {
            h->AddListener(
                path,
                "listener",
                "",
                "qsh",
                new QshListenerCallback(path, iface));
        }
    }
    catch (D_ExceptionInvalidPath)
    {
        std::string output = std::string("Unknown path \"") + path + "\"\n";
        // Send back the results
        EXTERNAL_ForwardData(
            iface,
            NULL,
            (void*) output.c_str(),
            output.size() + 1);
    }
}

void QshParseUnlisten(
    EXTERNAL_Interface* iface,
    char* input)
{
    std::string path;
    QshData* data = (QshData*) iface->data;
    D_Hierarchy* h = &iface->partition->dynamicHierarchy;

    // Get the path of the directory we are changing to
    path = data->cwd;
    QshFormPath(h, input, path);

    h->RemoveListeners(path, "qsh");
}
#endif // D_LISTENING_ENABLED

void QshParseRm(
    EXTERNAL_Interface* iface,
    char* input)
{
	std::string path;
    QshData* data = (QshData*) iface->data;
    D_Hierarchy* h = &iface->partition->dynamicHierarchy;
    D_Level* level;

    // Get the path of the directory we are going to remove.  Do not
    // resolve the final link (if the last level in the path is a link)
    path = data->cwd;
    QshFormPath(h, input, path, FALSE);

    // Make sure the current directory is not being removed
    if (path == data->cwd)
    {
        throw D_Exception("Must supply argument for rm");
    }

    // Remove the level, not resolving links
    h->RemoveLevel(path);
}

void QshParseMkdir(
    EXTERNAL_Interface* iface,
    char* input)
{
    std::string path;
    QshData* data = (QshData*) iface->data;
    D_Hierarchy* h = &iface->partition->dynamicHierarchy;

    // Make the path of the directory to create
    path = data->cwd;
    QshFormSimplePath(input, path);

    // Make sure a directory was supplied
    if (path == data->cwd)
    {
        throw D_Exception("Must supply argument for mkdir");
    }

#ifdef PARALLEL
    h->AddLevel(path, iface->partition->partitionId);
#else // PARALLEL
    h->AddLevel(path);
#endif // PARALLEL
}

void QshParseLn(
    EXTERNAL_Interface* iface,
    char* input,
    char* name)
{
	std::string path;
    std::string namePath;
    QshData* data = (QshData*) iface->data;
    D_Hierarchy* h = &iface->partition->dynamicHierarchy;

    // Get the source of the ln
    path = data->cwd;
    QshFormPath(h, input, path);

    // Make sure a directory was supplied
    if (path == data->cwd)
    {
        throw D_Exception("Must supply path argument for ln");
    }

    // Get the target of the ln
    namePath = data->cwd;
    QshFormSimplePath(name, namePath);

    // Make sure a directory was supplied
    if (namePath == data->cwd)
    {
        throw D_Exception("Must supply target argument for ln");
    }

    // Add the link
    h->AddLink(namePath, path);
}

void QshParseHelp(
    EXTERNAL_Interface* iface,
    char* input)
{
    // Local #define used for simplifying this function
    #define SEND_OUTPUT \
    EXTERNAL_ForwardData( \
        iface, \
        NULL, \
        output, \
        (int)(strlen(output) + 1))

    char output[MAX_STRING_LENGTH];

    // Print the menu
    sprintf(output, "\nHelp\n\n");
    SEND_OUTPUT;
    sprintf(output, "    ls [path]                get directory list\n");
    SEND_OUTPUT;
    sprintf(output, "    cd [path]                change directories\n");
    SEND_OUTPUT;
    sprintf(output, "    pwd                      print working directory\n");
    SEND_OUTPUT;
    sprintf(output, "    read <path>              read the value of an object\n");
    SEND_OUTPUT;
    sprintf(output, "    write <path> <value>     write a new value to an object\n");
    SEND_OUTPUT;
#ifdef D_LISTENING_ENABLED
    sprintf(output, "    listen <path> [help]     add a listener to an object\n");
    SEND_OUTPUT;
    sprintf(output, "    unlisten <path>          stop listening to an object\n");
    SEND_OUTPUT;
#endif // D_LISTENING_ENABLED
    sprintf(output, "    rm <path>                remove a directory (may be unrecoverable)\n");
    SEND_OUTPUT;
    sprintf(output, "    mkdir <path>             make a new directory\n");
    SEND_OUTPUT;
    sprintf(output, "    ln <path> <target>       create a link\n");
    SEND_OUTPUT;
    sprintf(output, "    <enter>                  skip a prompt\n");
    SEND_OUTPUT;
    sprintf(output, "    q                        quit\n\n");
    SEND_OUTPUT;
}

void QshParseInput(
    EXTERNAL_Interface* iface,
    char* input)
{
    QshData* data;
    char command[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];
    char* nextToken;
    char* firstArg;

    data = (QshData*) iface->data;

    // Get command token
    IO_GetToken(command, input, &nextToken);

    // Determine the first argument
    firstArg = nextToken;
    if (firstArg != NULL)
    {
        while (*firstArg != 0 && isspace(*firstArg))
        {
            firstArg++;
        }
    }

    // If a command was given, process it
    if (command[0] > 0)
    {
        // Get first argument
        IO_GetToken(arg, nextToken, &nextToken);

        // advance nextToken past all whitespace
        if (nextToken != NULL)
        {
            while (*nextToken != 0 && isspace(*nextToken))
            {
                nextToken++;
            }
        }

        // Execute the command
        if (strcmp(command, "ls") == 0)
        {
            QshParseLs(iface, arg, nextToken, data->cwd);
        }
        else if (strcmp(command, "cd") == 0)
        {
            QshParseCd(iface, arg);
        }
        else if (strcmp(command, "read") == 0)
        {
            QshParseRead(iface, arg);
        }
        else if (strcmp(command, "write") == 0)
        {
			std::string token = nextToken;
            QshParseWrite(iface, arg, token);
        }
        else if (strcmp(command, "pwd") == 0)
        {
            QshParsePwd(iface, arg);
        }
#ifdef D_LISTENING_ENABLED
        else if (strcmp(command, "listen") == 0)
        {
            QshParseListen(iface, arg, nextToken);
        }
        else if (strcmp(command, "unlisten") == 0)
        {
            QshParseUnlisten(iface, arg);
        }
#endif // D_LISTENING_ENABLED
        else if (strcmp(command, "rm") == 0)
        {
            QshParseRm(iface, arg);
        }
        else if (strcmp(command, "mkdir") == 0)
        {
            QshParseMkdir(iface, arg);
        }
        else if (strcmp(command, "ln") == 0)
        {
            QshParseLn(iface, arg, nextToken);
        }
        else if (strcmp(command, "?") == 0
                 || strcmp(command, "help") == 0)
        {
            QshParseHelp(iface, arg);
        }
        else
        {
            // Not a built-in command.  Check if it is a dynamic command.
			std::string first = firstArg;
            QshParseCommand(iface, command, first);
        }
    }

    QshSendPrompt(iface);
}

void QshReceive(EXTERNAL_Interface* iface)
{
    if (iface->partition->partitionId != 0)
    {
        return;
    }

    char in[256];
    unsigned int size;
    QshData* data;
    EXTERNAL_SocketErrorType err;
    std::string errString;

#ifdef TEST
    // Periodically update i, which is /node/1/position
    static int timesCalled = 0;
    if (++timesCalled % 100 == 0)
    {
        i = i + 1;
    }
#endif // TEST

     // Extract the interface-specific data
    data = (QshData*) iface->data;

    // Keep receiving data while availabile
    do
    {
        // Attempt to receive up to 256 bytes from the data socket.
        // Store the results in the "in" variable.  The FALSE parameter
        // specifies that this recv operation will not block.
        err = EXTERNAL_SocketRecv(
            &data->s,
            in,
            256,
            &size,
            FALSE);
        if (err != EXTERNAL_NoSocketError)
        {
            ERROR_ReportWarning("Error receiving data from socket");
            exit(0);
        }

        // If data was received
        if (size > 0)
        {
#ifdef DEBUG
            printf("Qsh received %d bytes\n", size);
#endif // DEBUG
            if (!iface->partition->dynamicHierarchy.IsEnabled())
            {
                errString = "QualNet is not dynamically enabled";
                EXTERNAL_ForwardData(
                    iface,
                    NULL,
                    (void*) errString.c_str(),
                    (int)(errString.size() + 1));
                return;
            }

            // Attempt to parse the input.  If an exception was thrown then
            // report the error to qualnet and to the interface
            try
            {
                QshParseInput(iface, in);
            }
            catch (D_Exception &e)
            {
                // Report warning in qualnet
                e.GetFullErrorString(errString);
                ERROR_ReportWarning((char*) errString.c_str());

                // Send warning to interface
                errString = e.GetError() + "\n";
                EXTERNAL_ForwardData(
                    iface,
                    NULL,
                    (void*) errString.c_str(),
                    (int)(errString.size() + 1));

                QshSendPrompt(iface);
                return;
            }
        }
    } while (size > 0);
}

void QshForward(
    EXTERNAL_Interface* iface,
    Node* node,
    void* forwardData,
    int forwardSize)
{
#ifdef DEBUG
    printf("Qsh send \"%s\" (%d bytes)\n", forwardData, forwardSize);
#endif // DEBUG

    QshData* data = (QshData*) iface->data;

    EXTERNAL_SocketSend(
        &data->s,
        (char*) forwardData,
        forwardSize);
}

void QshFinalize(EXTERNAL_Interface* iface)
{
    QshData* data;

    if (iface->partition->partitionId != 0)
    {
        return;
    }

    data = (QshData*) iface->data;

    if (EXTERNAL_SocketValid(&data->listenSocket))
    {
        EXTERNAL_SocketClose(&data->listenSocket);
    }

    if (EXTERNAL_SocketValid(&data->s))
    {
        EXTERNAL_SocketClose(&data->s);
    }
}
