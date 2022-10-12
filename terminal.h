#if !defined(__TERMINAL_H__)
#define __TERMINAL_H__

#include <errno.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include "common.h"

struct editorConfig {
    unsigned short screenrows;
    unsigned short screencols;
    struct termios orig_termios;
};

extern struct editorConfig terminal_config;

/*
DESCRIPTION:
    Urgent error handling.
    Simply, we just call clear_reposition() and then print error message. Finally, call exit().

TIME: 2022/10/10
*/
void die(const char *s);

/*
DESCRIPTION:
    Clear screen.

RETURN VALUE:
    On succeed, return 0
    On error, return 1 and set @errno.
    
    VT100 reference: https://vt100.net/docs/vt100-ug/chapter3.html#ED

TIME: 2022/10/10
*/
int clear_screen(void);

/*
DESCRIPTION:
    Change the position of cursor to top left corner.
    
RETURN VALUE:
    On succeed, return 0
    On error, return 1 and set @errno.
    
    VT100 reference: https://vt100.net/docs/vt100-ug/chapter3.html#CUP

TIME: 2022/10/10
*/
int cursor_reposition(void);

/*
DESCRIPTION:
    Call clear_screen() and then cursor_reposition().
    If clear_screen() failed, cursor_reposition() won't execute.

RETURN VALUE:
    On succeed, return 0
    On error, return 1 and set @errno.

TIME: 2022/10/10
*/
int clear_reposition(void);

/*
DESCRIPTION:
    Disable raw mode and release resource.

TIME: 2022/10/10
*/
void disableRawMode(void)
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &terminal_config.orig_termios) == -1)
        die("tcsetattr");
}

/*
DESCRIPTION:
    Enable raw mode.
    On error, it will call die() and never return.

TIME: 2022/10/10
*/
void enableRawMode(void);
int getCursorPosition(void);
int getWindowSize(void);

#endif  // !defined(__TERMINAL_H__)
