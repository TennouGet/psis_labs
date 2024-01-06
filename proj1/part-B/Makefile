all: server

proto: messages.proto
	protoc --c_out=. messages.proto

server: lizardsNroachesNwasps-server/server.c messages.pb-c.h
	gcc lizardsNroachesNwasps-server/server.c messages.pb-c.c -g -o lizardsNroachesNwasps-server/server -lzmq -lprotobuf-c -pthread -lncurses

display: Display-app/Display-app.c messages.pb-c.h
	gcc Display-app/Display-app.c messages.pb-c.c -g -o Display-app -lzmq -lprotobuf-c

roaches: Roaches-client/Roaches-client.c messages.pb-c.h
	gcc Roaches-client/Roaches-client.c messages.pb-c.c -g -o Roaches-client -lzmq -lprotobuf-c -lncurses

lizards: Lizard-client/Lizard-client.c messages.pb-c.h
	gcc Lizard-client/Lizard-client.c messages.pb-c.c -g -o Lizard-client/Lizard-client1 -lzmq -lprotobuf-c -lncurses