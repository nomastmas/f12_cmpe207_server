all:
	gcc -ggdb3 ./server/skeleton_server_udp.c ./lib/207layer.c -o ./bin/udp_server -pthread
	gcc -ggdb3 ./client/skeleton_client_udp.c ./lib/207layer.c -o ./bin/udp_client
	gcc -ggdb3 ./lib/TCPStateDiagram.c -o ./bin/TCPStateDiagram
	
udp_server:
	gcc -ggdb3 ./server/skeleton_server_udp.c ./lib/207layer.c -o ./bin/udp_server -pthread

udp_client:
	gcc -ggdb3 ./client/skeleton_client_udp.c ./lib/207layer.c -o ./bin/udp_client

TCPStateDiagram:
	gcc -ggdb3 ./lib/TCPStateDiagram.c -o ./bin/TCPStateDiagram

clean:
	rm -rf bin/*
