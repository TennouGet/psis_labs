server: server.c messages.pb-c.h
	gcc server.c messages.pb-c.c -g -o server -lzmq -lprotobuf-c -pthread -lncurses

lizards: Lizard-client.c messages.pb-c.h
	gcc Lizard-client.c messages.pb-c.c -g -o Lizard-client1 -lzmq -lprotobuf-c -lncurses	