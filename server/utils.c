#include "utils.h"

int createTCPSocket() {
    return socket(AF_INET,SOCK_STREAM,0);
}
    
struct sockaddr_in* createIPAddress(char *ip, int port) {
    struct sockaddr_in *address = malloc(sizeof(struct sockaddr_in));
    address->sin_family = AF_INET;
    address->sin_port = htons(port);
    inet_pton(AF_INET, ip, &address->sin_addr.s_addr);
    return address;
}

struct AcceptedClient* acceptIncomingConnection(int serverSocketFD) {
    struct sockaddr_in clientAddress;
    int clientAddressSize = sizeof(struct sockaddr_in);
    int clientSocketFD = accept(serverSocketFD, &clientAddress, &clientAddressSize);

    struct AcceptedClient* acceptedSocket = malloc(sizeof(struct AcceptedClient));
    acceptedSocket->address = clientAddress;
    acceptedSocket->acceptedSocketFD = clientSocketFD;
    acceptedSocket->acceptedSuccessfully = clientSocketFD > 0;
    acceptedSocket->ID_chatRoom = NULL;
    acceptedSocket->language = NULL;
    return acceptedSocket;
}

void startAcceptingIncomingConnection(int serverSocketFD) {
    while (1) {
        struct AcceptedSocket *clientSocket = acceptIncomingConnection(serverSocketFD);
        manageNewConnectionOnSeparateThread(clientSocket);
    }
}

void manageNewConnectionOnSeparateThread(struct AcceptedClient *client) {
    pthread_t id;
    pthread_create(&id,NULL,manageNewConnection,&client);
}

void manageNewConnection(struct AcceptedClient **client) {
    //login
    loggedClient[loggedClientCount++] = *client;
    //accesso alla stanza
    backEndPrompt(client);
    //chatRoom
}

void sendMessageToTheChatRoom(struct AcceptedClient *client, char *message) {
    for (int i = 0; i < loggedClientCount; i++) {
        if (loggedClient[i]->acceptedSocketFD != client->acceptedSocketFD) {
            if (loggedClient[i]->ID_chatRoom == client->ID_chatRoom) {
                send(loggedClient[i]->acceptedSocketFD,message,strlen(message),0);
            }
        }
    }
}

void backEndPrompt(struct AcceptedClient **client) {
    char buffer[8];
    const char error[] = "Command not found";
    while (1) {
        recv((*client)->acceptedSocketFD,buffer,8,0);
        if (strcmp(buffer, "\\e")) {
            close((*client)->acceptedSocketFD);
            free((*client)->address);
            free(*client);
            break;
        }
        else {
            //find in the db
            joinOrWaitForTheSelectedChatRoom(*client);
        }
    }
}

void manageChatRoom(struct AcceptedClient *client) {
    char buffer[1024];
    while(1) {
        int amountReceived = recv(client->acceptedSocketFD, buffer,1024,0);
        if (amountReceived < 0) {
            break;
        }
        if (strcmp(buffer,"\\exit")) {
            client->ID_chatRoom = NULL;
            //free thread in the queue
            break;
        }
        char **traslatedMessage = traslateMessage(buffer, client->ID_chatRoom->language);
        sendMessageToTheChatRoom(client,*traslatedMessage);
        free(*traslatedMessage);
        free(traslateMessage);
    }
}

char **traslateMessage(char *message, char *language) {
    char* token;
    char* rest = message;
    char** traslatedMessage = malloc(sizeof(char*));
    *traslatedMessage = malloc(sizeof(char) * 1024);
    
    while ((token = strtok_r(rest, " ", &rest))) {
        //traslate token
        strcat(*traslatedMessage, token);
        strcat(*traslatedMessage, " ");
    }
    return traslatedMessage;
}

void joinOrWaitForTheSelectedChatRoom(struct AcceptedClient **client) {
    pthread_mutex_lock(&((*client)->ID_chatRoom->mutex));
    if ((*client)->ID_chatRoom->maxClient > 0) {
        (*client)->ID_chatRoom->maxClient = (*client)->ID_chatRoom->maxClient - 1;
        pthread_mutex_unlock(&((*client)->ID_chatRoom->mutex));
    }
    else {
        while ((*client)->ID_chatRoom->maxClient < 0) {
            enqueue((*client)->ID_chatRoom->waitingClients,*client);
            pthread_cond_wait(&((*client)->ID_chatRoom->cond), &((*client)->ID_chatRoom->mutex));
        }
        *client = (struct AcceptedClient*) dequeue((*client)->ID_chatRoom->waitingClients);
        (*client)->ID_chatRoom->maxClient = (*client)->ID_chatRoom->maxClient + 1;
        pthread_mutex_unlock(&((*client)->ID_chatRoom->mutex));
    }
    manageChatRoom(*client);
}