#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include "terminal.h"

#define CTRL_KEY(k) ((k) &0x1f)

char editorReadKey()
{
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN)
            die("read");
    }
    return c;
}

int getCursorPosition(void)
{
    char buf[32];
    unsigned int i = 0;

    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
        return -1;

    while (i < sizeof(buf) - 1) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1)
            break;
        if (buf[i] == 'R')
            break;
        i++;
    }
    buf[i] = '\0';

    if (strncmp(buf,"\x1b[",2))
        return -1;
    if (sscanf(&buf[2], "%hu;%hu", &E.screenrows, &E.screencols) != 2)
        return -1;

    return 0;
}

int getWindowSize(void)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
            return -1;
        return getCursorPosition();
    } else {
        E.screencols = ws.ws_col;
        E.screenrows = ws.ws_row;
        return 0;
    }
}

/*** output ***/

void editorDrawRows(void)
{
    for (unsigned short y = 0; y < E.screenrows -1; y++) {
        write(STDOUT_FILENO, "~\r\n", 3);
    }
    char buf[1024];
    int len = snprintf(buf,sizeof(buf),"row = %hu, col = %hu",E.screenrows,E.screencols);
    if (len < 0)
        die("snprintf");
    write(STDOUT_FILENO,buf,len);
}

void editorRefreshScreen()
{
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    editorDrawRows();

    write(STDOUT_FILENO, "\x1b[H", 3);
}

/*** input ***/

void editorProcessKeypress()
{
    char c = editorReadKey();

    switch (c) {
    case CTRL_KEY('q'):
        write(STDOUT_FILENO, "\x1b[2J", 4);
        write(STDOUT_FILENO, "\x1b[H", 3);
        exit(0);
        break;
    }
}

/*** init ***/

void initEditor()
{
    if (getWindowSize() == -1)
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
