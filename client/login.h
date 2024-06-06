#ifndef LOGIN_H
#define LOGIN_H

#include <ncurses.h>

bool validateUsername(char* username, char* errormsg);
bool validatePassword(char* password, char* errormsg);

void getUsername(char* out, int row);
void getPassword(char* out, int row);
void getConfirm(char* out, int row, char* password);

#endif