#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>
#include <dlfcn.h>

int main(){


	int fd;

	while((fd = open("/tmp/fifo_teste", O_RDONLY))== -1){
	  if(mkfifo("/tmp/fifo_teste", 0666)!=0){
			printf("problem creating the fifo\n");
			exit(-1);
	  }else{
		  printf("fifo created\n");
	  }
	}
	printf("fifo just opened\n");
	int n;
	int i;
	char str[100];
	int count = 0;

    //

    void *lib_handle;
	int (*fn)(int *);
	int x, res=0;

	lib_handle = dlopen("lib/funcs.so", RTLD_LAZY);

    //

	while(1){

		n = read(fd, str, 100);
		if(n<=0){
			perror("read ");
			exit(-1);
		}
		count ++;
		printf("Str %d %s (%d bytes)\n", count, str, n);

        //

        str[strlen(str) - 1] = '\0';

        fn = dlsym(lib_handle, str);

        res = (*fn)(&x);
        printf("res from linked function: %d\n", res);

        //dlclose(lib_handle);

        //

	}

	printf("fifo aberto\n");

}