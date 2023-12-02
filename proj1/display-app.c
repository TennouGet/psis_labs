
#include <ncurses.h>
#include "header.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#define WINDOW_SIZE 15

#include <zmq.h>
#include <assert.h>
#include <string.h>


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
    printf("hello from remote\n");

    struct char_n_pos characters[10];
    int characters_n = 0;

    
    int read_fd = 0;

    char sub_name[20] = "screen";

    // Socket to talk to screns
    void *context = zmq_ctx_new();
    void *subscriber = zmq_socket(context, ZMQ_SUB);
    zmq_connect(subscriber, "tcp://localhost:5556");
    zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, sub_name, strlen(sub_name));

    printf("Connected to server");

    printf("sub_name: %s", sub_name);

    char buffer[20];

    zmq_recv (subscriber, buffer, strlen(buffer), 0);
    printf("buffer: %s", buffer);

    struct remote_screen screen;

    
    // ncurses initialization
	initscr();		    	
	cbreak();				
    keypad(stdscr, TRUE);   
	noecho();	    


    // creates a window and draws a border 
    WINDOW * my_win = newwin(WINDOW_SIZE, WINDOW_SIZE, 0, 0);
    box(my_win, 0 , 0);	
	wrefresh(my_win);

    while (1)
    {
        // TODO_7
        // receive message from screen trough socket

        zmq_recv (subscriber, &screen, sizeof(screen), 0);
        printf("recieved: %d", screen.pos_x);
        
        // TODO_11
        // process the movement message

        int i = 0;
        int found = 0;
        
        if(screen.msg_type == 1){

            for(i=0; i < characters_n; i++){
                if(characters[i].ch == screen.ch){
                    found = 1;
                    break;
                }
            }

            if(found==0){
                characters[characters_n].ch = screen.ch;
                characters[characters_n].x = screen.pos_x;
                characters[characters_n].y = screen.pos_y;
                characters_n++;
            }
            else{
                wmove(my_win, characters[i].y, characters[i].x);
                waddch(my_win,' ');
            }

            wmove(my_win, screen.pos_y, screen.pos_x);
            waddch(my_win, characters[i].ch);

            wrefresh(my_win);

            found = 0;
        }

    }
  	endwin();			// End curses mode

	return 0;
}