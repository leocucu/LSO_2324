#ifndef CLIENTS_H
#define CLIENTS_H

#include <netinet/in.h>

#define MAX_USERNAME_LEN 20

struct AcceptedClient{
    int acceptedSocketFD;
    struct sockaddr_in address;
    int acceptedSuccessfully;
    char language[3];
    char username[MAX_USERNAME_LEN + 1];
    struct ChatRoom *chatRoom;
    int isInQueue;
};

#endif