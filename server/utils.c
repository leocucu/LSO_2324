#include "utils.h"
#include "../customerr.h"
#include "database.h"
#include <stdlib.h>
#include <stdio.h>
#include "clients.h"
#include "rooms.h"

#define MAX_CLIENTS 10

struct AcceptedClient *clients[MAX_CLIENTS] = {NULL};
extern struct ChatRoom rooms[];

int loggedClientCount = 0;

//  Socket

int createTCPSocket() {
    return socket(AF_INET,SOCK_STREAM,0);
}
    
struct sockaddr_in* createIPAddress(const char *ip, int port) {
    struct sockaddr_in *address = malloc(sizeof(struct sockaddr_in));
    address->sin_family = AF_INET;
    address->sin_port = htons(port);
    inet_pton(AF_INET, ip, &address->sin_addr.s_addr);
    return address;
}

struct AcceptedClient* acceptIncomingConnection(int serverSocketFD) {
    struct sockaddr_in clientAddress;
    socklen_t clientAddressSize = sizeof(struct sockaddr_in);
    int clientSocketFD = accept(serverSocketFD, (struct sockaddr*)&clientAddress, &clientAddressSize);

    struct AcceptedClient* acceptedSocket = malloc(sizeof(struct AcceptedClient));
    acceptedSocket->address = clientAddress;
    acceptedSocket->acceptedSocketFD = clientSocketFD;
    acceptedSocket->acceptedSuccessfully = clientSocketFD > 0;
    acceptedSocket->chatRoom = NULL;

    for(int i = 0; i < MAX_CLIENTS; i++){
        if(clients[i] == NULL){
            clients[i] = acceptedSocket;
            break;
        }
    }
    return acceptedSocket;
}

void startAcceptingIncomingConnection(int serverSocketFD) {
    while (1) {
        struct AcceptedClient *clientSocket = acceptIncomingConnection(serverSocketFD);
        manageNewConnectionOnSeparateThread(clientSocket);
    }
}

void manageNewConnectionOnSeparateThread(struct AcceptedClient *client) {
    pthread_t id;
    pthread_create(&id,NULL,manageNewConnection, client);
}

//  Client Handler

void* manageNewConnection(void* args) {
    //login
    struct AcceptedClient* client = (struct AcceptedClient*)args;
    char buffer[1024];
    int n;

    while(1){
        if((n = read(client->acceptedSocketFD, buffer, 1024)) <= 0) removeClient(client);

        if((int)buffer[0] == '0'){
            printf("Logging in\n");
            if(login(client) == LOGIN_OK)
                break;
        } else if((int)buffer[0] == '1') {
            printf("Register in\n");
            if(registerClient(client) == LOGIN_OK)
                break;
        }
    }
    //accesso alla stanza

    while(1){
        if((n = read(client->acceptedSocketFD, buffer, 1024)) <= 0) removeClient(client);
        buffer[n] = '\0';
        int room;
        sscanf(buffer, "%d", &room);
        printf("client requested room %d, %s\n", room, rooms[room].language);
        joinOrWaitForTheSelectedChatRoom(client, &(rooms[room]));
        char msg[2] = {'0', '\0'};
        if(write(client->acceptedSocketFD, msg, strlen(msg)) <= 0) removeClient(client);
            
        sprintf(buffer, "%s joined the room\n", client->username);
        printf("%s joined the room\n", client->username);
        sendMessageToTheChatRoom(&(room[rooms]), buffer);
        manageChatRoom(client, &rooms[room]);
    }

    return NULL;
}

void removeClient(struct AcceptedClient *client){
    printf("Client disconnected\n");
    if(client->chatRoom != NULL && client->isInQueue){
        pthread_mutex_lock(&(client->chatRoom->mutex));
        findAndRemove(&(client->chatRoom->waitingClients), client);
        pthread_mutex_unlock(&(client->chatRoom->mutex));
    } else if(client->chatRoom != NULL){
        leaveChatRoom(client);
    }

    close(client->acceptedSocketFD);
    for(int i = 0; i < MAX_CLIENTS; i++){
        if(client == clients[i]){
            free(clients[i]);
            clients[i] = NULL;
            pthread_exit(NULL);
            break;
        }
    }
}

//  Login

int login(struct AcceptedClient* client){
    int n;
    char username[1024];
    char dbpassword[1024];
    char password[1024];
    char language[5];
    char res[1024];

    if((n = read(client->acceptedSocketFD, username, 1024)) <= 0) removeClient(client);
    username[n] = '\0';
    printf("%s\n", username);
    
    if((n = read(client->acceptedSocketFD, password, 1024)) <= 0) removeClient(client);
    password[n] = '\0';
    printf("%s\n", password);

    // // DÀ SEMPRE OK, SOLO PER TESTING !!!!!!!!!!!!!!!
    // sprintf(res, "%d", LOGIN_OK);
    // if(write(client->acceptedSocketFD, res, strlen(res)) <= 0) removeClient(client);
    // strcpy(client->username, username);
    // strcpy(client->language, "IT");
    // return LOGIN_OK;
    // // RICORDARTI DI TOGLIERLO !!!!!!!!!!!!!!!!!!!!!!

    if(getLogin((const char*)username, dbpassword, language) != DB_OK){
        sprintf(res, "%d", LOG_USERNAME_NOT_EXISTS);
        if(write(client->acceptedSocketFD, res, strlen(res)) <= 0) removeClient(client);
        return LOG_USERNAME_NOT_EXISTS;
    }

    if(strcmp(password, dbpassword) == 0){
        sprintf(res, "%d", LOGIN_OK);
        if(write(client->acceptedSocketFD, res, strlen(res)) <= 0) removeClient(client);
        strcpy(client->username, username);
        strcpy(client->language, language);
        return LOGIN_OK;
    } else {
        sprintf(res, "%d", LOG_WRONG_PASSWORD);
        if(write(client->acceptedSocketFD, res, strlen(res)) <= 0) removeClient(client);
        return LOG_WRONG_PASSWORD; 
    }
}

int registerClient(struct AcceptedClient* client){
    int n;
    char username[1024];
    char password[1024];
    char langs[1024] = {0};
    char language[3];
    char res[1024];

    if((n = read(client->acceptedSocketFD, username, 1024)) <= 0) removeClient(client);
    username[n] = '\0';

    if((n = read(client->acceptedSocketFD, password, 1024)) <= 0) removeClient(client);
    password[n] = '\0';

    if(getLanguages(langs) != DB_OK) removeClient(client);

    if(write(client->acceptedSocketFD, langs, strlen(langs)) <= 0) removeClient(client);
    if((n = read(client->acceptedSocketFD, language, 3)) <= 0) removeClient(client);
    language[n] = '\0';


    if(insertUser((const char*)username, (const char*)password, (const char*)language) == DB_OK){
        sprintf(res, "%d", LOGIN_OK);
        if(write(client->acceptedSocketFD, res, strlen(res)) <= 0) removeClient(client);
        strcpy(client->username, username);
        strcpy(client->language, language);
        return LOGIN_OK;
    } else {
        sprintf(res, "%d", REG_USERNAME_ALREDY_EXISTS);
        if(write(client->acceptedSocketFD, res, strlen(res)) <= 0) removeClient(client);
        return REG_USERNAME_ALREDY_EXISTS; 
    }
}

