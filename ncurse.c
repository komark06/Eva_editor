#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "common.h"

void startup(void)
{
    if (!initscr()) {
        fprintf(stderr, "Can't enter terminal mode\n");
        exit(EXIT_FAILURE);
    }
    if (!has_colors()) {
        addstr("This terminal doesn't support color.\n");
        goto Fail;
    }
    if (noecho()) {
        addstr("Can't enable noecho mode\n");
        goto Fail;
    }
    if (cbreak()) {
        addstr("Can't enable cbreak mode\n");
        goto Fail;
    }
    if (keypad(stdscr, TRUE)) {
        addstr("Can't detect function key.\n");
        goto Fail;
    }
    if (start_color()) {
        addstr("Can't enable color mode.\n");
        goto Fail;
    }
    for (int i = 1; i < 8; ++i) {
        if (init_pair(i, i, COLOR_BLACK) == ERR) {
            addstr("Can't initilize color pair\n");
            goto Fail;
        }
    }
    return;
Fail:
    getch();
    if (endwin() == ERR)
        exit(EXIT_FAILURE);
}

void test(void)
{
    startup();
    srand(time(NULL));
    addstr("Enter test mode. Press F1 to quit\n");
    int ch = getch();
    while (ch != KEY_F(1)) {
        addch(COLOR_PAIR(rand() % 8) | ch);
        refresh();
        ch = getch();
    }
    if (endwin() == ERR)
        exit(EXIT_FAILURE);
}

void readfile(const char *filename)
{
    startup();
    srand(time(NULL));
    errno = 0;
    FILE *fr = fopen(filename, "r");
    if (!fr) {
        printw("Can't open %s: %s\n", filename, strerror(errno));
        goto Fail;
    }
    addstr("Press F1 to quit\n");
    int ch = fgetc(fr);
    while (ch != EOF) {
        addch(COLOR_PAIR(rand() % 8) | ch);
        refresh();
        ch = fgetc(fr);
    }
    getch();
    errno = 0;
    if (fclose(fr)) {
        clear();
        printw("Can't close %s: %s\n", filename, strerror(errno));
        goto Fail;
    }
    if (endwin())
        exit(EXIT_FAILURE);
    return;
Fail:
    addstr("Press any key to quit.\n");
    getch();
    if (endwin() == ERR)
        exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    if (argc == 1) {
        test();
    } else if (argc == 2) {
        readfile(argv[1]);
    }
    return 0;
}