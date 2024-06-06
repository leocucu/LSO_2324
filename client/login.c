#include "login.h"
#include "ncursesUI.h"
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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


void getUsername(char* out, int row){
    echo();
    char username[1024];
    char errormsg[50];

    mvprintw(5, 0, "%d", is_keypad(stdscr));

    do{
        move(row, 0);
        clrtoeol();
        printw("Username: ");
        int res = scanw("%s", username);
        if(res == 0 || res == EOF){
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
}

void getPassword(char* out, int row){
    char password[1024];
    char errormsg[50];
    int i = 0;

    noecho();
    for(i = 0; i < MAX_ATTEMPTS; i++){
        move(row, 0);
        clrtoeol();
        printw("Password: ");
        int res = scanw("%s", password);
        if(res == 0 || res == EOF){
            printerrmsg(stderrw, "Empty Password");
        }else if (validatePassword(password, errormsg)){
            break;
        } else {
            printerrmsg(stderrw, errormsg);
        }
    }

    if(i == MAX_ATTEMPTS) {
        endwin();
        printf("Max attempts reached\n");
        exit(1);
    }

    stderrw != NULL && wclear(stderrw);
    wrefresh(stderrw);
    strcpy(out, password);
    echo();

}

void getConfirm(char* out, int row, char* password){
    char confirm[1024];
    char errormsg[50];
    int i = 0;

    noecho();
    for(i = 0; i < MAX_ATTEMPTS; i++){
        move(row, 0);
        clrtoeol();
        printw("Confirm Password: ");
        int res = scanw("%s", confirm);
        if(res == 0 || res == EOF){
            printerrmsg(stderrw, "Empty Password");
        }else if (validateConfirm(password, confirm, errormsg)){
            break;
        } else {
            printerrmsg(stderrw, errormsg);
        }
    }

    if(i < MAX_ATTEMPTS) {
        endwin();
        printf("Max attempts reached\n");
        exit(1);
    }

    stderrw != NULL && wclear(stderrw);
    wrefresh(stderrw);
    strcpy(out, confirm);
    echo();

}