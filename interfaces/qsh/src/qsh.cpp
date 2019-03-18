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

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define DEBUG 0

#define RECEIVE_SIZE 2000

char promptBuffer[256];
int gRunning = 1;
char prompt = 0;

void HandlePrompt(int s)
{
    char buffer[256];
    int sentSize;

    fgets(buffer, 256, stdin);

    // Overwrite the return character with a NULL character
    buffer[strlen(buffer) - 1] = 0;

    prompt = 0;

#if DEBUG
    printf("Read %d bytes: %s\n", strlen(buffer), buffer);
#endif // DEBUG

    // If it is the quit command
    if (strcmp(buffer, "q") == 0 || strcmp(buffer, "Q") == 0)
    {
        gRunning = 0;
        return;
    }
        
    // Send the buffer, including terminating NULL character
    sentSize = send(s, buffer, strlen(buffer) + 1, 0);
    assert(sentSize == strlen(buffer) + 1);

    return;
}

char HandleReceive(int s)
{
    fd_set readset;
    struct timeval timeout;
    int retval;
    char buf[RECEIVE_SIZE];
    char last[RECEIVE_SIZE];
    char prompt = 0;
    char* ch;

    // Set up the select timeout as 10ms
    FD_ZERO(&readset);
    FD_SET(s, &readset);
    timeout.tv_sec = 0;
    timeout.tv_usec = 10000;

    // Look for data for 10ms
    retval = select(s + 1, &readset, NULL, NULL, &timeout);
    if (retval)
    {
        // If there is data, receive up to RECEIVE_SIZE bytes
        retval = recv(s, buf, RECEIVE_SIZE, 0);
        assert(retval > 0);

#if DEBUG
        printf("received \"%s\" (%d bytes)\n", buf, retval);
#endif // DEBUG

        // Parse each string received
        ch = buf;
        while (ch - buf < retval - 1)
        {
#if DEBUG
            printf("parsed \"%s\"\n", ch);
#endif // DEBUG

            // If the string is \\prompt\\ then get user input, otherwise
            // print out the data we received
            if (strstr(ch, "\\\\prompt\\\\") != NULL)
            {
                prompt = 1;
                strcpy(promptBuffer, last);
            }
            else
            {
                printf("%s", ch);
            }

            strcpy(last, ch);

            // Jump to the next string
            ch += strlen(ch) + 1;
        }
    }

    return prompt;
}

int main()
{
    int s;
    int err;
    struct sockaddr_in connectAddr;

    err = setvbuf(stdin, NULL, _IONBF, 0);
    if (err == EOF)
    {
        printf("Cannot set to non-buffered input\n");
        exit(0);
    }

    // If running on windows then initialize the winsock library
#ifdef _WIN32
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0)
    {
        printf("Error initializing socket library\n");
        return -1;
    }
#endif

    // Create a socket connecting to 127.0.0.1:5132
    s = socket(AF_INET, SOCK_STREAM, 0);
    assert(s != -1);

    connectAddr.sin_family = AF_INET;
    connectAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    connectAddr.sin_port = htons(5132);
    memset(&connectAddr.sin_zero, 0, 8);

    // Connect to QualNet
    printf("Attempting to connect to QualNet...\n");
    err = connect(s, (sockaddr*) &connectAddr, sizeof(sockaddr));
    assert(err != -1);

    // Main event loop
    while (gRunning)
    {
#if DEBUG
        printf("receive\n");
#endif // DEBUG

        // Receive data.  If return value is 1 show the prompt for user
        // input
        if (HandleReceive(s))
        {
            prompt = 1;

#if DEBUG
            printf("prompt\n");
#endif // DEBUG
        }

        if (prompt)
        {
            HandlePrompt(s);
        }
    }

    // Close the socket
#ifdef _WIN32
    // Windows
    closesocket(s);

    WSACleanup();
#else // _WIN32
    // unix/linux
    shutdown(s, SHUT_RDWR);
#endif // _WIN32

    return 0;
}
