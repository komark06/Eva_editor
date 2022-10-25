CC=gcc
DISABLE=-Wno-unused-result
fsan?=1
CFLAG=-g -Og -Wextra -Wall -Winline -std=c99 -pedantic $(FSAN) $(DISABLE)
OBJ=common.o eva_string.o terminal.o main.o
EXE=eva

ifneq ($(fsan),0)
	FSAN=-fsanitize=leak -fsanitize=address -fsanitize=undefined
endif

obj:$(OBJ) 
	clang-format -i *.c *.h
	$(CC) -o $(EXE) $(OBJ) $(CFLAG)

%.o: %.c
	$(CC) $< -c -o $@ $(CFLAG)

.PHONY:	clean
clean:
	rm -f *.o $(EXE)
