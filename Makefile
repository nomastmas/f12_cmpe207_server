all:
	gcc -ggdb3 -o ./bin/client207 -I./lib/ ./client/client_shre_shut.c ./lib/207layer.c ./lib/teardown207.c ./lib/connect207.c
	gcc -ggdb3 -o ./bin/server207 -I./lib/ ./server/close.c ./lib/207layer.c ./lib/teardown207.c ./lib/connect207.c

SHRE_CONNECT207:
	gcc -ggdb3 -o ./bin/connect207 -I./lib/ ./client/client_shre.c ./lib/207layer.c ./client/connect207.c

client207:
	gcc -ggdb3 -o ./bin/client207 -I./lib/ ./client/client_shre_shut.c ./lib/207layer.c ./lib/teardown207.c ./lib/connect207.c

server207:
	gcc -ggdb3 -o ./bin/server207 -I./lib/ ./server/close.c ./lib/207layer.c ./lib/teardown207.c ./lib/connect207.c


clean:
	rm -rf bin/*
