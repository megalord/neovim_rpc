SRC = test.h main.c rpc.c cmp.c socket.c
OBJ = main.o rpc.o cmp.o socket.o
PROG = test

$(PROG): $(OBJ)
				gcc $(OBJ) -o $(PROG)

$(OBJ): $(SRC)

cmp.h:
	curl -o cmp.h https://raw.githubusercontent.com/camgunz/cmp/master/cmp.h
cmp.c: cmp.h
	curl -o cmp.c https://raw.githubusercontent.com/camgunz/cmp/master/cmp.c

test2: $(OBJ)
				gcc -g -Wall main.c rpc.c cmp.c socket.c -o test
