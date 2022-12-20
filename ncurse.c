#include <errno.h>
#include <limits.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "common.h"
#include "content.h"
#include "eva_string.h"

void startup(void)
{
    if (!initscr()) {
        fprintf(stderr, "Can't enter terminal mode\n");
        exit(EXIT_FAILURE);
    }
    if (!has_colors()) {
        addstr("This terminal doesn't support color mode.\n");
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
    endwin();
    exit(EXIT_FAILURE);
}

void test(void)
{
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
    errno = 0;
    FILE *fr = fopen(filename, "r");
    if (!fr) {
        printw("Can't open %s: %s\n", filename, strerror(errno));
        goto IO_Fail;
    }
    errno = 0;
    content_t *content = new_cont();
    if (!content) {
        printw("%s\n", strerror(errno));
        goto Fail;
    }
    errno = 0;
    while (!feof(fr)) {
        int err = add_cont(&content, fr);
        if (err) {
            printf("add_text: %s\n", strerror(errno));
            goto Fail;
        }
    }
    int cur = 0, row, col, mrow, mcol;
    getyx(stdscr, row, col);
    getmaxyx(stdscr, mrow, mcol);
    while (row < mrow && cur < last_cont(content)) {
        evastr str = read_cont(content, cur);
        if (mcol > evalen(str))
            addnstr(str, evalen(str));
        else
            addnstr(str, mcol - 1);
        cur++;
        row++;
    }
    refresh();
    getch();
    errno = 0;
    if (fclose(fr)) {
        clear();
        printw("Can't close %s: %s\n", filename, strerror(errno));
        goto IO_Fail;
    }
    del_cont(content);
    if (endwin())
        exit(EXIT_FAILURE);
    return;
Fail:
    errno = 0;
    if (fclose(fr)) {
        clear();
        printw("Can't close %s: %s\n", filename, strerror(errno));
    }
    del_cont(content);
IO_Fail:
    addstr("Press any key to quit.\n");
    getch();
    if (endwin() == ERR)
        exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    printf("%u\n", UINT32_MAX);
    FILE *fr = fopen("trash.txt", "r");
    if (!fr) {
        printw("Can't open %s: %s\n", "trash.txt", strerror(errno));
        return -1;
    }
    errno = 0;
    content_t *content = new_cont();
    if (!content) {
        printw("%s\n", strerror(errno));
        return -2;
    }
    errno = 0;
    while (!feof(fr)) {
        int err = add_cont(&content, fr);
        if (err) {
            printf("add_text: %s\n", strerror(errno));
            return -3;
        }
    }
    return 0;

    startup();
    if (argc == 1) {
        test();
    } else if (argc == 2) {
        readfile(argv[1]);
    }
    return 0;
}