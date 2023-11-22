main: machine-control-client.c
	gcc -c machine-control-client.c
	gcc -c human-control-client.c
	gcc -c server.c
	gcc machine-control-client.o remote-char.h -o machine-control-client -lncurses
	gcc human-control-client.o remote-char.h -o human-control-client -lncurses
	gcc server.o remote-char.h -o server -lncurses
