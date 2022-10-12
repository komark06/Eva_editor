CC=gcc
DISABLE = -Wno-unused-result
CFLAG =-g -Og -Wextra -Wall -Winline -std=c99 -pedantic -fsanitize=leak -fsanitize=address -fsanitize=undefined $(DISABLE)
OBJ=common.o eva_string.o terminal.o main.o
EXE=eva

obj:$(OBJ) 
	$(CC) -o $(EXE) $(OBJ) $(CFLAG)

%.o: %.c
	$(CC) $< -c -o $@ $(CFLAG)

.PHONY:	clean
clean:
	rm -f *.o $(EXE)
