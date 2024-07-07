#include "room.h"
#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "ncursesUI.h"

#define BUFFER_SIZE 1024

extern WINDOW* stderrw;

void reqJoinRoom(int sockfd, int room){
    unsigned char command = (unsigned char)room;
    write(sockfd, &command, 1);
}

int waitInQueue(int sockfd){
    unsigned char cod;

    do{
        int n = read(sockfd, &cod, 1);
        if (n > 0){
            if (cod == 0){
                return 1;
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

int getLanguages(char*** languages, int sockfd){
    char buffer[BUFFER_SIZE];
    int r;
    if((r = read(sockfd, buffer, BUFFER_SIZE - 1)) <= 0) return -1;
    buffer[r] = '\0';

    (*languages) = malloc(sizeof(char *) * (r / 2));
    for(int i = 0; i < r / 2; i++){
        (*languages)[i] = malloc(sizeof(char) * 3);
        (*languages)[i][0] = buffer[2*i];
        (*languages)[i][1] = buffer[2*i + 1];
        (*languages)[i][2] = '\0';
    }


    return (r/2);
}

void freeLanguages(char* languages[], int n){
    for(int i = 0; i < n; i++){
        free(languages[i]);
    }

    free(languages);
}

int getRooms(char*** rooms, int sockfd){
    unsigned char roomn = 0, max, con;
    read(sockfd, &roomn, 1);
    (*rooms) = malloc(sizeof(char *) * (roomn + 2));
    for (int i = 0; i < roomn; i++){
        (*rooms)[i] = malloc(sizeof(char) * (MAX_ROOMNAME_LEN + 80));
        char lan[3];
        int n = read(sockfd, (*rooms)[i], MAX_ROOMNAME_LEN + 1);
        n = read(sockfd, lan, 3);
        n = read(sockfd, &con, 1);
        n = read(sockfd, &max, 1);
        for(int j = 0; j < strlen((*rooms)[i]) - 20; j++){
            strcat((*rooms)[i], " ");
        }
        strcat((*rooms)[i], " ");
        strcat((*rooms)[i], lan);
        sprintf((*rooms)[i], "%s\tConnected: %d\tMax: %d", (*rooms)[i], con, max);
    }
    (*rooms)[roomn + 1] = malloc(5);
    (*rooms)[roomn] = malloc(9);
    strcpy((*rooms)[roomn + 1], "Exit");
    strcpy((*rooms)[roomn], "Add Room");

    return roomn + 2;
}

void freeRooms(char** rooms, int n){
    for(int i = 0; i < n; i++){
        free(rooms[i]);
    }

    free(rooms);
}

int handleCommand(char* message){
    char command[10];
    char arg[1024];
    sscanf(message, "/%s %s", command, arg);

    if(strcmp(command, "exit") == 0){
        return 1;
    } else if(strcmp(command, "help") == 0){
        printerrmsg(stderrw, "Command List: /exit for exit, /help for help");
        return 2;
    }

    printerrmsg(stderrw, "Unknown Command: Type /help to get the list of valid commands");
    return 0;
}