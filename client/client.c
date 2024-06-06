#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <ctype.h>

#include <ncurses.h>

#include "ncursesUI.h"
#include "login.h"

#define PORT 12345
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 1024

int sockfd;
WINDOW* stderrw;

void clearExit(){
    endwin();
    close(sockfd);
    exit(0);
}

int main(){
    struct sockaddr_in servaddr;
    int row, col;

    // // Create socket
    // if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    //     perror("socket creation failed");
    //     exit(EXIT_FAILURE);
    // }

    // servaddr.sin_family = AF_INET;
    // servaddr.sin_port = htons(PORT);
    // if(inet_pton(AF_INET, SERVER_IP, &servaddr.sin_addr) <= 0) {
    //     perror("Invalid address/ Address not supported");
    //     exit(EXIT_FAILURE);
    // }

    // // Connect to server
    // if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    //     perror("Connection Failed");
    //     exit(EXIT_FAILURE);
    // }

    initscr();
    getmaxyx(stdscr, row, col);

    stderrw = newwin(1, col, row - 1, 0);

    //  Login o Registrazione
    WINDOW* loginMenu = newwin(3, col, 0, 0);
    
    const char* chv[] = {"Login", "Register", "Exit"};
    int isLogged = 1;

    do{
        int selected = wmenu(loginMenu, 3, chv);
        switch(selected){
        case 0:{
            //  Login
            char username[1024];
            char password[1024];

            getUsername(username, 0);
            getPassword(password, 1);

            break; 
        }
        case 1:{
            char username[1024];
            char password[1024];
            char confirmp[1024];

            getUsername(username, 0);
            getPassword(password, 1);
            getConfirm(confirmp, 2, password);
            //  Register
            break;
        }
        default:
            //  Exit
            clearExit();
            break;
        }
    }while(!isLogged);

    //  Inizio del ciclo:
    //  Lista stanze
    //  Apertura chat

    endwin();
    close(sockfd);

    return 0;
}