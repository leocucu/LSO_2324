#ifndef NCURSESUI_H
#define NCURSESUI_H

#include <ncurses.h>

int wmenu(WINDOW *win, unsigned int chn, const char* chv[]);
int menu(unsigned int chn, const char* chv[]);
void printerrmsg(WINDOW* stderrw, char* msg);

#endif