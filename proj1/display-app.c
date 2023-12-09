
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

void update_window(WINDOW * my_win, remote_screen screen, int mode){

    int i = 0;

    if(mode==0 || mode==2){ //delete lizard at previous position

        mvwaddch(my_win, screen.old_x, screen.old_y, ' ');

        switch (screen.old_direction)
        {
        case UP:
            for(i=0; i < 5; i++){
                mvwaddch(my_win, screen.old_x+i, screen.old_y, ' ');
            }
            break;
        case DOWN:
            for(i=0; i < 5; i++){
                mvwaddch(my_win, screen.old_x-i, screen.old_y, ' ');
            }
            break;
        case LEFT:
            for(i=0; i < 5; i++){
                mvwaddch(my_win, screen.old_x, screen.old_y+i, ' ');
            }
            break;
        case RIGHT:
            for(i=0; i < 5; i++){
                mvwaddch(my_win, screen.old_x, screen.old_y-i, ' ');
            }
            break;
        default:
            mvprintw(WINDOW_SIZE+3, 0, "\rERROR: no lizard direction.");
            break;
        }
    }

    if(mode==1 || mode==2){ //add lizard at new position
        
        switch (screen.new_direction)
        {
        case UP:
            for(i=0; i < 5; i++){
                mvwaddch(my_win, screen.new_x+i, screen.new_y, '.');
            }
            break;
        case DOWN:
            for(i=0; i < 5; i++){
                wmove(my_win, screen.new_x-i, screen.new_y);
                waddch(my_win, '.');
            }
            break;
        case LEFT:
            for(i=0; i < 5; i++){
                mvwaddch(my_win, screen.new_x, screen.new_y+i, '.');
            }
            break;
        case RIGHT:
            for(i=0; i < 5; i++){
                mvwaddch(my_win, screen.new_x, screen.new_y-i, '.');
            }
            break;
        default:
            mvprintw(WINDOW_SIZE+3, 0, "\rERROR: no lizard direction.");
            break;
        }

        mvwaddch(my_win, screen.new_x, screen.new_y, screen.ch);
    }

}

int main()
{	
    printf("hello from remote\n");

    struct char_n_pos characters[10];
    int characters_n = 0;

    // screen matrix 
    int screen_roaches[10][5];

    int n = 0;
    while(n!=10){
        screen_roaches[n][0]=0;
        screen_roaches[n][1]=0;
        screen_roaches[n][2]=0;
        screen_roaches[n][3]=0;
        screen_roaches[n][4]=0;
        n++;
    }


    
    int read_fd = 0;

    char sub_name[20] = "screen";

    // Socket to talk to screns
    void *context = zmq_ctx_new();
    void *subscriber = zmq_socket(context, ZMQ_SUB);
    zmq_connect(subscriber, "tcp://localhost:5557");
    zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, sub_name, strlen(sub_name));

    printf("Connected to server");

    printf("sub_name: %s", sub_name);

    char buffer[20];

    struct remote_screen screen;

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

    while (1)
    {

        // receive message from screen trough socket
        zmq_recv (subscriber, &screen, sizeof(screen), 0);
        
        // process the movement message

        int i = 0;
        
        if(screen.msg_type == 1){

            
            update_window(my_win, screen, 2);

            wrefresh(my_win);
        }

        if(screen.msg_type == 3){

            
            update_window(my_win, screen, 2);

            wrefresh(my_win);
        }

    }
  	endwin();			// End curses mode

	return 0;
}