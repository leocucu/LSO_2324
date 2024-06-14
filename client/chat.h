#ifndef CHAT_H
#define CHAT_H

#include <ncurses.h>

bool getStringNonBlocking(WINDOW* win, char* input_buffer, int* input_len);
void inputClear(WINDOW* win);
void initChatWindows(WINDOW* messageswin, WINDOW* messagesboxwin, WINDOW* inputwin);
void printMessage(WINDOW* win, char* message, int* currentline, int winsize, int wincols);

#endif