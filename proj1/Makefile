all: server.c
	gcc -c lizard-client.c
	gcc -c server.c
	gcc -c remote-display-client.c
	gcc -c display-app.c
	gcc lizard-client.o header.h -o lizard-client-client -lncurses -lzmq
	gcc server.o header.h -o server -lncurses -lzmq
	gcc remote-display-client.o header.h -o remote-display-client -lncurses -lzmq
	gcc display-app.o header.h -o display-app -lncurses -lzmq