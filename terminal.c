#include "terminal.h"

void enableRawMode(void)
{
    if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1)
        die(GETATTR_ERR);
    if (atexit(disableRawMode)) {
        clear_reposition();
        fprintf(stderr, "atexit failed.\n\r");
        exit(1);
    }

    struct termios raw = E.orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die(SETATTR_ERR);
    struct termios test;
    if (tcgetattr(STDIN_FILENO, &test) == -1)
        die(GETATTR_ERR);
    if (raw.c_iflag != test.c_iflag || raw.c_oflag != test.c_oflag ||raw.c_cflag != test.c_cflag ||
        raw.c_lflag != test.c_lflag || test.c_cc[VMIN] != 0 ||
        test.c_cc[VTIME] != 1) {
        clear_reposition();
        fprintf(stderr, "Can't set terminal to raw mode.\n\r");
        exit(1);
    }
}