#ifndef CLIENTS_H
#define CLIENTS_H

#include <netinet/in.h>

struct AcceptedClient{
    int acceptedSocketFD;
    struct sockaddr_in address;
    int acceptedSuccessfully;
    char language[3];
    char username[30];
    struct ChatRoom *chatRoom;
    int isInQueue;
};

#endif