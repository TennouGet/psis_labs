all: server

proto: messages.proto
	protoc --c_out=. messages.proto

server: lizardsNroachesNwasps-server/server.c messages.pb-c.h
	gcc lizardsNroachesNwasps-server/server.c messages.pb-c.c -g -o server -lzmq -lprotobuf-c
	gcc lizardsNroachesNwasps-server/lizard_threads.c messages.pb-c.c -g -o lizard_threads -lzmq -lprotobuf-c

display: Display-app/Display-app.c messages.pb-c.h
	gcc Display-app/Display-app.c messages.pb-c.c -g -o Display-app -lzmq -lprotobuf-c