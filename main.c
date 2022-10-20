#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include "eva_string.h"
#include "terminal.h"

#define Memory_Error_Message "Not enough memory"

#define CTRL_KEY(k) ((k) &0x1f)

/* Global variable */
struct editorConfig terminal_config;

char editorReadKey(void)
{
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN)
            die("read");
    }
    return c;
}

/*** output ***/

void editorDrawRows(void)
{
    for (unsigned short y = 0; y < terminal_config.screenrows -1; y++) {
        char buf[32];
        int len = snprintf(buf,sizeof(buf),"%hu\r\n",y+1);
        if (len < 0)
            die("snprintf");
        write(STDOUT_FILENO, buf, len);
        //write(STDOUT_FILENO, "~\r\n", 3);
    }
    char buf[1024];
    int len = snprintf(buf,sizeof(buf),"row = %hu, col = %hu",terminal_config.screenrows,terminal_config.screencols);
    if (len < 0)
        die("snprintf");
    write(STDOUT_FILENO,buf,len);
}

void editorRefreshScreen(void)
{
    if (clear_reposition())
        die(Memory_Error_Message);
    editorDrawRows();
    if (cursor_reposition())
        die(Memory_Error_Message);
    if (refresh())
        die("write");
}

/*** input ***/

void editorProcessKeypress()
{
    char c = editorReadKey();

    switch (c) {
    case CTRL_KEY('q'):
        clear_reposition();
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
