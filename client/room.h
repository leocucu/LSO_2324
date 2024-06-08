#ifndef ROOM_H
#define ROOM_H

void reqJoinRoom(int sockfd, int room);
int waitInQueue(int sockfd);

#endif