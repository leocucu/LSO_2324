#ifndef _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE_EXTENDED
#endif

#ifndef NCURSES_WIDECHAR
#define NCURSES_WIDECHAR 1
#endif

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
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
#define MIN_COL 50

struct client{
    char username[MAX_USERNAME_LEN + 1];
    int language;
    int room;
};

int sockfd;
WINDOW* stderrw;
WINDOW* mainw;

void cleanExit();
void cleanExitp(const char* message);

void cleanExit(){
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

    const char *hostname = getenv("SERVER_IP");
    if(hostname == NULL){
        if(inet_pton(AF_INET, SERVER_IP, &servaddr.sin_addr) <= 0) {
                perror("Invalid address/ Address not supported");
                exit(EXIT_FAILURE);
        }
    } else {
        struct hostent *server = gethostbyname(hostname);
        if(server == NULL){
                perror("Failed to resolve server hostname");
                exit(EXIT_FAILURE);
        }
        memcpy(&servaddr.sin_addr.s_addr, server->h_addr, server->h_length);
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);

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

    atexit(cleanExit);

    mainw = newwin(row - 1, col, 0, 0);
    stderrw = newwin(1, col, row - 1, 0);

    //  Login o Registrazione
    WINDOW* loginMenu = newwin(3, col, 0, 0);
    refresh();

    
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
            char username[MAX_USERNAME_LEN + 1];
            char password[MAX_USERNAME_LEN + 1];
            unsigned char res;

            getUsername(username, 0);
            if(write(sockfd, username, MAX_USERNAME_LEN + 1) <= 0) exit(1);
            if(getPassword(password, 1) < 0){
                cleanExitp("Max attempts reached");
            }
            if(write(sockfd, password, MAX_USERNAME_LEN + 1) <= 0) exit(1);
            clear();
            refresh();

            int r;
            if((r = read(sockfd, &res, 1)) <= 0) cleanExit();
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
            char username[MAX_USERNAME_LEN + 1];
            char password[MAX_USERNAME_LEN + 1];
            char confirmp[MAX_USERNAME_LEN + 1];
            char buffer[1024];
            unsigned char code;

            getUsername(username, 0);
            if(write(sockfd, username, MAX_USERNAME_LEN + 1) <= 0) cleanExit();

            if(getPassword(password, 1) < 0){
                cleanExitp("Max attempts reached");
            }

            if(getConfirm(confirmp, 2, password) < 0){
                cleanExitp("Max attempts reached");
            }

            if(write(sockfd, password, MAX_USERNAME_LEN + 1) <= 0) cleanExit();
            int n;

            char **languages;
            n = getLanguages(&languages, sockfd);
            if(n < 0) cleanExit();

            mvprintw(3, 0, "Select your Language:");
            WINDOW* menu = newwin(row-4, col, 4, 0);
            wrefresh(menu);
            refresh();
            int ch = womenu(menu, n, (const char **) languages);

            if(write(sockfd, languages[ch], 3) <= 0) cleanExit();
            clear();
            refresh();
            freeLanguages(languages, n);

            int r;
            if((r = read(sockfd, &code, 1)) <= 0) cleanExit();
            if(code > 0){
                printerrmsg(stderrw, loginErrorStr(code));
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

    do{

        //  Rooms Menu
        char** rooms;
        int n = getRooms(&rooms, sockfd);
        
        WINDOW* roomsMenu = newwin(n, col, 0, 0);
        refresh();

        unsigned char selected = (unsigned char)(wmenu(roomsMenu, n, (const char**)rooms));

        if (selected >= 0 && selected < n - 2){
            reqJoinRoom(sockfd, selected);

            clear();
            refresh();

            if(waitInQueue(sockfd) < 0){
                cleanExit();
            }
        } else if(selected == n - 2){
            clear();
            refresh();
            if(write(sockfd, &selected, 1) <= 0) cleanExit();

            mvprintw(0, 0, "Nome Stanza: ");
            char nome[MAX_ROOMNAME_LEN + 1];
            getAlnumString(nome, MAX_ROOMNAME_LEN, 0);
            if(write(sockfd, nome, MAX_ROOMNAME_LEN + 1) <= 0) cleanExit();


            mvprintw(1, 0, "Massimo di Utenti:  ");
            unsigned char max = (unsigned char)getNum();
            if(write(sockfd, &max, 1) <= 0) cleanExit();

            mvprintw(2, 0, "Seleziona la lingua:");
            WINDOW* languagesMenu = newwin(1, col, 3, 0);
            refresh();

            char **languages;
            int n = getLanguages(&languages, sockfd);
            if(n < 0) cleanExit();
            int c = womenu(languagesMenu, n, (const char **)languages);
            if(write(sockfd, languages[c], 2) <= 0) cleanExit();

            freeLanguages(languages, n);
            continue;
        } else{
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
        wint_t input_buffer[BUFFER_SIZE];
        input_buffer[0] = L'\0';
        int input_len = 0;
        int currentLine = 0;
        int scrollOffset = 0;
        int res;
        flushinp();

        freeRooms(rooms, n);

        do{
            //  Read String char by char Non-Blocking Way
            if((res = getStringNonBlocking(inputwin, input_buffer, &input_len)) == KEY_ENTER){
                char msgToSend[BUFFER_SIZE * 4];
                wcstombs(msgToSend, input_buffer, BUFFER_SIZE * 4);
                if(write(sockfd, msgToSend, strlen(msgToSend)) < 0) cleanExit();
                if(strncmp(msgToSend, "/", 1) == 0){
                    if(handleCommand(msgToSend) == 1) break;
                }

                inputClear(inputwin);
                input_len = 0;
                input_buffer[0] = L'\0';
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