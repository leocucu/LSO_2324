#ifndef UTILS_H
#define UTILS_H
    #include <stdio.h>
    #include <unistd.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <pthread.h>
    #include <string.h>
    #include "queuelst.h"

    //  Socket
    int createTCPSocket();
    struct sockaddr_in* createIPAddress(const char *ip, int port);    
    struct AcceptedClient* acceptIncomingConnection(int serverSocketFD);
    void startAcceptingIncomingConnection(int serverSocketFD);
    void manageNewConnectionOnSeparateThread(struct AcceptedClient *client);
    void setTimeoutOnSocket(int sock, int seconds);

    //  Client Handler
    void* manageNewConnection(void* args);
    void removeClient(struct AcceptedClient* client);

    //  Login
    int registerClient(struct AcceptedClient* client);
    int login(struct AcceptedClient* client);


#endif