HEAD = src/rpc_methods.h
SRC = test.c src/rpc.c lib/cmp.c src/socket.c
PROG = print_buffer

$(PROG): $(SRC) $(HEAD)
	gcc -iquote lib -iquote src -Werror $(SRC) -o $(PROG)

debug: $(SRC)
	gcc -iquote lib -iquote src -g -Wall $(SRC) -o $(PROG)

lib/cmp.h:
	curl -o lib/cmp.h https://raw.githubusercontent.com/camgunz/cmp/master/cmp.h
lib/cmp.c: lib/cmp.h
	curl -o lib/cmp.c https://raw.githubusercontent.com/camgunz/cmp/master/cmp.c

rpc_methods:
	gcc -iquote lib scripts/api.c -o gen_api && ./gen_api > src/rpc_methods.h && rm gen_api
