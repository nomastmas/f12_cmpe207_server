all:
	gcc -ggdb3 ./server/skeleton_server_udp.c ./lib/207layer.c ./lib/207bind.c ./lib/207socket.c ./lib/207listen.c ./lib/207accept.c ./lib/connect207.c -o ./bin/udp_server -pthread
	gcc -ggdb3 ./client/skeleton_client_udp.c ./lib/207layer.c ./lib/207bind.c ./lib/207socket.c -o ./bin/udp_client

clean:
	rm -rf bin/*
