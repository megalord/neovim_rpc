HEAD = nvim_rpc.h
SRC = nvim_rpc.c print_buffer.c lib/cmp.c
PROG = print_buffer

nvim_rpc: scripts/api.c scripts/rpc_base.c scripts/rpc_base.h lib/cmp.h
	gcc -iquote lib scripts/api.c -o gen_api && ./gen_api && rm gen_api

$(PROG): $(SRC) $(HEAD)
	gcc -iquote lib -iquote src -Werror $(SRC) -o $(PROG)

debug: $(SRC)
	gcc -iquote lib -iquote src -g -Wall $(SRC) -o $(PROG)

lib/cmp.h:
	curl -o lib/cmp.h https://raw.githubusercontent.com/camgunz/cmp/master/cmp.h
lib/cmp.c: lib/cmp.h
	curl -o lib/cmp.c https://raw.githubusercontent.com/camgunz/cmp/master/cmp.c
