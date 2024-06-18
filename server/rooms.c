#include "rooms.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "database.h"
#include "utils.h"

struct ChatRoom rooms[MAX_ROOMS];
const char* names[] = {"GeneraleIT", "GeneraleEN", "GeneraleFR", "GeneraleES", "GeneralePT"};
const char* languages[] = {"IT", "EN", "FR", "ES", "PT"};
int max[] = {1, 3, 3, 3, 3};

//  Rooms
int joinOrWaitForTheSelectedChatRoom(struct AcceptedClient *client, struct ChatRoom *r) {
    pthread_mutex_lock(&(r->mutex));
    char buffer[1024];
    printf("%s vuole entrare in %s, connected: %d, max: %d\n", client->username, r->name, r->connected, r->maxClient);
    client->chatRoom = r;
    if (r->connected < r->maxClient) {
        joinRoom(client, r);
    }
    else {
        printf("%s si mette in coda\n", client->username);
        client->isInQueue = 1;
        enqueue(&(r->waitingClients), client);
        int turn = r->waitingClients.size;
        while (1) {
            if(r->connected < r->maxClient && (struct AcceptedClient*)r->waitingClients.tail->item == client){
                joinRoom(client, r);
                dequeue(&(r->waitingClients));
                client->isInQueue = 0;
                break;
            }
            sprintf(buffer, "%d", turn);
            write(client->acceptedSocketFD, buffer, 10);
            turn--;
            pthread_cond_wait(&(r->cond), &(r->mutex));
        }
        // *client = (struct AcceptedClient*) dequeue(r->waitingClients);
    }

    pthread_mutex_unlock(&(r->mutex));
    return 0;
}

void joinRoom(struct AcceptedClient* client, struct ChatRoom* r){
    for(int i = 0; i < r->maxClient; i++){
        if(r->clients[i] == NULL){
            r->clients[i] = client;
            r->connected = r->connected + 1;
            break;
        }
    }
}

void leaveChatRoom(struct AcceptedClient* client){
    pthread_mutex_lock(&(client->chatRoom->mutex));
    for (int i = 0; i < client->chatRoom->maxClient; i++){
        if (client->chatRoom->clients[i] == client){
            client->chatRoom->clients[i] = NULL;
            client->chatRoom->connected--;
            break;
        }
    }
    pthread_cond_broadcast(&(client->chatRoom->cond));
    pthread_mutex_unlock(&(client->chatRoom->mutex));
    client->chatRoom = NULL;
}

void initRooms(){
    for(int i = 0; i < MAX_ROOMS; i++){
        rooms[i].connected = 0;
        rooms[i].maxClient = max[i];
        strcpy(rooms[i].language, languages[i]);
        strcpy(rooms[i].name, names[i]);

        rooms[i].clients = malloc(sizeof(struct AcceptedClient*) * rooms[i].maxClient);
        for(int j = 0; j < rooms[i].maxClient; j++ ){
            rooms[i].clients[j] = NULL;
        }
        initq(&(rooms[i].waitingClients));

        pthread_mutex_init(&(rooms[i].mutex), NULL);
        pthread_cond_init(&(rooms[i].cond), NULL);
        printf("Room %s %s, max: %d, connected: %d\n", rooms[i].name, rooms[i].language, rooms[i].maxClient, rooms[i].connected);
    }
}

//  Chat
void sendMessageToTheChatRoom(struct ChatRoom *r, char *message) {
    pthread_mutex_lock(&(r->mutex));
    for (int i = 0; i < r->maxClient; i++) {
        if(r->clients[i] != NULL) {
            send(r->clients[i]->acceptedSocketFD, message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&(r->mutex));
}

void manageChatRoom(struct AcceptedClient *client, struct ChatRoom* r) {
    int n;
    char buffer[1024];
    char translated[1024];
    while (1){
        if ((n = read(client->acceptedSocketFD, buffer, 1024)) <= 0)
            removeClient(client);
        char messaggio[2048];
        buffer[n] = '\0';
        if(buffer[0] == '/'){
            if (strcmp(buffer, "/exit") == 0){
                sprintf(messaggio, "%s left room\n", client->username);
                leaveChatRoom(client);
                sendMessageToTheChatRoom(r, messaggio);
                break;
            }
        }
        else{
            if(strcmp(client->language, r->language) == 0){
                sprintf(messaggio, "<%s>:   %s", client->username, buffer);
                sendMessageToTheChatRoom(r, messaggio);
            } else {
                traslateMessage(buffer, translated, client->language, r->language);
                sprintf(messaggio, "<%s>:   %s", client->username, translated);
                sendMessageToTheChatRoom(r, messaggio);
            }
        }
    }
}

void traslateMessage(char *message, char*translated, const char *lan1, const char *lan2) {
    char* token;
    char translTok[50];
    char* rest = message;
    strcpy(translated, "");
    printf("Traduco %s da %s a %s\n", message, lan1, lan2);
    
    while ((token = strtok_r(rest, " ", &rest))) {
        //traslate token
        if(getTranslation((const char*)token, translTok, lan1, lan2) != DB_OK)
            strcpy(translTok, token);
        strcat(translated, translTok);
        strcat(translated, " ");
    }
}