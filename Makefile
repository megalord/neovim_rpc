HEAD = test.h
SRC = main.c rpc.c cmp.c socket.c
PROG = test

$(PROG): $(SRC) $(HEAD)
				gcc $(SRC) -o $(PROG)

cmp.h:
	curl -o cmp.h https://raw.githubusercontent.com/camgunz/cmp/master/cmp.h
cmp.c: cmp.h
	curl -o cmp.c https://raw.githubusercontent.com/camgunz/cmp/master/cmp.c

test2: $(SRC)
				gcc -g -Wall main.c rpc.c cmp.c socket.c -o test
