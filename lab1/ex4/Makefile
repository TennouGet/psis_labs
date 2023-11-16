main:	main.c
	 gcc -c main.c
	 gcc lib1.c -o lib1.so -ldl -shared -fPIC
	 gcc lib2.c -o lib2.so -ldl -shared -fPIC
	 gcc main.o lib1.so lib2.so lib.h -o ex4

clean:
	rm -f ex4