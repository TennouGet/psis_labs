all: server.c
	gcc -c lizard-client.c
	gcc -c server.c
	gcc -c display-app.c
	gcc lizard-client.o header.h -o lizard-client -lncurses -lzmq
	gcc -Wall -g server.c header.h -lzmq -lncurses -o server
	gcc display-app.o header.h -o display-app -lncurses -lzmq