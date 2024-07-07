#ifndef NCURSESUI_H
#define NCURSESUI_H

#include <ncurses.h>

int wmenu(WINDOW *win, unsigned int chn, const char* chv[]);
int womenu(WINDOW *win, unsigned int chn, const char* chv[]);
int menu(unsigned int chn, const char* chv[]);
void printerrmsg(WINDOW* stderrw, const char* msg);
int wgetAlnumString(WINDOW* win, char *oust, int max, char echo);
int wgetNum(WINDOW* win);
int getAlnumString(char *out, int max, char echo);
int getNum();

#endif