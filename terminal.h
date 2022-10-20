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
    Disable raw mode and release resource.

TIME: 2022/10/10
*/
void disableRawMode(void);

/*
DESCRIPTION:
    Enable raw mode.
    On error, it will call die() and never return.

TIME: 2022/10/10
*/
void enableRawMode(void);

/*
DESCRIPTION:
    Get the row and column of cursor and assign it to @terminal_config.

RETURN VALUE:
    On succeed, return 0
    On error, return -1 and set @errno.
            , return -2. It indicates that there is no escape sequence.

TIME: 2022/10/20
*/
int getWindowSize(void);

/*
DESCRIPTION:
    Urgent error handling.
    Simply, we just call clear_reposition() and then print error message. Finally, call exit().

TIME: 2022/10/10
*/
void die(const char *s);

/*
DESCRIPTION:
    Refresh screen.

RETURN VALUE:
    On succeed, return 0.
    On error, return 1 and set @errno.

TIME: 2022/10/17
*/
int refresh(void);

/*
    [WARNING]: Below functions don't change on screen immediately. It will effect only when the function refresh() is called.
*/

/*
DESCRIPTION:
    Add content to screen.

RETURN VALUE:
    On succeed, return 0
    On error, return -1.
              return -2 when @buf is NULL.

TIME: 2022/10/20
*/
int add_screen(const char *buf, size_t len);

/*
DESCRIPTION:
    Clear screen.

RETURN VALUE:
    On succeed, return 0
    On error, return -1.
    
    VT100 reference: https://vt100.net/docs/vt100-ug/chapter3.html#ED

TIME: 2022/10/10
*/
int clear_screen(void);

/*
DESCRIPTION:
    Change the position of cursor to top left corner.
    
RETURN VALUE:
    On succeed, return 0
    On error, return -1.
    
    VT100 reference: https://vt100.net/docs/vt100-ug/chapter3.html#CUP

TIME: 2022/10/10
*/
int reposition_cursor(void);

/*
DESCRIPTION:
    Call clear_screen() and then reposition_cursor().
    If clear_screen() failed, reposition_cursor() won't execute.

RETURN VALUE:
    On succeed, return 0
    On error, return -1.

TIME: 2022/10/10
*/
int clear_reposition(void);

/*
DESCRIPTION:
    Hide cursor.

RETURN VALUE:
    On succeed, return 0
    On error, return -1.

TIME: 2022/10/20
*/
int hide_cursor(void);

/*
DESCRIPTION:
    Show cursor.

RETURN VALUE:
    On succeed, return 0
    On error, return -1.

TIME: 2022/10/20
*/
int show_cursor(void);

#endif  // !defined(__TERMINAL_H__)
