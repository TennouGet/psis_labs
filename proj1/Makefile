all: server.c
	gcc -c lizard-client.c
	gcc -c server.c
	gcc -c display-app.c
	gcc lizard-client.o header.h -o lizard-client -lncurses -lzmq
	gcc server.o header.h -o server -lncurses -lzmq
	gcc display-app.o header.h -o display-app -lncurses -lzmq