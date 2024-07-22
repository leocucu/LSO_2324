#include "rooms.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include "database.h"
#include "utils.h"

int curr_rooms = 0;
int max_rooms = 20;
struct ChatRoom **rooms;
pthread_rwlock_t rooms_global_lock = PTHREAD_RWLOCK_INITIALIZER;

//  Rooms
void initRooms(){
    if(getRoomsCount(&curr_rooms) != DB_OK) exit(EXIT_FAILURE);
    max_rooms = (int)(curr_rooms * 1.5);
    rooms = malloc(sizeof(struct ChatRoom*) * max_rooms);

    char* names[curr_rooms];
    char* languages[curr_rooms];
    int max[curr_rooms];

    for(int i = 0; i < curr_rooms; i++){
        names[i] = malloc(sizeof(char) * (MAX_ROOMNAME_LEN + 1));
        languages[i] = malloc(sizeof(char) * 3);
    }

    if(getRooms(languages, names, max) != DB_OK) exit(EXIT_FAILURE);

    for(int i = 0; i < curr_rooms; i++){
        rooms[i] = malloc(sizeof(struct ChatRoom));
        rooms[i]->connected = 0;
        rooms[i]->maxClient = max[i];
        strcpy(rooms[i]->language, languages[i]);
        strcpy(rooms[i]->name, names[i]);

        rooms[i]->clients = malloc(sizeof(struct AcceptedClient*) * rooms[i]->maxClient);
        for(int j = 0; j < rooms[i]->maxClient; j++ ){
            rooms[i]->clients[j] = NULL;
        }
        initq(&(rooms[i]->waitingClients));

        pthread_mutex_init(&(rooms[i]->mutex), NULL);
        pthread_cond_init(&(rooms[i]->cond), NULL);
        printf("Room %s %s, max: %d, connected: %d\n", rooms[i]->name, rooms[i]->language, rooms[i]->maxClient, rooms[i]->connected);
    }

    for(int i = 0; i < curr_rooms; i++){
        free(names[i]);
        free(languages[i]);
    }
}

int joinOrWaitForTheSelectedChatRoom(struct AcceptedClient *client, struct ChatRoom *r) {
    pthread_mutex_lock(&(r->mutex));
    char buffer[1024];
    printf("%s required access to room %s\n", client->username, r->name);
    client->chatRoom = r;
    if (r->connected < r->maxClient && r->waitingClients.head == NULL) {
        joinRoom(client, r);
    }
    else {
        printf("%s is waiting in queue\n", client->username);
        client->isInQueue = 1;
        enqueue(&(r->waitingClients), client);
        unsigned char turn = (unsigned char)r->waitingClients.size;
        while (1) {
            if(r->connected < r->maxClient && (struct AcceptedClient*)r->waitingClients.tail->item == client){
                joinRoom(client, r);
                dequeue(&(r->waitingClients));
                client->isInQueue = 0;
                break;
            }
            write(client->acceptedSocketFD, &turn, 1);
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

void addRoom(const char* name, const char* language, int maxClient){
    pthread_rwlock_wrlock(&rooms_global_lock);
    if(curr_rooms >= max_rooms){
        max_rooms *= 2;
        void *ptr = realloc(rooms, sizeof(struct ChatRoom*) * max_rooms);
    }
    rooms[curr_rooms] = malloc(sizeof(struct ChatRoom));
    rooms[curr_rooms]->connected = 0;
    rooms[curr_rooms]->maxClient = maxClient;
    strcpy(rooms[curr_rooms]->language, language);
    strcpy(rooms[curr_rooms]->name, name);
    rooms[curr_rooms]->clients = malloc(sizeof(struct AcceptedClient *) * rooms[curr_rooms]->maxClient);

    for (int j = 0; j < rooms[curr_rooms]->maxClient; j++){
        rooms[curr_rooms]->clients[j] = NULL;
    }
    initq(&(rooms[curr_rooms]->waitingClients));

    pthread_mutex_init(&(rooms[curr_rooms]->mutex), NULL);
    pthread_cond_init(&(rooms[curr_rooms]->cond), NULL);
    printf("Added room %s %s, max: %d, connected: %d\n", rooms[curr_rooms]->name, rooms[curr_rooms]->language, rooms[curr_rooms]->maxClient, rooms[curr_rooms]->connected);
    curr_rooms++;

    insertRoom(name, language, maxClient);

    pthread_rwlock_unlock(&rooms_global_lock);
}

int sendRooms(struct AcceptedClient* client){
    pthread_rwlock_rdlock(&rooms_global_lock);
    int n = curr_rooms;
    char buffer[MAX_ROOMNAME_LEN + 1];
    unsigned char number = (unsigned char)curr_rooms;
    unsigned char max, con;
    write(client->acceptedSocketFD, &number, 1);
    for(int i = 0; i < n; i++){
        strcpy(buffer, rooms[i]->name);
        write(client->acceptedSocketFD, buffer, MAX_ROOMNAME_LEN + 1);
        write(client->acceptedSocketFD, rooms[i]->language, 3);
        con = (unsigned char)rooms[i]->connected;
        max = (unsigned char)rooms[i]->maxClient;
        write(client->acceptedSocketFD, &con, 1);
        write(client->acceptedSocketFD, &max, 1);
    }

    pthread_rwlock_unlock(&rooms_global_lock);
    return n;
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

    setTimeoutOnSocket(client->acceptedSocketFD, 30);
    while (1){
        if ((n = read(client->acceptedSocketFD, buffer, 1024)) <= 0){
            removeClient(client);
        }
        char messaggio[2048];
        buffer[n] = '\0';
        if(buffer[0] == '/'){
            if (strcmp(buffer, "/exit") == 0){
                sprintf(messaggio, "%s left room\n", client->username);
                setTimeoutOnSocket(client->acceptedSocketFD, 0);
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
    char* rest2;
    strcpy(translated, "");
    
    while ((token = strtok_r(rest, " '", &rest))) {
        //traslate token

        if(getTranslation((const char*)token, translTok, lan1, lan2) != DB_OK)
            strcpy(translTok, token);
        strcat(translated, translTok);
        strcat(translated, " ");
    }
}