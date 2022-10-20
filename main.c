#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#define GETATTR_ERR "tcgetattr"
#define SETATTR_ERR "tcsetattr"

struct editorConfig {
    unsigned short screenrows;
    unsigned short screencols;
    struct termios orig_termios;
};

struct editorConfig terminal_config;

static inline void clear_reposition(void)
{
    write(STDIN_FILENO,"\x1b[2J\x1b[H",8);
}

void die(const char *s)
{

    perror(s);
    putchar('\r');
    exit(EXIT_FAILURE);
}

void disableRawMode(void)
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &terminal_config.orig_termios) == -1)
        die("tcsetattr");
    
}

void enableRawMode(void)
{
    if (tcgetattr(STDIN_FILENO, &terminal_config.orig_termios) == -1)
        die(GETATTR_ERR);
    if (atexit(disableRawMode)) {
        clear_reposition();
        fprintf(stderr, "atexit failed.\n\r");
        exit(1);
    }
    struct termios raw = terminal_config.orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 1;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die(SETATTR_ERR);
    struct termios test;
    if (tcgetattr(STDIN_FILENO, &test) == -1)
        die(GETATTR_ERR);
    if (raw.c_iflag != test.c_iflag || raw.c_oflag != test.c_oflag ||raw.c_cflag != test.c_cflag ||
        raw.c_lflag != test.c_lflag || test.c_cc[VMIN] != 1 ||
        test.c_cc[VTIME] != 1) {
        clear_reposition();
        fprintf(stderr, "Can't set terminal to raw mode.\n\r");
        exit(1);
    }
}

int main()
{
    enableRawMode();
    char c;
    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q') {
    if (iscntrl(c)) {
      printf("%d\n\r", c);
    } else {
      printf("%d ('%c')\n\r", c, c);
    }
  }

    return 0;
}
