#include "utils.h"
#include "rooms.h"
#include "database.h"
#include <stdlib.h>

#define PORT 12345
#define IPADDR "127.0.0.1"

int main(){
    //  Inizializzazione DB
    if(connectdb() != DB_OK){
        printf("Failed to connect to DB\n");
        exit(1);
    }
    //  Creazione e Bind socket
    int serverSocketFD = createTCPSocket();
    struct sockaddr_in *serverAddress = createIPAddress(IPADDR, PORT);
    if (bind(serverSocketFD, (struct sockaddr*)serverAddress,sizeof(*serverAddress)) < 0) {
        perror("bind");
        exit(1);
    }
    printf("Socket was bound successfully\n");
    
    listen(serverSocketFD,10);
    initRooms();
    startAcceptingIncomingConnection(serverSocketFD);
    closedb();
    close(serverSocketFD);
    shutdown(serverSocketFD,SHUT_RDWR);

    return 0;

}
