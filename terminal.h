#if !defined(__TERMINAL_H__)
#define __TERMINAL_H__

#include <errno.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include "common.h"

#define GETATTR_ERR "tcgetattr"
#define SETATTR_ERR "tcsetattr"

struct editorConfig {
    unsigned short screenrows;
    unsigned short screencols;
    struct termios orig_termios;
};

static struct editorConfig E;

static ssize_t clear_screen(void)
{
    return write(STDOUT_FILENO, "\x1b[2J", 4) != 4;
}

static ssize_t cursor_reposition(void)
{
    return write(STDOUT_FILENO, "\x1b[H", 3) != 3;
}

static inline void clear_reposition(void)
{
    clear_screen();
    cursor_reposition();
}

static void die(const char *s)
{
    clear_reposition();
    show_errno(s);
    putchar('\r');
    exit(1);
}

static inline void disableRawMode(void)
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
        die(SETATTR_ERR);
}

void enableRawMode(void);

#endif  // !defined(__TERMINAL_H__)
