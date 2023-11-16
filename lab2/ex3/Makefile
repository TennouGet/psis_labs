main: server.c
	gcc -c -fPIC funcs.c
	gcc -shared -o funcs.so funcs.o
	gcc -c server.c
	gcc -c client.c
	gcc server.o funcs.so -o server
	gcc client.o -o client
	