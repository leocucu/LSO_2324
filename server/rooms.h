#ifndef ROOMS_H
#define ROOMS_H

#include <pthread.h>
#include "queuelst.h"
#include "clients.h"

#define MAX_ROOMS 5

struct ChatRoom{
    char name[30];
    char language[3];
    int maxClient;
    int connected;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    struct queue waitingClients;
    struct AcceptedClient **clients;
};

//  Rooms
int joinOrWaitForTheSelectedChatRoom(struct AcceptedClient *client, struct ChatRoom *r);
void initRooms();
void leaveChatRoom(struct AcceptedClient* client);
void joinRoom(struct AcceptedClient* client, struct ChatRoom* r);

//  Chat
void sendMessageToTheChatRoom(struct ChatRoom *r, char *message);
void manageChatRoom(struct AcceptedClient *client, struct ChatRoom* r);
void traslateMessage(char *message, char*translated, const char *lan1, const char* lan2);  


#endif