HEAD = nvim_rpc.h
SRC = nvim_rpc.c lib/cmp.c
PROG = print_buffer

nvim_rpc: scripts/api.c scripts/rpc_base.c scripts/rpc_base.h lib/cmp.h
	gcc -iquote lib scripts/api.c -o gen_api && ./gen_api && rm gen_api

$(PROG): $(SRC) $(HEAD) $(PROG).c
	gcc -iquote lib -iquote src -Werror $(SRC) $(PROG).c -o $(PROG)

debug: $(SRC)
	gcc -iquote lib -iquote src -g -Wall $(SRC) -o $(PROG)

test: $(SRC) test/test.c test/test.h
	gcc -iquote lib -iquote . -Wall $(SRC) test/test.c -o test/main && ./test/main

lib/cmp.h:
	curl -o lib/cmp.h https://raw.githubusercontent.com/camgunz/cmp/master/cmp.h
lib/cmp.c: lib/cmp.h
	curl -o lib/cmp.c https://raw.githubusercontent.com/camgunz/cmp/master/cmp.c
