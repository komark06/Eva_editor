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
        int len = snprintf(buf,sizeof(buf),"%hu",y+1);
        if (len < 0)
            die("snprintf");
        if (add_screen(buf,len))
            die(Memory_Error_Message);
        if (terminal_config.screenrows/2 == y){
            unsigned short wellen = sizeof(WELCOME);
            int pad;
            if  (wellen > terminal_config.screencols){
                wellen = terminal_config.screencols;
                pad = 0;
            } else{
                pad = (terminal_config.screencols - wellen)/2;
            }
            while(pad > 0){
                if(add_screen(" ",1))
                    die(Memory_Error_Message);
                pad--;
            }
            if(add_screen(WELCOME,wellen))
                die(Memory_Error_Message);
        } else{
            if(add_screen("\n\r",3))
                die(Memory_Error_Message);
        }
    }
    char ed[1024];
    int len = snprintf(ed,sizeof(ed),"row = %hu, col = %hu",terminal_config.screenrows,terminal_config.screencols);
    if (len < 0)
        die("snprintf");
    if (add_screen(ed,len))
        die(Memory_Error_Message);
}

void editorRefreshScreen(void)
{
    if (hide_cursor())
        die(Memory_Error_Message);
    if (clear_reposition())
        die(Memory_Error_Message);
    editorDrawRows();
    if (reposition_cursor())
        die(Memory_Error_Message);
    if (show_cursor())
        die(Memory_Error_Message);
    if (refresh())
        die("write");
}

/*** input ***/

void editorProcessKeypress(void)
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

void initEditor(void)
{
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
