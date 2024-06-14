#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>


#define BUFFER_SIZE 1024

void reqJoinRoom(int sockfd, int room){
    char command[BUFFER_SIZE];
    sprintf(command, "%d", room);
    write(sockfd, command, strlen(command));
}

int waitInQueue(int sockfd){
    char res[BUFFER_SIZE];
    int cod;

    do{
        int n = read(sockfd, res, BUFFER_SIZE);
        if (n > 0){
            sscanf(res, "%d", &cod);
            if (cod == 0){
                break;
            }

            mvprintw(0, 0, "You are the %d in line", cod);
            refresh();
        }
        else{
            return n;
        }
    } while (1);

    return 0;
}