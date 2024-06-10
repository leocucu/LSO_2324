#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>

//  Array di client
struct AcceptedClient{
    int acceptedSocketFD;
    struct sockaddr_in address;
    int acceptedSuccessfully;
    char language;
    char *chatRoom;
};

struct AcceptedClient loggedClient[10];
int loggedClientCount = 0;

//utils 
int createTCPSocket();
struct sockaddr_in* createIPAddress(char *ip, int port);
struct AcceptedClient* acceptIncomingConnection(int serverSocketFD);
void startAcceptingIncomingConnection(int serverSocketFD);
void manageNewConnectionOnSeparateThread(struct AcceptedClient *client);
void manageNewConnection(struct AcceptedClient *client);
void sendMessageToTheChatRoom(struct AcceptedClient client, char *message);
void backEndPrompt(struct AcceptedClient *client);
void chatRoom(struct AcceptedClient *client);
char *traslateMessage(char *message, char *chatRoom);

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
    pthread_create(&id,NULL,manageNewConnection,client);
}

void manageNewConnection(struct AcceptedClient *client) {
    //login
    loggedClient[loggedClientCount++] = *client;
    //accesso alla stanza
    backEndPrompt(client);
    //chatRoom
}

void sendMessageToTheChatRoom(struct AcceptedClient client, char *message) {
    for (int i = 0; i < loggedClientCount; i++) {
        if (loggedClient[i].acceptedSocketFD != client.acceptedSocketFD) {
            if (strcmp(loggedClient->chatRoom, client.chatRoom)) {
                send(loggedClient->acceptedSocketFD,message,strlen(message),0);
            }
        }
    }
}

void backEndPrompt(struct AcceptedClient *client) {
    char buffer[8];
    const char error[] = "Command not found";
    while (1) {
        recv(client->acceptedSocketFD,buffer,8,0);
        if (strcmp(buffer, "\\exit")) {
            break;
        }
        else if (strcmp(buffer,"\\c ENG-ITA")) {
            strcat(client->chatRoom,"ENG-ITA");
            chatRoom(client);
        }
        else if (strcmp(buffer,"\\c ITA-ENG")) {
            strcat(client->chatRoom,"ITA-ENG");
            chatRoom(client);
        }
        else if (strcmp(buffer,"\\c LSO-CHATROOM")) {
            strcat(client->chatRoom,"LSO-CHAT");
            chatRoom(client);
        }
        else {
            sent(error,sizeof(error),0);
        }
    }
}

void chatRoom(struct AcceptedClient *client) {
    char buffer[1024];
    while(1) {
        int amountReceived = recv(client->acceptedSocketFD, buffer,1024,0);
        if (amountReceived < 0) {
            break;
        }
        if (strcmp(buffer,"\\exit")) {
            client->chatRoom = "";
            break;
        }
        char *traslatedMessage = traslateMessage(buffer, client->chatRoom);
        sendMessageToTheChatRoom(*client,traslatedMessage);
    }
}

//  Mutex per i client

int main(){
    //  Inizializzazione DB

    //  Creazione e Bind socket
    int serverSocketFD = createTCPSocket();
    struct sockaddr_in *serverAddress = createIPAddress("127.0.0.1", 2000);
    int result = bind(serverSocketFD,serverAddress,sizeof(*serverAddress));
    if (result == 0) {
        printf("Socket was bound successfully\n");
    }
    listen(serverSocketFD,10);

    //  Inizio ciclo:
    //  accettazione del client
    startAcceptingIncomingConnection(serverSocketFD);
    //  creazione del thread per gestire il client

    close(serverSocketFD);
    shutdown(serverSocketFD,SHUT_RDWR);

    return 0;
}
