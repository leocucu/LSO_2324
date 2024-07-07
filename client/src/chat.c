#ifndef NCURSES_WIDECHAR
#define NCURSES_WIDECHAR 1
#endif

#ifndef _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE_EXTENDED
#endif

#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include "ncursesUI.h"

#define BUFFER_SIZE 1024
extern WINDOW* stderrw;

int getStringNonBlocking(WINDOW* win, wint_t* input_buffer, int* input_len){
    nodelay(win, TRUE);
    wint_t ch;

    int code = wget_wch(win, &ch);
    int max = getmaxx(win) - 2;
    int left = 0;

    if (code != ERR){
        if (ch == '\n' || ch == KEY_ENTER){
            return KEY_ENTER;
        }
        else if (ch == '\b' || ch == KEY_BACKSPACE || ch == 127){
            if ((*input_len) > 0){
                (*input_len)--;

                input_buffer[(*input_len)] = L'\0';
                wmove(win, 1, 1);
                wclrtoeol(win);
                box(win, ACS_VLINE, ACS_HLINE);
                mvwaddwstr(win, 1, 1, input_buffer);
                wrefresh(win);
            }
        }
        else if (ch == KEY_UP) {
            return KEY_UP;
        }
        else if (ch == KEY_DOWN){
            return KEY_DOWN;
        }
        else if (((*input_len) < max - 1) && (code != KEY_CODE_YES)){
            input_buffer[(*input_len)++] = ch;
            input_buffer[(*input_len)] = L'\0';
            wmove(win, 1, 1);
            wclrtoeol(win);
            box(win, 0, 0);
            mvwaddwstr(win, 1, 1, input_buffer);
            wrefresh(win);
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