#include <stdio.h>
#include <zmq.h>
#include <stdlib.h>

#include <string.h>
#include "remote-char.h"
#include <ncurses.h>
#define WINDOW_SIZE 15

typedef struct char_n_pos {

   char ch;

   int x;

   int y;

} char_n_pos;

int main(){
    char line[100];
    char dpt_name[100] = "screen";
    //printf("What is the department of this building? (DEEC, DEI, ...)");
    //fgets(line, 100, stdin);
    //sscanf(line, "%s", &dpt_name);
    //printf("We will broadcast all messages from the president of IST and %s\n", dpt_name);

    // Connect to the server using ZMQ_SUB

    void *context = zmq_ctx_new ();
    void *subscriber = zmq_socket (context, ZMQ_SUB);
    zmq_connect(subscriber, "tcp://localhost:5557");
    zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, dpt_name, strlen(dpt_name));

    printf("Connected to server");
    
    // subscribe to topics

    struct remote_screen screen;

    struct char_n_pos characters[10];
    int characters_n = 0;

    char buffer[20];

    zmq_recv (subscriber, buffer, strlen(buffer), 0);
    printf("buffer: %s", buffer);

    // ncurses initialization
	initscr();		    	
	cbreak();				
    keypad(stdscr, TRUE);   
	noecho();	    


    // creates a window and draws a border 
    WINDOW * my_win = newwin(WINDOW_SIZE, WINDOW_SIZE, 0, 0);
    box(my_win, 0 , 0);	
	wrefresh(my_win);
    
    while(1){

        // receive messages
        zmq_recv (subscriber, &screen, sizeof(screen), 0);
        //printf("message from server pox_y=%d\n", screen.pos_y);

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
            if(found == 1){
                wmove(my_win, characters[i].y, characters[i].x);
                waddch(my_win,' ');
            }

            wmove(my_win, screen.pos_y, screen.pos_x);
            waddch(my_win, characters[i].ch);

            characters[i].x = screen.pos_x;
            characters[i].y = screen.pos_y;

            wrefresh(my_win);

            found = 0;
        }
        
    }

    zmq_close (subscriber);
    zmq_ctx_destroy (context);

}