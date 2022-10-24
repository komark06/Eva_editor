CC=gcc
CFLAG =-g -Og -Wextra -Wall -Winline -std=c99 -pedantic
EXE=eva

all:main.c
	$(CC) -o $(EXE) $^ $(CFLAG)

.PHONY:	clean
clean:
	rm -f *.o $(EXE)
