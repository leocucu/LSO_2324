#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>
#include <locale.h>

#include <ncurses.h>

#include "ncursesUI.h"
#include "login.h"
#include "room.h"
#include "chat.h"

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

void cleanExit();
void cleanExitp(const char* message);

void cleanExit(){
    char buf[BUFFER_SIZE];
    clear();
    refresh();
    endwin();
    close(sockfd);
    exit(0);
}

void cleanExitp(const char* message){
    clear();
    refresh();
    endwin();
    close(sockfd);
    printf("%s\n", message);
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

    setlocale(LC_ALL, "");

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
        char command[1];
        command[0] = (char)(selected + '0');
        if(write(sockfd, command, 1) <= 0) cleanExit();
        switch(selected){
        case 0:{
            //  Login
            char username[1024];
            char password[1024];
            char buffer[BUFFER_SIZE];
            int res;

            getUsername(username, 0);
            if(write(sockfd, username, strlen(username)) <= 0) cleanExit();
            if(getPassword(password, 1) < 0){
                cleanExitp("Max attempts reached");
            }
            if(write(sockfd, password, strlen(password)) <= 0) cleanExit();
            clear();
            refresh();

            int r;
            if((r = read(sockfd, buffer, BUFFER_SIZE - 1)) <= 0) cleanExit();
            buffer[r] = '\0';
            sscanf(buffer, "%d", &res);
            if(res > 0){
                printerrmsg(stderrw, loginErrorStr(res));
            } else {
                isLogged = true;
                strcpy(client.username, username);
            }

            break; 
        }
        case 1:{
            //  Register
            char username[1024];
            char password[1024];
            char confirmp[1024];
            char buffer[1024];
            int res;

            getUsername(username, 0);
            if(write(sockfd, username, strlen(username)) <= 0) cleanExit();
            if(getPassword(password, 1) < 0){
                cleanExitp("Max attempts reached");
            }
            if(getConfirm(confirmp, 2, password) < 0){
                cleanExitp("Max attempts reached");
            }
            if(write(sockfd, password, strlen(password)) <= 0) cleanExit();
            int r;

            if((r = read(sockfd, buffer, BUFFER_SIZE - 1)) <= 0) cleanExit();
            buffer[r] = '\0';
            char* languages[r / 2];
            for(int i = 0; i < r / 2; i++){
                languages[i] = malloc(sizeof(char) * 3);
                languages[i][0] = buffer[2*i];
                languages[i][1] = buffer[2*i + 1];
                languages[i][2] = '\0';
            }
            mvprintw(3, 0, "Select your Language:");
            WINDOW* menu = newwin(row-4, col, 4, 0);
            wrefresh(menu);
            refresh();
            int ch = wmenu(menu, r / 2, (const char **) languages);
            if(write(sockfd, languages[ch], strlen(languages[ch])) <= 0) cleanExit();
            clear();
            refresh();
            for(int i = 0; i < r / 2; i++){
                free(languages[i]);
            }
            if((r = read(sockfd, buffer, BUFFER_SIZE - 1)) <= 0) cleanExit();
            sscanf(buffer, "%d", &res);
            if(res > 0){
                printerrmsg(stderrw, loginErrorStr(res));
            } else {
                isLogged = true;
                strcpy(client.username, username);
            }
            break;
        }
        default:
            //  Exit
            cleanExit();
            break;
        }

    }while(!isLogged);

    clear();
    refresh();
    flushinp();

    printerrmsg(stderrw, "You are logged in");

    WINDOW* roomsMenu = newwin(5, col, 0, 0);

    do{

        //  Rooms Menu
        const char *rooms[5] = {"IT", "EN", "FR", "ES", "Exit"};
        int selected = wmenu(roomsMenu, 5, rooms);
        if (selected >= 0 && selected < 4){
            reqJoinRoom(sockfd, selected);

            clear();
            refresh();

            if(waitInQueue(sockfd) < 0){
                cleanExit();
            }
        }
        else{
            cleanExit();
        }

        //  Start Chatroom
        clear();
        refresh();

        WINDOW *inputwin = newwin(3, col, row - 4, 0);
        WINDOW *messagesboxwin = newwin(row - 6, col, 0, 0);
        WINDOW *messageswin = newpad(100, col - 2);
        set_nonblocking(sockfd);

        initChatWindows(messageswin, messagesboxwin, inputwin);
        prefresh(messageswin, 0, 0, 1, 1, row - 8, col - 2);

        char message_buffer[BUFFER_SIZE] = {0};
        char input_buffer[BUFFER_SIZE] = {0};
        input_buffer[0] = '\0';
        int input_len = 0;
        int currentLine = 0;
        int scrollOffset = 0;
        int res;
        flushinp();

        do{
            //  Read String char by char Non-Blocking Way
            if((res = getStringNonBlocking(inputwin, input_buffer, &input_len)) == KEY_ENTER){
                if(write(sockfd, input_buffer, strlen(input_buffer)) < 0) cleanExit();
                if(strncmp(input_buffer, "/", 1) == 0){
                    if(handleCommand(input_buffer) == 1) break;
                }

                inputClear(inputwin);
                input_len = 0;
                input_buffer[0] = '\0';
            } else if(res == KEY_UP && currentLine >= row - 8 && scrollOffset != (currentLine - (row - 8))) {
                scrollOffset++;
                prefresh(messageswin, (currentLine - (row - 8)) - scrollOffset, 0, 1, 1, row - 8, col - 2);
            } else if(res == KEY_DOWN && currentLine >= row - 8 && scrollOffset != 0){
                scrollOffset--;
                prefresh(messageswin, (currentLine - (row - 8)) - scrollOffset, 0, 1, 1, row - 8, col - 2);
            }

            // Read and print messages from server
            int n;
            if ((n = read(sockfd, message_buffer, sizeof(message_buffer) - 1)) == 0) cleanExit();

            if (n > 0) {
                message_buffer[n] = '\0';
                printMessage(messageswin, message_buffer, &currentLine, row - 8, col - 2);
                wrefresh(inputwin);
            }

        } while(1);

        clear();
        delwin(inputwin);
        delwin(messageswin);
        delwin(messagesboxwin);
        refresh();

        toggle_nonblocking(sockfd);
    } while(1);

    endwin();
    close(sockfd);

    return 0;
}