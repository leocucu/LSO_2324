#include "login.h"
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "ncursesUI.h"

#define MAX_ATTEMPTS 5

extern WINDOW* stderrw;

bool validateUsername(char* username, char* errormsg){
    int len = strlen(username);
    if(len < 5){
        char msg[] = "Min Username length: 5 character";
        errormsg != NULL && strcpy(errormsg, msg);
        return false;
    } else if(len > 20){
        char msg[] = "Max Username length: 20 character";
        errormsg != NULL && strcpy(errormsg, msg);
        return false;
    }


    for(int i = 0; i < len; i++){
        if(!isalnum(username[i])){
            char msg[] = "Username should only contain Alphanumerical characters";
            errormsg != NULL && strcpy(errormsg, msg);
            return false;
        }
    }

    return true;
}

bool validatePassword(char* password, char* errormsg){
    int len = strlen(password);
    if(len < 5){
        char msg[] = "Min password length: 5 character";
        errormsg != NULL && strcpy(errormsg, msg);
        return false;
    } else if(len > 20){
        char msg[] = "Max password length: 20 character";
        errormsg != NULL && strcpy(errormsg, msg);
        return false;
    }


    for(int i = 0; i < len; i++){
        if(!isalnum(password[i])){
            char msg[] = "Password should only contain Alphanumerical characters";
            errormsg != NULL && strcpy(errormsg, msg);
            return false;
        }
    }

    return true;
}

bool validateConfirm(char* password1, char*  password2, char* errormsg){
    if(strcmp(password1, password2) != 0){
        char msg[] = "Password Does Not Match";
        errormsg != NULL && strcpy(errormsg, msg);

        return false;
    }

    return true;
}


int getUsername(char* out, int row){
    echo();
    char username[MAX_USERNAME_LEN + 1];
    char errormsg[50];
    int len;

    do{
        move(row, 0);
        clrtoeol();
        printw("Username: ");
        len = getAlnumString(username, MAX_USERNAME_LEN, 0);
        if(len == 0){
            printerrmsg(stderrw, "Empty Username");
        }else if (validatePassword(username, errormsg)){
            break;
        } else {
            printerrmsg(stderrw, errormsg);
        }

    } while (1);

    stderrw != NULL && wclear(stderrw);
    wrefresh(stderrw);
    strcpy(out, username);

    return len;
}

int getPassword(char* out, int row){
    char password[MAX_USERNAME_LEN + 1];
    char errormsg[50];
    int i = 0;
    int len;

    noecho();
    for(i = 0; i < MAX_ATTEMPTS; i++){
        move(row, 0);
        clrtoeol();
        printw("Password: ");
        len = getAlnumString(password, MAX_USERNAME_LEN, '*');
        if(len == 0){
            printerrmsg(stderrw, "Empty Password");
        }else if (validatePassword(password, errormsg)){
            break;
        } else {
            printerrmsg(stderrw, errormsg);
        }
    }

    if(i == MAX_ATTEMPTS) {
        return -1;
    }

    stderrw != NULL && wclear(stderrw);
    wrefresh(stderrw);
    strcpy(out, password);
    echo();
    return len;

}

int getConfirm(char* out, int row, char* password){
    char confirm[MAX_USERNAME_LEN + 1];
    char errormsg[50];
    int i = 0;

    noecho();
    for(i = 0; i < MAX_ATTEMPTS; i++){
        move(row, 0);
        clrtoeol();
        printw("Confirm Password: ");
        int len = getAlnumString(confirm, MAX_USERNAME_LEN, '*');
        if(len == 0){
            printerrmsg(stderrw, "Empty Password");
        }else if (validateConfirm(password, confirm, errormsg)){
            break;
        } else {
            printerrmsg(stderrw, errormsg);
        }
    }

    if(i == MAX_ATTEMPTS) {
        return -1;
    }

    stderrw != NULL && wclear(stderrw);
    wrefresh(stderrw);
    strcpy(out, confirm);
    echo();

    return 0;
}

const char* loginErrorStr(LoginError code){
    switch (code) {
    case LOG_USERNAME_NOT_EXISTS:{
        return "Username not found";
        break;
    }
    case LOG_WRONG_PASSWORD:{
        return "Wrong Password";
        break;
    }
    case REG_USERNAME_ALREDY_EXISTS:{
        return "Username already exists";
        break;
    }
    default:
        break;
    }

    return "Login Error";
}