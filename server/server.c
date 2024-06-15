#include "utils.h"

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
    //  create the chatRoom, initialize the mutexs and the conds
    //  Inizio ciclo:
    //  accettazione del client
    startAcceptingIncomingConnection(serverSocketFD);
    //  creazione del thread per gestire il client
    //  destroy mutexs and conds
    close(serverSocketFD);
    shutdown(serverSocketFD,SHUT_RDWR);

    return 0;
}
