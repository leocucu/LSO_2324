#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include "ncursesUI.h"

#define BUFFER_SIZE 1024
extern WINDOW* stderrw;

int getStringNonBlocking(WINDOW* win, char* input_buffer, int* input_len){
    nodelay(win, TRUE);

    int ch = wgetch(win);
    int max = getmaxx(win) - 2;
    int left = 0;

    if (ch != ERR){
        if (ch == '\n' || ch == KEY_ENTER){
            return KEY_ENTER;
        }
        else if (ch == '\b' || ch == KEY_BACKSPACE || ch == 127){
            if ((*input_len) > 0){
                if(input_buffer[(*input_len) - 1] < 0){
                    (*input_len)--;
                }
                (*input_len)--;

                input_buffer[(*input_len)] = '\0';
                wmove(win, 1, 1);
                wclrtoeol(win);
                box(win, 0, 0);
                wrefresh(win);
                mvwprintw(win, 1, 1, "%s", input_buffer);
            }
        }
        else if (ch == KEY_UP) {
            return KEY_UP;
        }
        else if (ch == KEY_DOWN){
            return KEY_DOWN;
        }
        else if ((*input_len) < max - 1){
            input_buffer[(*input_len)++] = ch;
            input_buffer[(*input_len)] = '\0';
            wmove(win, 1, 1);
            wclrtoeol(win);
            box(win, 0, 0);
            wrefresh(win);
            mvwprintw(win, 1, 1, "%s", input_buffer);
        }
    }

    return -1;
}

void inputClear(WINDOW* win){
    wclear(win);
    box(win, 0, 0);
    wmove(win, 1, 1);
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
        
    wrefresh(inputwin);
    wrefresh(messagesboxwin);
    // wrefresh(messageswin);
}

void printMessage(WINDOW* win, char* message, int* currentline, int winrows, int wincols){
    mvwprintw(win, (*currentline), 0, "%s", message);
    (*currentline)++;
    for(int i = ((int)(strlen(message) / wincols)); i > 0; i--)
        (*currentline)++;
    prefresh(win, (*currentline) - winrows, 0, 1, 1, winrows, wincols);
}