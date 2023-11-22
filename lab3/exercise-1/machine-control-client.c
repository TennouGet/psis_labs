#include "remote-char.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

int main()
{	
    //TODO_4

    int write_fd = 0;

    while((write_fd = open(SERVER_INPUT, O_WRONLY))== -1){
	  if(mkfifo(SERVER_INPUT, 0666)!=0){
			printf("problem creating the write fifo\n");
			exit(-1);
	  }else{
		  printf("write fifo created\n");
	  }
	}
    
	printf("fifo just opened\n");

    //TODO_5

    char character[10];

    printf("Select a character to use:");
	fgets(character, 10, stdin);
    printf("Selected character: %s.", character);

    struct remote_char_t join, move;

    join.msg_type = 0;
    join.ch = character[0];

    move.ch = character[0];
    move.msg_type = 1;

    // TODO_6

    write(write_fd, &join, sizeof(join));

    int sleep_delay;
    direction_t direction;
    int n = 0;
    while (1)
    {
        sleep_delay = random()%700000;
        usleep(sleep_delay);
        direction = random()%4;
        n++;
        switch (direction)
        {
        case LEFT:
           printf("%d Going Left   ", n);
           move.direction = LEFT;
            break;
        case RIGHT:
            printf("%d Going Right   ", n);
            move.direction = RIGHT;
           break;
        case DOWN:
            printf("%d Going Down   ", n);
            move.direction = DOWN;
            break;
        case UP:
            printf("%d Going Up    ", n);
            move.direction = UP;
            break;
        }

        //TODO_9

        //done!

        //TODO_10

        write(write_fd, &move, sizeof(move));
    }

 
	return 0;
}