server: server.c messages.pb-c.h
	gcc server.c messages.pb-c.c -g -o server -lzmq -lprotobuf-c -pthread -lncurses

lizards: Lizard-client.c messages.pb-c.h
	gcc Lizard-client.c messages.pb-c.c -g -o Lizard-client1 -lzmq -lprotobuf-c -lncurses

proto: messages.proto
	protoc --c_out=. messages.proto

roaches: Roaches-client.c
	gcc -g Roaches-client.c messages.pb-c.c header.h -lzmq -lprotobuf-c -lncurses -o Roaches-client