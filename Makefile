all:
	gcc -ggdb3 ./server/skeleton_server_udp.c ./lib/207layer.c ./lib/207bind.c ./lib/207socket.c ./lib/207listen.c ./lib/207accept.c ./lib/connect207.c -o ./bin/udp_server -pthread


	gcc -ggdb3 -o ./bin/connect207 -I./client/lib/ ./client/client_shre.c ./client/TCPStateDiagram.c ./client/connect207.c

SHRE_CONNECT207:
	gcc -ggdb3 -o ./bin/connect207 -I./client/lib/ ./client/client_shre.c ./client/TCPStateDiagram.c ./client/connect207.c

clean:
	rm -rf bin/*
