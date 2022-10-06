CC=gcc
DISABLE = -Wno-unused-result
CFLAG =-g -Og -Wextra -Wall -Winline -std=c99 -pedantic $(DISABLE)
OBJ=common.o terminal.o editor.o
EXE=eva

obj:$(OBJ) 
	$(CC) -o $(EXE)  $(OBJ) $(CFLAG)

hooks:
	@scripts/install-git-hooks

%.o: %.c
	$(CC) $< -c -o $@ $(CFLAG)

.PHONY:	clean
clean:
	rm -f *.o $(EXE)
