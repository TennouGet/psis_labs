main: server.c
	gcc -c human-control-client.c
	gcc -c server.c
	gcc human-control-client.o remote-char.h -o human-control-client -lncurses -lzmq
	gcc server.o remote-char.h -o server -lncurses -lzmq