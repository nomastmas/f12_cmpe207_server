all:
	gcc -ggdb3 ./server/skeleton_server_udp.c ./lib/207layer.c -o ./bin/udp_server -pthread
	gcc -ggdb3 ./client/skeleton_client_udp.c ./lib/207layer.c -o ./bin/udp_client
	
udp_server:
	gcc -ggdb3 ./server/skeleton_server_udp.c ./lib/207layer.c -o ./bin/udp_server -pthread

udp_client:
	gcc -ggdb3 ./client/skeleton_client_udp.c ./lib/207layer.c -o ./bin/udp_client

clean:
	rm -rf bin/*