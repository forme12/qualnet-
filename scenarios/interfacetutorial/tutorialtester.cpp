#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <stdio.h>
#include <string.h>
#include <assert.h>

int gRunning = 1;

void HandlePrompt(int s)
{
    char command[256];
    int sentSize;

    printf("Enter command (\"?\" = help)\n");
    printf(": ");
    fgets(command, 256, stdin);
    if (strlen(command) <= 1)
    {
        return;
    }

    command[strlen(command) - 1] = 0;

    if (strcmp(command, "q") == 0 || strcmp(command, "Q") == 0)
    {
        gRunning = 0;
    }
    else if (strcmp(command, "?") == 0)
    {
        printf("\n");
        printf("Menu\n");
        printf("----\n");
        printf("<src node ID> <dest node ID> s <data>     send data between nodes\n");
        printf("<src node ID> <dest node ID> g            query dest node's data\n");
        printf("[enter]                                   skip a prompt\n");
        printf("q                                         quit\n");
    }
    else
    {
        // Send the command, including terminating NULL character
        sentSize = send(s, command, strlen(command) + 1, 0);
        assert(sentSize == strlen(command) + 1);
    }
}

void HandleReceive(int s)
{
    fd_set readset;
    struct timeval timeout;
    int retval;
    char buf[256];

    FD_ZERO(&readset);
    FD_SET(s, &readset);
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    retval = select(s + 1, &readset, NULL, NULL, &timeout);
    if (retval)
    {
        retval = recv(s, buf, 256, 0);
        assert(retval > 0);
        printf("Received \"%s\"\n", buf);
    }
}

int main()
{
    int s;
    int err;
    struct sockaddr_in connectAddr;

#ifdef _WIN32
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0)
    {
        printf("Error initializing socket library\n");
        return -1;
    }
#endif

    s = socket(AF_INET, SOCK_STREAM, 0);
    assert(s != -1);

    connectAddr.sin_family = AF_INET;
    connectAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    connectAddr.sin_port = htons(5132);
    memset(&connectAddr.sin_zero, 0, 8);

    printf("Attempting to connect to QualNet...\n");
    err = connect(s, (sockaddr*) &connectAddr, sizeof(sockaddr));
    assert(err != -1);

    while (gRunning)
    {
        HandlePrompt(s);
        HandleReceive(s);
    }

#ifdef _WIN32
        closesocket(s);

        WSACleanup();
#else /* unix/linux */
        shutdown(s, SHUT_RDWR);
#endif

    return 0;
}
