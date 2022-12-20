CC=gcc
DISABLE=-Wno-unused-result
# FSAN = -fsanitize=address -fsanitize=leak
CFLAG=-g -O0 -Wextra -Wall -Winline -std=c99 -pedantic $(FSAN) $(DISABLE) 
OBJ=common.o eva_string.o ncurse.o content.o
EXE=eva

obj:$(OBJ) 
	clang-format -i *.c *.h
	$(CC) -o $(EXE) $(OBJ) $(CFLAG) -lncurses

%.o: %.c
	$(CC) $< -c -o $@ $(CFLAG)

.PHONY:	clean
clean:
	rm -f *.o $(EXE)
