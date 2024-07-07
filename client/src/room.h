#ifndef ROOM_H
#define ROOM_H

#define MAX_ROOMNAME_LEN 20

void reqJoinRoom(int sockfd, int room);
int waitInQueue(int sockfd);
int getLanguages(char*** languages, int sockfd);
void freeLanguages(char* languages[], int n);
int getRooms(char*** rooms, int sockfd);
void freeRooms(char** rooms, int n);
int handleCommand(char* message);

#endif