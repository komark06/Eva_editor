#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include "eva_string.h"
#include "terminal.h"

#define Memory_Error_Message "Not enough memory"
#define WELCOME "Welcomde to Eva_Editor 0.0.1\n\r"
#define CTRL_KEY(k) ((k) &0x1f)

enum editorKey {
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    PAGE_UP,
    PAGE_DOWN
};


/* Global variable */
struct editorConfig terminal_config;

int editorReadKey(void)
{
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN)
            die("read");
    }
    if (c == '\x1b') {
        char seq[3];
        if (read(STDIN_FILENO, &seq[0], 1) != 1)
            return '\x1b';
        if (read(STDIN_FILENO, &seq[1], 1) != 1)
            return '\x1b';
        if (seq[0] == '[') {
            if (seq[1] >= '0' && seq[1] <= '9') {
                if (read(STDIN_FILENO, &seq[2], 1) != 1)
                    return '\x1b';
                if (seq[2] == '~') {
                    switch (seq[1]) {
                    case '5':
                        return PAGE_UP;
                    case '6':
                        return PAGE_DOWN;
                    }
                }
            } else {
                switch (seq[1]) {
                case 'A':
                    return ARROW_UP;
                case 'B':
                    return ARROW_DOWN;
                case 'C':
                    return ARROW_RIGHT;
                case 'D':
                    return ARROW_LEFT;
                }
            }
        }
        return '\x1b';
    } else {
        return c;
    }
}

/*** output ***/

void editorDrawRows(void)
{
    for (unsigned short y = 0; y < terminal_config.screencols / 2; y++) {
        if (add_screen("~\n\r", 4))
            die(Memory_Error_Message);
    }
    if (add_screen("~", 1))
        die(Memory_Error_Message);
    unsigned short wellen = sizeof(WELCOME) - 1;
    int pad;
    if (wellen > terminal_config.screenrows) {
        wellen = terminal_config.screenrows;
        pad = 0;
    } else {
        pad = (terminal_config.screenrows - wellen) / 2;
    }
    if (add_space(pad))
        die(Memory_Error_Message);
    if (add_screen(WELCOME, wellen))
        die(Memory_Error_Message);
    for (unsigned short y = 0; y < terminal_config.screencols / 2 - 1; y++) {
        if (add_screen("~\n\r", 4))
            die(Memory_Error_Message);
    }
    char ed[128];
    int len = snprintf(ed, sizeof(ed), "(%hu,%hu)", terminal_config.currentcol,
                       terminal_config.currentrow);
    if (len < 0)
        die("snprintf");
    if (add_screen(ed, len))
        die(Memory_Error_Message);
}

void editorRefreshScreen(void)
{
    if (hide_cursor() || clear_reposition())
        die(Memory_Error_Message);
    editorDrawRows();
    if (move_cursor() || show_cursor())
        die(Memory_Error_Message);
    if (refresh())
        die("refresh");
}

/*** input ***/

void editorMoveCursor(int key)
{
    switch (key) {
    case ARROW_LEFT:
        if (terminal_config.currentrow > 1)
            terminal_config.currentrow--;
        break;
    case ARROW_RIGHT:
        if (terminal_config.currentrow != terminal_config.screenrows)
            terminal_config.currentrow++;
        break;
    case ARROW_UP:
        if (terminal_config.currentcol > 1)
            terminal_config.currentcol--;
        break;
    case ARROW_DOWN:
        if (terminal_config.currentcol != terminal_config.screencols)
            terminal_config.currentcol++;
        break;
    }
}

void editorProcessKeypress(void)
{
    int c = editorReadKey();

    switch (c) {
    case CTRL_KEY('q'):
        exit(0);
        break;
    case PAGE_UP:
        terminal_config.currentcol = 1;
        break;
    case PAGE_DOWN:
        terminal_config.currentcol = terminal_config.screencols;
        break;
    case ARROW_UP:
    case ARROW_DOWN:
    case ARROW_LEFT:
    case ARROW_RIGHT:
        editorMoveCursor(c);
        break;
    }
}

/*** init ***/

void initEditor(void)
{
    terminal_config.currentcol = 1;
    terminal_config.currentrow = 1;
    if (getWindowSize())
        die("getWindowSize");
}

int main()
{
    enableRawMode();
    initEditor();
    while (1) {
        editorRefreshScreen();
        editorProcessKeypress();
    }

    return 0;
}
