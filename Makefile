all:
	gcc -ggdb3 ./server/skeleton_server_udp.c ./lib/207layer.c -o ./bin/udp_server -pthread
	gcc -ggdb3 ./client/skeleton_client_udp.c ./lib/207layer.c -o ./bin/udp_client
	gcc -ggdb3 ./lib/TCPStateDiagram.c -o ./bin/TCPStateDiagram
	gcc -ggdb3 ./lib/207layer.c ./lib/207bind.c ./lib/207socket.c ./lib/TCP_socket_bind_test.c -o ./bin/test
	
udp_server:
	gcc -ggdb3 ./server/skeleton_server_udp.c ./lib/207layer.c -o ./bin/udp_server -pthread

udp_client:
	gcc -ggdb3 ./client/skeleton_client_udp.c ./lib/207layer.c -o ./bin/udp_client

TCPStateDiagram:
	gcc -ggdb3 ./lib/TCPStateDiagram.c -o ./bin/TCPStateDiagram

SocketBind:
	gcc -ggdb3 ./lib/207layer.c ./lib/207bind.c ./lib/207socket.c ./lib/TCP_socket_bind_test.c -o ./bin/test

SHRE_CONNECT207:
	gcc -ggdb3 -o ./bin/connect207 -I./lib/ ./client/client_shre.c ./lib/TCPStateDiagram.c ./lib/connect207.c
	
SHRE_LISTEN207:
	gcc -ggdb3 -o ./bin/listen207 -I./lib/ ./server/listen.c ./lib/TCPStateDiagram.c ./lib/connect207.c
	
clean:
	rm -rf bin/*
