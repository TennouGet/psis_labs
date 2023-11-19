#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>
#include <dlfcn.h>

void send_message(int write_fd, char* pt){

	while((write_fd = open("/tmp/fifo_one_server_writa", O_WRONLY))== -1){
	  if(mkfifo("/tmp/fifo_one_server_writa", 0666)!=0){
			printf("problem creating the write fifo\n");
			exit(-1);
	  }else{
		  printf("write fifo created\n");
	  }
	}

	printf("sending function result");
	write(write_fd, pt, strlen(pt));

}

int main(){


	int read_fd, write_fd;

	while((read_fd = open("/tmp/fifo_teste", O_RDONLY))== -1){
	  if(mkfifo("/tmp/fifo_teste", 0666)!=0){
			printf("problem creating the read fifo\n");
			exit(-1);
	  }else{
		  printf("read fifo created\n");
	  }
	}
	printf("read fifo just opened\n");

	int n;
	int i;
	char read_str[100], read_x[10];
	int count = 0;

    //

    void *lib_handle;
	int (*fn)(int );
	int x, res=0;

	lib_handle = dlopen("lib/funcs.so", RTLD_LAZY);

    //

	int mode = 0;

	while(1){

		if(mode==0){
			read_fd = open("/tmp/fifo_teste", O_RDONLY);
			n = read(read_fd, read_str, 100);
			if(n<=0){
				perror("read ");
				exit(-1);
			}
			count ++;
			printf("Str %d %s (%d bytes)\n", count, read_str, n);

			//

			read_str[strlen(read_str) - 1] = '\0';

			fn = dlsym(lib_handle, read_str);

			read(read_fd, read_x, 10);
			int x = atoi(read_x);
			res = (*fn)(x);
			printf("res from linked function: %d\n", res);

			close(read_fd);
			mode=1;
		}

		if(mode==1){
			char res_s [50];
			sprintf(res_s, "%d", res);
			send_message(write_fd, &res_s[0]);
			close(write_fd);
			mode=0;
		}
		

        //dlclose(lib_handle);

        //

	}

	printf("fifo aberto\n");

}