all: lizardsNroaches-server.c
	gcc -Wall -g Lizard-client.c header.h -lzmq -lncurses -o Lizard-client
	gcc -Wall -g Roaches-client.c header.h -lzmq -lncurses -o Roaches-client
	gcc -Wall -g lizardsNroaches-server.c header.h -lzmq -lncurses -o lizardsNroaches-server
	gcc -Wall -g Display-app.c header.h -lzmq -lncurses -o Display-app
	