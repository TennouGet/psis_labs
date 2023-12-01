
#include <ncurses.h>
#include "remote-char.h"
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
    struct char_n_pos characters[25];
    int characters_n = 0;

    
    int read_fd = 0;

    // Socket to talk to clients
    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REP);
    int rc = zmq_bind (responder, "tcp://*:5555");
    assert (rc == 0);

    // Socket to publish to displays
    void *pub_context = zmq_ctx_new ();
    void *publisher = zmq_socket (pub_context, ZMQ_PUB);
    int rc2 = zmq_bind (publisher, "tcp://*:5557");
    assert(rc2 == 0);

    struct remote_char_t client;
    struct remote_screen screen;

    char buffer[20] = "screen";

    screen.ch = 'a';
    screen.msg_type=0;
    screen.pos_x=0;

    zmq_send (publisher, buffer, strlen(buffer), ZMQ_SNDMORE);
    zmq_send (publisher, &screen, sizeof(screen), 0);


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

        // receive message from client trough socket
        zmq_recv (responder, &client, sizeof(remote_char_t), 0);
        zmq_send (responder, "World", 5, 0);

        // process connection messages

        if(client.msg_type == 0){
            characters[characters_n].ch = client.ch;
            characters[characters_n].x = WINDOW_SIZE/2;
            characters[characters_n].y = WINDOW_SIZE/2;
            characters_n++;


        }
        
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

        screen.ch = characters[i].ch;
        screen.pos_x = characters[i].x;
        screen.pos_y = characters[i].y;
        screen.msg_type = 1;
        zmq_send (publisher, buffer, strlen(buffer), ZMQ_SNDMORE);
        zmq_send (publisher, &screen, sizeof(screen), 0);

    }
  	endwin();			// End curses mode

	return 0;
}