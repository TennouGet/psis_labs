#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int main(){


	int fd, read_fd;
	char str[100], read_str[100];
	int n;
	while((fd = open("/tmp/fifo_teste", O_WRONLY))== -1){
	  if(mkfifo("/tmp/fifo_teste", 0666)!=0){
			printf("problem creating the fifo\n");
			exit(-1);
	  }else{
		  printf("fifo created\n");
	  }
	}
	printf("fifo just opened for writing\n");

/*	while((read_fd = open("/tmp/fifo_one_server_writa", O_RDONLY))== -1){
	  if(mkfifo("/tmp/fifo_one_server_writa", 0666)!=0){
			printf("problem creating the read fifo\n");
			exit(-1);
	  }else{
		  printf("read fifo created\n");
	  }
	}
	printf("read fifo just opened\n");*/

	int count = 0, mode = 0;

	while(1){

		if(mode==0){
			fd = open("/tmp/fifo_teste", O_WRONLY);
			printf("write a string:");
			fgets(str, 100, stdin);
			write(fd, str, 100);
			close(fd);
			mode=1;
		}


		if(mode==1){
			read_fd = open("/tmp/fifo_one_server_writa", O_RDONLY);
			n = read(read_fd, read_str, 100);
			if(n<=0){
				perror("read ");
				//exit(-1);
			}
			count ++;
			printf("Str %d %s (%d bytes)\n", count, read_str, n);
			close(read_fd);
			mode=0;
		}
		

	}

}
