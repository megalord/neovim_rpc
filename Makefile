SRC = test.h main.c rpc.c cmp.c socket.c
OBJ = main.o rpc.o cmp.o socket.o
PROG = test

$(PROG): $(OBJ)
				gcc $(OBJ) -o $(PROG)

$(OBJ): $(SRC)

test2: $(OBJ)
				gcc -g -Wall main.c rpc.c cmp.c socket.c -o test
