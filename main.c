#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include "eva_string.h"
#include "terminal.h"

#define WELCOME "Welcomde to Eva_Editor 0.0.1\n\r"
#define CTRL_KEY(k) ((k) &0x1f)

enum editorKey {
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    DEL_KEY,
    PAGE_UP,
    PAGE_DOWN,
    HOME_KEY,
    END_KEY
};

/* Global variable */
struct editorConfig terminal_config;

/*** row operation ***/

int editorAppendRow(const char *s, size_t len)
{
    if (terminal_config.content.maxrow == terminal_config.content.currow) {
        if (sizeof(*terminal_config.content.str) >
            SIZE_MAX / 2 / terminal_config.content.maxrow)
            return -1;
        char **str = realloc(terminal_config.content.str,
                             sizeof(*terminal_config.content.str) *
                                 terminal_config.content.maxrow * 2);
        if (!str)
            return -1;
        terminal_config.content.str = str;
        terminal_config.content.maxrow *= 2;
    }
    terminal_config.content.str[terminal_config.content.currow] =
        evannew(s, len);
    if (!terminal_config.content.str[terminal_config.content.currow])
        return -1;
    terminal_config.content.currow++;
    return 0;
}

/*** file i/o ***/

void editorOpen(const char *filename)
{
    errno = 0;
    FILE *fr = fopen(filename, "r");
    if (!fr)
        die("fopen");
    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    while ((linelen = getline(&line, &linecap, fr)) != -1) {
        while (linelen > 0 &&
               (line[linelen - 1] == '\n' || line[linelen - 1] == '\r'))
            linelen--;
        if (editorAppendRow(line, linelen))
            memory_error();
    }
    free(line);
    fclose(fr);
}


/*** input ***/

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
                    case '1':
                        return HOME_KEY;
                    case '3':
                        return DEL_KEY;
                    case '4':
                        return END_KEY;
                    case '5':
                        return PAGE_UP;
                    case '6':
                        return PAGE_DOWN;
                    case '7':
                        return HOME_KEY;
                    case '8':
                        return END_KEY;
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
                case 'H':
                    return HOME_KEY;
                case 'F':
                    return END_KEY;
                }
            }
        } else if (seq[0] == 'O') {
            switch (seq[1]) {
            case 'H':
                return HOME_KEY;
            case 'F':
                return END_KEY;
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
    // Draw top half of screen
    for (unsigned short y = 0; y < terminal_config.screenrows / 2; y++) {
        if (add_screen("~\n\r", 4))
            memory_error();
    }
    // Draw middle line
    if (add_screen("~", 1))
        memory_error();
    unsigned short wellen = sizeof(WELCOME) - 1;
    int pad;
    if (wellen > terminal_config.screencols) {
        wellen = terminal_config.screencols;
        pad = 0;
    } else {
        pad = (terminal_config.screencols - wellen) / 2;
    }
    if (add_space(pad))
        memory_error();
    if (add_screen(WELCOME, wellen))
        memory_error();
    // Draw down half of screen
    for (unsigned short y = 0; y < terminal_config.screenrows / 2 - 1; y++) {
        if (add_screen("~\n\r", 4))
            memory_error();
    }
    // Draw last line
    char ed[128];
    int len = snprintf(ed, sizeof(ed), "(%hu,%hu)", terminal_config.currentcol,
                       terminal_config.currentrow);
    if (len < 0)
        die("snprintf");
    if (add_screen(ed, len))
        memory_error();
}

void editorDrawFile()
{
    if (hide_cursor() || clear_reposition())
        memory_error();
    for (unsigned int i = 0; i < terminal_config.screenrows; ++i) {
        const evastr src = terminal_config.content.str[i];
        if (add_screen(src, evalen(src)) || add_screen("\n\r", 2))
            memory_error();
    }
    if (move_cursor() || show_cursor())
        memory_error();
    if (refresh())
        die("refresh");
}

void editorRefreshScreen(void)
{
    if (hide_cursor() || clear_reposition())
        memory_error();
    editorDrawRows();
    if (move_cursor() || show_cursor())
        memory_error();
    if (refresh())
        die("refresh");
}

/*** input ***/

void editorMoveCursor(int key)
{
    switch (key) {
    case ARROW_LEFT:
        if (terminal_config.currentcol > 1)
            terminal_config.currentcol--;
        break;
    case ARROW_RIGHT:
        if (terminal_config.currentcol != terminal_config.screencols)
            terminal_config.currentcol++;
        break;
    case ARROW_UP:
        if (terminal_config.currentrow > 1)
            terminal_config.currentrow--;
        break;
    case ARROW_DOWN:
        if (terminal_config.currentrow != terminal_config.screenrows)
            terminal_config.currentrow++;
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
    case HOME_KEY:
        terminal_config.currentrow = 1;
        break;
    case END_KEY:
        terminal_config.currentrow = terminal_config.screenrows;
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

void initEditor(int yes)
{
    terminal_config.currentcol = 1;
    terminal_config.currentrow = 1;
    if (getWindowSize())
        die("getWindowSize");
    if (yes) {
        terminal_config.content.str = malloc(
            sizeof(*terminal_config.content.str) * terminal_config.currentrow);
        if (!terminal_config.content.str)
            memory_error();
        terminal_config.content.maxrow = terminal_config.currentrow;
    }
}

int main(int argc, char **argv)
{
    enableRawMode();
    if (argc >= 2) {
        initEditor(1);
        editorOpen(argv[1]);
        while (1) {
            editorDrawFile();
            editorProcessKeypress();
        }
    }

    // Welcome
    initEditor(0);
    while (1) {
        editorRefreshScreen();
        editorProcessKeypress();
    }

    return 0;
}
