#include "utils.h"
#include "customerr.h"
#include "database.h"
#include <stdlib.h>
#include <stdio.h>
#include "clients.h"
#include "rooms.h"
#include "passwordhash.h"

#define MAX_CLIENTS 10

struct AcceptedClient *clients[MAX_CLIENTS] = {NULL};
extern struct ChatRoom** rooms;
extern int curr_rooms;
extern int max_rooms;
extern pthread_rwlock_t rooms_global_lock;

int loggedClientCount = 0;

//  Socket

int createTCPSocket() {
    return socket(AF_INET,SOCK_STREAM,0);
}
    
struct sockaddr_in* createIPAddress(const char *ip, int port) {
    struct sockaddr_in *address = malloc(sizeof(struct sockaddr_in));
    address->sin_family = AF_INET;
    address->sin_port = htons(port);
    if(ip != NULL)
        inet_pton(AF_INET, ip, &address->sin_addr.s_addr);
    else
        address->sin_addr.s_addr = INADDR_ANY;
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

void setTimeoutOnSocket(int sock, int seconds){
    struct timeval timeout;
    timeout.tv_sec = seconds;  // Imposta il timeout a 10 secondi
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
}

//  Client Handler

void* manageNewConnection(void* args) {
    //login
    struct AcceptedClient* client = (struct AcceptedClient*)args;
    char buffer[1024];
    int n;

    client->username[0] = '\0';

    // Login Manager
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

    //  Rooms Manager
    while(1){
        int roomn = sendRooms(client);
        printf("stanze inviate\n");

        unsigned char ch;
        if((n = read(client->acceptedSocketFD, &ch, 1)) <= 0) removeClient(client);

        //  Room selection
        if(ch < roomn){

            //  Valid Room Enter
            pthread_rwlock_rdlock(&rooms_global_lock);
            struct ChatRoom* r = rooms[ch];
            pthread_rwlock_unlock(&rooms_global_lock);

            printf("%s requested room %s, %s\n", client->username, r->name, r->language);
            joinOrWaitForTheSelectedChatRoom(client, r);
            unsigned char msg = 0;
            if(write(client->acceptedSocketFD, &msg, 1) <= 0) removeClient(client);
                
            sprintf(buffer, "%s joined the room\n", client->username);
            printf("%s joined the room\n", client->username);
            sendMessageToTheChatRoom(r, buffer);
            manageChatRoom(client, r);
        }
        else if(ch == roomn){
            //  Create New Room
            char roomName[MAX_ROOMNAME_LEN + 1];
            char selectedLang[3];
            char langs[1024];
            int n;
            unsigned char max;

            printf("%s is creating new Room\n", client->username);

            if((n = read(client->acceptedSocketFD, roomName, MAX_ROOMNAME_LEN + 1)) <= 0) removeClient(client);
            roomName[n - 1] = '\0';

            if((n = read(client->acceptedSocketFD, &max, 1)) <= 0) removeClient(client);
            
            if(getLanguages(langs) != DB_OK) removeClient(client);
            if(write(client->acceptedSocketFD, langs, strlen(langs)) <= 0) removeClient(client);

            if((n = read(client->acceptedSocketFD, selectedLang, 2)) <= 0) removeClient(client);
            selectedLang[2] = '\0';

            addRoom((const char*)roomName, (const char*)selectedLang, max);
            
        } else {

            //  Exit
            removeClient(client);
        }
    }

    return NULL;
}

void removeClient(struct AcceptedClient *client){
    printf("%s disconnected\n", strlen(client->username) == 0 ? "Client" : client->username);

    if((client->chatRoom != NULL) && (client->isInQueue == 1)){
        pthread_mutex_lock(&(client->chatRoom->mutex));
        findAndRemove(&(client->chatRoom->waitingClients), client);
        pthread_mutex_unlock(&(client->chatRoom->mutex));
    } else if(client->chatRoom != NULL){
        printf("Faccio uscire dalla stanza\n");
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
    char username[MAX_USERNAME_LEN + 1];
    char dbpassword[HASH_SIZE + 1];
    char password[MAX_USERNAME_LEN + 1];
    char language[5];
    unsigned char res;

    if((n = read(client->acceptedSocketFD, username, MAX_USERNAME_LEN + 1)) <= 0) removeClient(client);
    
    if((n = read(client->acceptedSocketFD, password, MAX_USERNAME_LEN + 1)) <= 0) removeClient(client);

    // // DÀ SEMPRE OK, SOLO PER TESTING !!!!!!!!!!!!!!!
    // sprintf(res, "%d", LOGIN_OK);
    // if(write(client->acceptedSocketFD, res, strlen(res)) <= 0) removeClient(client);
    // strcpy(client->username, username);
    // strcpy(client->language, "IT");
    // return LOGIN_OK;
    // // RICORDARTI DI TOGLIERLO !!!!!!!!!!!!!!!!!!!!!!

    if(getLogin((const char*)username, dbpassword, language) != DB_OK){
        res = (unsigned char)LOG_USERNAME_NOT_EXISTS;
        if(write(client->acceptedSocketFD, &res, 1) <= 0) removeClient(client);
        return LOG_USERNAME_NOT_EXISTS;
    }

    if(verify_password(password, dbpassword) == 0){
        res = (unsigned char)LOGIN_OK;
        if(write(client->acceptedSocketFD, &res, 1) <= 0) removeClient(client);
        strcpy(client->username, username);
        strcpy(client->language, language);
        return LOGIN_OK;
    } else {
        res = (unsigned char)LOG_WRONG_PASSWORD;
        if(write(client->acceptedSocketFD, &res, 1) <= 0) removeClient(client);
        return LOG_WRONG_PASSWORD; 
    }
}

int registerClient(struct AcceptedClient* client){
    int n;
    char username[MAX_USERNAME_LEN + 1];
    char password[MAX_USERNAME_LEN + 1];
    char langs[1024] = {0};
    char language[3];
    unsigned char res;

    if((n = read(client->acceptedSocketFD, username, MAX_USERNAME_LEN + 1)) <= 0) removeClient(client);

    if((n = read(client->acceptedSocketFD, password, MAX_USERNAME_LEN + 1)) <= 0) removeClient(client);

    char hashed[HASH_SIZE + 1];
    hash_password(password, hashed);

    if(getLanguages(langs) != DB_OK) removeClient(client);

    if(write(client->acceptedSocketFD, langs, strlen(langs)) <= 0) removeClient(client);
    if((n = read(client->acceptedSocketFD, language, 3)) <= 0) removeClient(client);
    language[n - 1] = '\0';

    if(insertUser((const char*)username, (const char*)hashed, (const char*)language) == DB_OK){
        res = (unsigned char)LOGIN_OK;
        if(write(client->acceptedSocketFD, &res, 1) <= 0) removeClient(client);
        strcpy(client->username, username);
        strcpy(client->language, language);
        return LOGIN_OK;
    } else {
        res = (unsigned char)REG_USERNAME_ALREDY_EXISTS;
        if(write(client->acceptedSocketFD, &res, 1) <= 0) removeClient(client);
        return REG_USERNAME_ALREDY_EXISTS; 
    }
}
