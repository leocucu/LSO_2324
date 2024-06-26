#ifndef LOGIN_H
#define LOGIN_H

#include <ncurses.h>
#include "../customerr.h"

bool validateUsername(char* username, char* errormsg);
bool validatePassword(char* password, char* errormsg);

int getUsername(char* out, int row);
int getPassword(char* out, int row);
int getConfirm(char* out, int row, char* password);
const char* loginErrorStr(LoginError code);

#endif