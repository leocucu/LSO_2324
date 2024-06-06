#include "ncursesUI.h"
#include <ncurses.h>
#include <stdlib.h>

int wmenu(WINDOW *win, unsigned int chn, const char* chv[]){
    bool isKeypad = is_keypad(win);
    int prevCursorState = curs_set(0);
    keypad(win, TRUE);
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
    curs_set(prevCursorState);
    wclear(win);
    wrefresh(win);
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
