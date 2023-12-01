remote-client: remote-display-client.c
	gcc -c human-control-client.c
	gcc -c server.c
	gcc -c remote-display-client.c
	gcc -c remote.c
	gcc human-control-client.o remote-char.h -o human-control-client -lncurses -lzmq
	gcc server.o remote-char.h -o server -lncurses -lzmq
	gcc remote-display-client.o remote-char.h -o remote-display-client -lncurses -lzmq
	gcc remote.o remote-char.h -o remote -lncurses -lzmq