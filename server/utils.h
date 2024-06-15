#ifndef UTILS_H
#define UTILS_H
    #include <stdio.h>
    #include <unistd.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <pthread.h>
    #include <string.h>
    #include "queue.h"

    //  chatRoom
    struct ChatRoom {
        int ID;
        char *name;
        char *language;
        int maxClient;
        pthread_mutex_t mutex;
        pthread_cond_t cond;
        struct AcceptedClient *waitingClients[MAXQUEUE + 1];
    };

    //  Array di client
    struct AcceptedClient{
        int acceptedSocketFD;
        struct sockaddr_in address;
        int acceptedSuccessfully;
        char *language;
        struct ChatRoom *ID_chatRoom;
    };

    struct AcceptedClient *loggedClient[10];
    int loggedClientCount = 0;

    int createTCPSocket();
    struct sockaddr_in* createIPAddress(char *ip, int port);    
    struct AcceptedClient* acceptIncomingConnection(int serverSocketFD);
    void startAcceptingIncomingConnection(int serverSocketFD);
    void manageNewConnectionOnSeparateThread(struct AcceptedClient *client);
    void manageNewConnection(struct AcceptedClient **client);
    void sendMessageToTheChatRoom(struct AcceptedClient *client, char *message);
    void backEndPrompt(struct AcceptedClient **client);
    void manageChatRoom(struct AcceptedClient *client);
    char **traslateMessage(char *message, char *language);  
    void joinOrWaitForTheSelectedChatRoom(struct AcceptedClient **client);

#endif