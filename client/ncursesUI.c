#include "ncursesUI.h"
#include <ncurses.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

int wmenu(WINDOW *win, unsigned int chn, const char* chv[]){
    noecho();
    bool isKeypad = is_keypad(win);
    keypad(win, TRUE);
    int cursor = curs_set(0);
    int key;
    int c = 0;

    while(1){
        for(int i = 0; i < chn; i++){
            if(c == i) {
                wattron(win, A_REVERSE);
            }
            mvwprintw(win, i, 0, chv[i]);
            wattroff(win, A_REVERSE);
        }

        wrefresh(win);
        key = wgetch(win);

        switch(key){
            case KEY_UP:
                c > 0 && c--;
                break;
            case KEY_DOWN:
                c < (chn - 1) && c++;
                break;
            default:
                break;
        }

        if(key == '\n') break;
    }

    keypad(win, isKeypad);
    wclear(win);
    wrefresh(win);
    curs_set(cursor);
    return c;
}

int menu(unsigned int chn, const char* chv[]){
    return wmenu(stdscr, chn, chv);
}

void printerrmsg(WINDOW* stderrw, char* msg){
    wclear(stderrw);
    wrefresh(stderrw);
    mvwprintw(stderrw, 0, 0, "Error: %s", msg);
    wrefresh(stderrw);
}

int wgetAlnumString(WINDOW* win, char *out, int max, char echo){
    int isKeypad = is_keypad(win);
    char str[max + 1];
    int i = 0;
    int key;

    noecho();
    keypad(win, TRUE);

    do{
        key = wgetch(win);

        if(i < max && isalnum(key)){
            str[i++] = key;
            str[i] = '\0';
            waddch(win, echo == 0 ? key : echo);
        } else if ((key == KEY_BACKSPACE || key == 127 || key == '\b') &&  i > 0) {
            str[--i] = '\0';
            wmove(win, getcury(win), getcurx(win) - 1);
            wdelch(win);
        } else if ((key == KEY_ENTER || key == '\n')){
            break;
        }
    } while(1);

    keypad(win, isKeypad);
    strcpy(out, str);
    return i;
}

int getAlnumString(char *out, int max, char echo){
    return wgetAlnumString(stdscr, out, max, echo);
}