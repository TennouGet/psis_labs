
#include <ncurses.h>
#include "remote-char.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#define WINDOW_SIZE 15

typedef struct char_n_pos {

   char ch;

   int x;

   int y;

} char_n_pos;

direction_t random_direction(){
    return  random()%4;

}

void new_position(int* x, int *y, direction_t direction){
    switch (direction)
    {
    case UP:
        (*x) --;
        if(*x ==0)
            *x = 2;
        break;
    case DOWN:
        (*x) ++;
        if(*x ==WINDOW_SIZE-1)
            *x = WINDOW_SIZE-3;
        break;
    case LEFT:
        (*y) --;
        if(*y ==0)
            *y = 2;
        break;
    case RIGHT:
        (*y) ++;
        if(*y ==WINDOW_SIZE-1)
            *y = WINDOW_SIZE-3;
        break;
    default:
        break;
    }
}

int main()
{	
    struct char_n_pos characters[10];
    int characters_n = 0;

	// TODO_3
    // create and open the FIFO for reading
    
    int read_fd = 0;

    while((read_fd = open(SERVER_INPUT, O_RDONLY))== -1){
	  if(mkfifo(SERVER_INPUT, 0666)!=0){
			printf("problem creating the read fifo\n");
			exit(-1);
	  }else{
		  printf("read fifo created\n");
	  }
	}
    printf("read fifo opened!\n");

    struct remote_char_t client;


    // ncurses initialization
	initscr();		    	
	cbreak();				
    keypad(stdscr, TRUE);   
	noecho();	    


    /* creates a window and draws a border */
    WINDOW * my_win = newwin(WINDOW_SIZE, WINDOW_SIZE, 0, 0);
    box(my_win, 0 , 0);	
	wrefresh(my_win);

    /* information about the character */
    int ch;
    int pos_x = WINDOW_SIZE/2;
    int pos_y = WINDOW_SIZE/2;



    direction_t  direction;
    char str[20];
    int n, count;

    while (1)
    {
        // TODO_7
        // receive message from the clients

        n = read(read_fd, &client, sizeof(client));
		if(n<=0){
			perror("read ");
			exit(-1);
		}

        //TODO_8
        // process connection messages

        if(client.msg_type == 0){
            characters[characters_n].ch = client.ch;
            characters[characters_n].x = WINDOW_SIZE/2;
            characters[characters_n].y = WINDOW_SIZE/2;
            characters_n++;
        }
        
        // TODO_11
        // process the movement message

        int i = 0;
        
        if(client.msg_type == 1){

            for(i=0; i < characters_n; i++){
                if(characters[i].ch == client.ch)
                    break;
            }

            wmove(my_win, characters[i].y, characters[i].x);
            waddch(my_win,' ');

            new_position(&characters[i].y, &characters[i].x, client.direction);

            wmove(my_win, characters[i].y, characters[i].x);
            waddch(my_win, characters[i].ch);

            wrefresh(my_win);
        }

    }
  	endwin();			/* End curses mode		  */

	return 0;
}