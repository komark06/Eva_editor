#include "terminal.h"
#include "eva_string.h"
#include <sys/ioctl.h>

#define GETATTR_ERR "tcgetattr"
#define SETATTR_ERR "tcsetattr"

#define CLEAR_SCREEN "\x1b[2J" // VT100 reference: https://vt100.net/docs/vt100-ug/chapter3.html#ED
#define CURSOR_REPOSITION "\x1b[H" // VT100 reference: https://vt100.net/docs/vt100-ug/chapter3.html#CUP


/* Global variable */
static evastr bufio = NULL;

void die(const char *s)
{
    clear_reposition();
    show_errno(s);
    putchar('\r');
    exit(EXIT_FAILURE);
}

void disableRawMode(void)
{
    evafree(bufio);
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

static inline int getCursorPosition(void)
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
        return -2;
    if (sscanf(&buf[2], "%hu;%hu", &terminal_config.screenrows, &terminal_config.screencols) != 2)
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
        terminal_config.screencols = ws.ws_col;
        terminal_config.screenrows = ws.ws_row;
        return 0;
    }
}

int refresh(void)
{
    evastr cur = bufio;
    uint32_t len = evalen(bufio);
    do{
        errno = 0;
        ssize_t sv = write(STDIN_FILENO,cur,len);
        if (sv == -1)
            return -1;
        cur += sv;
        len -= sv;
    } while(len);
    evafree(bufio);
    bufio = NULL;
    return 0;
}

int add_screen(const char *buf, size_t len)
{
    if (!buf)
        return -1;
    evastr dst = evancat(bufio,buf,len);
    if (!dst)
        return -1;
    bufio = dst;
    return 0;
}

int clear_screen(void)
{
    evastr dst = evancat(bufio,CLEAR_SCREEN,sizeof(CLEAR_SCREEN)-1);
    if (!dst)
        return -1;
    bufio = dst;
    return 0;
}

int cursor_reposition(void)
{
    evastr dst = evancat(bufio,CURSOR_REPOSITION,sizeof(CURSOR_REPOSITION)-1);
    if (!dst)
        return -1;
    bufio = dst;
    return 0;
}

int clear_reposition(void)
{
    return clear_screen() || cursor_reposition();
}
