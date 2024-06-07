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

#define MIN_ROW 10
#define MIN_COL 20

struct client{
    char username[20];
    int language;
    int room;
};

int sockfd;
WINDOW* stderrw;
WINDOW* mainw;

void clearExit(){
    endwin();
    close(sockfd);
    exit(0);
}

void set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void toggle_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    flags &= (~O_NONBLOCK);
    fcntl(fd, F_SETFL, flags);
}


int main(){
    struct sockaddr_in servaddr;
    int row, col;
    struct client client;

    // Create socket
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    if(inet_pton(AF_INET, SERVER_IP, &servaddr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to server
    if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Connection Failed");
        exit(EXIT_FAILURE);
    }

    initscr();
    getmaxyx(stdscr, row, col);
    keypad(stdscr, TRUE);

    if(row < MIN_ROW || col < MIN_COL){
        close(sockfd);
        endwin();
        printf("Please enlarge your terminal window\n");
        exit(1);
    }


    mainw = newwin(row - 1, col, 0, 0);
    stderrw = newwin(1, col, row - 1, 0);

    //  Login o Registrazione
    WINDOW* loginMenu = newwin(3, col, 0, 0);

    
    const char* chv[] = {"Login", "Register", "Exit"};
    bool isLogged = false;

    do{
        int selected = wmenu(loginMenu, 3, chv);
        switch(selected){
        case 0:{
            //  Login
            char username[1024];
            char password[1024];
            char buffer[BUFFER_SIZE];
            int res;

            getUsername(username, 0);
            write(sockfd, username, strlen(username));
            getPassword(password, 1);
            write(sockfd, password, strlen(password));
            clear();
            refresh();

            int r = read(sockfd, buffer, BUFFER_SIZE - 1);
            sscanf(buffer, "%d", &res);
            if(res > 0){
                loginErrorHandler(res);
            } else {
                isLogged = true;
                strcpy(client.username, username);
            }

            break; 
        }
        case 1:{
            char username[1024];
            char password[1024];
            char confirmp[1024];
            char buffer[1024];
            int res;

            getUsername(username, 0);
            write(sockfd, username, strlen(username));
            getPassword(password, 1);
            getConfirm(confirmp, 2, password);
            write(sockfd, password, strlen(password));
            clear();
            refresh();

            int r = read(sockfd, buffer, BUFFER_SIZE - 1);
            sscanf(buffer, "%d", &res);
            if(res > 0){
                loginErrorHandler(res);
            } else {
                isLogged = true;
                strcpy(client.username, username);
            }
            //  Register
            break;
        }
        default:
            //  Exit
            clearExit();
            break;
        }

    }while(!isLogged);

    clear();
    refresh();

    WINDOW* roomsMenu = newwin(5, col, 0, 0);

    do{
        const char *rooms[5] = {"ITA", "ENG", "FRA", "ESP", "Exit"};
        int selected = wmenu(roomsMenu, 5, rooms);
        if(selected >= 0 && selected < 4){
            char command[BUFFER_SIZE];
            sprintf(command, "/r %d", selected);
            write(sockfd, command, strlen(command));
            client.room = selected;
        } else {
            clearExit();
        }

        clear();
        refresh();

        set_nonblocking(sockfd);
        WINDOW *inputwin = newwin(3, col, row - 4, 0);
        WINDOW *messagesboxwin = newwin(row - 6, col, 0, 0);
        WINDOW *messageswin = newwin(row - 8, col - 2, 1, 1);

        nodelay(inputwin, TRUE);
        scrollok(messageswin, TRUE);
        wsetscrreg(messageswin, 0, getmaxy(messageswin));
        keypad(stdscr, TRUE);
        keypad(inputwin, TRUE);
        refresh();

        box(inputwin, 0, 0);
        box(messagesboxwin, 0, 0);
        refresh();
        wmove(inputwin, 1, 1);
        leaveok(stdscr, TRUE);
        
        wrefresh(inputwin);
        wrefresh(messagesboxwin);
        wrefresh(messageswin);

        char message_buffer[BUFFER_SIZE] = {0};
        char input_buffer[BUFFER_SIZE] = {0};
        int input_len = 0;
        int currentLine = 0;

        do{
            //  Read String char by char Non-Blocking Way
            int ch = wgetch(inputwin);
            if(ch != ERR){
                if(ch == '\n' || ch == KEY_ENTER){
                    write(sockfd, input_buffer, strlen(input_buffer));
                    if(strcmp(input_buffer, "/exit") == 0){
                        client.room = -1;
                        break;
                    }

                    memset(input_buffer, 0, BUFFER_SIZE);
                    input_len = 0;
                } else if (ch == '\b' || ch == KEY_BACKSPACE || ch == 127){
                    if (input_len > 0) {
                        input_buffer[--input_len] = '\0';
                    }
                } else if (isprint(ch)){
                    input_buffer[input_len++] = ch;
                    input_buffer[input_len] = '\0';
                }
            }

            // Read messages from server
            int n = read(sockfd, message_buffer, sizeof(message_buffer) - 1);
            if (n > 0) {
                message_buffer[n] = '\0';
                if(currentLine == getmaxy(messageswin)) currentLine--;
                mvwprintw(messageswin, currentLine, 0, "%s", message_buffer);
                currentLine++;
                wrefresh(messageswin);
            }

            // Display input buffer
            wclear(inputwin);
            box(inputwin, 0, 0);
            mvwprintw(inputwin, 1, 1, "%s", input_buffer);
            usleep(5);
            wrefresh(inputwin);
        } while(1);

        clear();
        delwin(inputwin);
        delwin(messageswin);
        delwin(messagesboxwin);
        refresh();
        toggle_nonblocking(sockfd);
    } while(1);
    //  Lista stanze
    //  Apertura chat

    endwin();
    close(sockfd);

    return 0;
}