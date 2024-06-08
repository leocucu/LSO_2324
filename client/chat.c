#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#define BUFFER_SIZE 1024

bool getStringNonBlocking(WINDOW* win, char* input_buffer, int* input_len){
    nodelay(win, TRUE);

    int ch = wgetch(win);
    int max = getmaxx(win) - 2;
    int left = 0;

    if (ch != ERR){
        if (ch == '\n' || ch == KEY_ENTER){
            return true;
        }
        else if (ch == '\b' || ch == KEY_BACKSPACE || ch == 127){
            if ((*input_len) > 0){
                mvwaddch(win, 1, 1 + (*input_len), ' ');
                wmove(win, 1, 1 + (*input_len));
                input_buffer[--(*input_len)] = '\0';
            }
        }
        else if (isprint(ch) && (*input_len) < max - 1){
            input_buffer[(*input_len)++] = ch;
            input_buffer[(*input_len)] = '\0';
            mvwaddch(win, 1, 1 + (*input_len), ch);
        }
    }

    return false;
}

void inputClear(WINDOW* win){
    wclear(win);
    box(win, 0, 0);
    wrefresh(win);
}

void initChatWindows(WINDOW* messageswin, WINDOW* messagesboxwin, WINDOW* inputwin){
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
}

void printMessage(WINDOW* win, char* message, int* currentline){
    if ((*currentline) == getmaxy(win))
        (*currentline)--;
    mvwprintw(win, (*currentline), 0, "%s", message);
    (*currentline)++;
    wrefresh(win);
}