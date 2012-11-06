all:
	gcc -ggdb3 ./server/skeleton_server_udp.c ./lib/207layer.c ./lib/207bind.c ./lib/207socket.c -o ./bin/udp_server -pthread
	gcc -ggdb3 ./client/skeleton_client_udp.c ./lib/207layer.c ./lib/207bind.c ./lib/207socket.c -o ./bin/udp_client
	gcc -ggdb3 ./lib/TCPStateDiagram.c -o ./bin/TCPStateDiagram
	gcc -ggdb3 ./lib/207layer.c ./lib/207bind.c ./lib/207socket.c ./lib/TCP_socket_bind_test.c -o ./bin/test
	
udp_server:
	gcc -ggdb3 ./server/skeleton_server_udp.c ./lib/207layer.c ./lib/207bind.c ./lib/207socket.c -o ./bin/udp_server -pthread

udp_client:
	gcc -ggdb3 ./client/skeleton_client_udp.c ./lib/207layer.c ./lib/207bind.c ./lib/207socket.c -o ./bin/udp_client

TCPStateDiagram:
	gcc -ggdb3 ./lib/TCPStateDiagram.c -o ./bin/TCPStateDiagram

SocketBind:
	gcc -ggdb3 ./lib/207layer.c ./lib/207bind.c ./lib/207socket.c ./lib/TCP_socket_bind_test.c -o ./bin/test

clean:
	rm -rf bin/*
